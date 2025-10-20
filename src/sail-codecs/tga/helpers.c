/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2021 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static const uint16_t TGA2_EXTENSION_AREA_LENGTH = 495;

sail_status_t tga_private_read_file_header(struct sail_io* io, struct TgaFileHeader* file_header)
{
    SAIL_TRY(io->strict_read(io->stream, &file_header->id_length, sizeof(file_header->id_length)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_type, sizeof(file_header->color_map_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->image_type, sizeof(file_header->image_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->first_color_map_entry_index,
                             sizeof(file_header->first_color_map_entry_index)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_elements, sizeof(file_header->color_map_elements)));
    SAIL_TRY(
        io->strict_read(io->stream, &file_header->color_map_entry_size, sizeof(file_header->color_map_entry_size)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->x, sizeof(file_header->x)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->y, sizeof(file_header->y)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->width, sizeof(file_header->width)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->height, sizeof(file_header->height)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->bpp, sizeof(file_header->bpp)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->descriptor, sizeof(file_header->descriptor)));

    /* Validate TGA header fields to detect non-TGA files */
    if (file_header->bpp > 32 || file_header->bpp == 0) {
        SAIL_LOG_ERROR("TGA: Invalid bpp %d in file header, must be 1-32", file_header->bpp);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }
    if (file_header->image_type > 11) {
        SAIL_LOG_ERROR("TGA: Invalid image type %d, must be 0-11", file_header->image_type);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }
    if (file_header->color_map_type > 1) {
        SAIL_LOG_ERROR("TGA: Invalid color map type %d, must be 0-1", file_header->color_map_type);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    return SAIL_OK;
}

sail_status_t tga_private_read_file_footer(struct sail_io* io, struct TgaFooter* footer)
{
    SAIL_TRY(io->strict_read(io->stream, &footer->extension_area_offset, sizeof(footer->extension_area_offset)));
    SAIL_TRY(io->strict_read(io->stream, &footer->developer_area_offset, sizeof(footer->developer_area_offset)));
    SAIL_TRY(io->strict_read(io->stream, &footer->signature, sizeof(footer->signature)));

    return SAIL_OK;
}

enum SailPixelFormat tga_private_sail_pixel_format(int image_type, int bpp)
{
    switch (image_type)
    {
    case TGA_INDEXED:
    case TGA_INDEXED_RLE:
    {
        return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    }

    case TGA_TRUE_COLOR:
    case TGA_TRUE_COLOR_RLE:
    {
        switch (bpp)
        {
        case 16: return SAIL_PIXEL_FORMAT_BPP16_BGR555;
        case 24: return SAIL_PIXEL_FORMAT_BPP24_BGR;
        case 32: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    case TGA_GRAY:
    case TGA_GRAY_RLE:
    {
        switch (bpp)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    default:
    {
        return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
    }
}

enum SailPixelFormat tga_private_palette_bpp_to_sail_pixel_format(int bpp)
{
    switch (bpp)
    {
    case 15:
    case 16:
    case 24: return SAIL_PIXEL_FORMAT_BPP24_RGB;
    case 32: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
    default:
    {
        SAIL_LOG_ERROR("TGA: Palette bit depth %d is not supported", bpp);
        return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
    }
}

sail_status_t tga_private_fetch_id(struct sail_io* io,
                                   const struct TgaFileHeader* file_header,
                                   struct sail_meta_data_node** meta_data_node)
{
    struct sail_meta_data_node* meta_data_node_local;
    SAIL_TRY(sail_alloc_meta_data_node_and_value(&meta_data_node_local));

    struct sail_meta_data* meta_data = meta_data_node_local->meta_data;

    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    meta_data->key         = SAIL_META_DATA_ID;
    meta_data->value->type = SAIL_VARIANT_TYPE_STRING;
    meta_data->value->size = file_header->id_length + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(meta_data->value->size, &meta_data->value->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, meta_data->value->value, meta_data->value->size - 1),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    (sail_variant_to_string(meta_data->value))[meta_data->value->size - 1] = '\0';

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t tga_private_fetch_extension(struct sail_io* io,
                                          double* gamma,
                                          struct sail_meta_data_node** meta_data_node)
{
    /* Find the last node. */
    struct sail_meta_data_node** last_meta_data_node = meta_data_node;

    while (*last_meta_data_node != NULL)
    {
        last_meta_data_node = &(*last_meta_data_node)->next;
    }

    /* Extension area length. */
    {
        uint16_t length;
        SAIL_TRY(io->strict_read(io->stream, &length, sizeof(length)));

        if (length != TGA2_EXTENSION_AREA_LENGTH)
        {
            SAIL_LOG_WARNING("TGA: Don't know how to handle extension area length of %u bytes (expected %u)", length,
                             TGA2_EXTENSION_AREA_LENGTH);
            return SAIL_OK;
        }
    }

    /* Author. */
    {
        char author[41];
        SAIL_TRY(io->strict_read(io->stream, author, sizeof(author)));
        author[40] = '\0';

        if (strlen(author) > 0)
        {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_AUTHOR,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, author));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Comments. */
    {
        char comments[80 * 4 + 1];
        SAIL_TRY(io->strict_read(io->stream, comments, 81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        comments[80 * 4] = '\0';

        if (strlen(comments) > 0)
        {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_COMMENT,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, comments));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Date/time stamp. YYYY.MM.DD hh:mm:ss. */
    {
        uint16_t month, day, year, hour, minute, second;
        SAIL_TRY(io->strict_read(io->stream, &month, sizeof(month)));
        SAIL_TRY(io->strict_read(io->stream, &day, sizeof(day)));
        SAIL_TRY(io->strict_read(io->stream, &year, sizeof(year)));
        SAIL_TRY(io->strict_read(io->stream, &hour, sizeof(hour)));
        SAIL_TRY(io->strict_read(io->stream, &minute, sizeof(minute)));
        SAIL_TRY(io->strict_read(io->stream, &second, sizeof(second)));

        if (month != 0 || day != 0 || year != 0 || hour != 0 || minute != 0 || second != 0)
        {
            struct tm time_tm             = {.tm_sec   = second,
                                             .tm_min   = minute,
                                             .tm_hour  = hour,
                                             .tm_mday  = day,
                                             .tm_mon   = month - 1,
                                             .tm_year  = year - 1900,
                                             .tm_wday  = 0,
                                             .tm_yday  = 0,
                                             .tm_isdst = 0};
            const unsigned long timestamp = (unsigned long)mktime(&time_tm);

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_CREATION_TIME,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_unsigned_long((*last_meta_data_node)->meta_data->value, timestamp));

            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Job Id. */
    {
        char job[41];
        SAIL_TRY(io->strict_read(io->stream, job, sizeof(job)));
        job[40] = '\0';

        if (strlen(job) > 0)
        {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(
                sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_JOB, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, job));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Job Time. */
    {
        uint16_t hour, minute, second;
        SAIL_TRY(io->strict_read(io->stream, &hour, sizeof(hour)));
        SAIL_TRY(io->strict_read(io->stream, &minute, sizeof(minute)));
        SAIL_TRY(io->strict_read(io->stream, &second, sizeof(second)));

        if (hour != 0 || minute != 0 || second != 0)
        {
            char timestamp[20];
            snprintf(timestamp, sizeof(timestamp), "%05d:%02d:%02d", hour, minute, second);

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_TIME_CONSUMED,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, timestamp));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Software. */
    {
        char software[41];
        SAIL_TRY(io->strict_read(io->stream, software, sizeof(software)));
        software[40] = '\0';

        if (strlen(software) > 0)
        {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_SOFTWARE,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, software));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Software Version. */
    {
        uint16_t version;
        uint8_t version_letter;
        SAIL_TRY(io->strict_read(io->stream, &version, sizeof(version)));
        SAIL_TRY(io->strict_read(io->stream, &version_letter, sizeof(version_letter)));

        if (version != 0)
        {
            char version_string[10];
            const double version_divided = version / 100.0;

            if (version_letter == ' ')
            {
                snprintf(version_string, sizeof(version_string), "%.2f", version_divided);
            }
            else
            {
                snprintf(version_string, sizeof(version_string), "%.2f.%c", version_divided, version_letter);
            }

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_SOFTWARE_VERSION,
                                                                   &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, version_string));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Key Color. */
    SAIL_TRY(io->seek(io->stream, 4, SEEK_CUR));

    /* Pixel Aspect Ratio. */
    SAIL_TRY(io->seek(io->stream, 4, SEEK_CUR));

    /* Gamma. */
    {
        uint16_t gamma_num, gamma_denom;
        SAIL_TRY(io->strict_read(io->stream, &gamma_num, sizeof(gamma_num)));
        SAIL_TRY(io->strict_read(io->stream, &gamma_denom, sizeof(gamma_denom)));

        if (gamma_denom != 0)
        {
            *gamma = (double)gamma_num / gamma_denom;
        }
    }

    return SAIL_OK;
}

sail_status_t tga_private_fetch_palette(struct sail_io* io,
                                        const struct TgaFileHeader* file_header,
                                        struct sail_palette** palette)
{
    size_t element_size_in_bytes = ((size_t)file_header->color_map_entry_size + 7) / 8;
    size_t bytes_to_skip         = file_header->first_color_map_entry_index * element_size_in_bytes;

    if (bytes_to_skip > 0)
    {
        SAIL_TRY(io->seek(io->stream, (long)bytes_to_skip, SEEK_CUR));
    }

    enum SailPixelFormat palette_pixel_format =
        tga_private_palette_bpp_to_sail_pixel_format(file_header->color_map_entry_size);

    if (palette_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    unsigned palette_elements = (unsigned)file_header->color_map_elements - file_header->first_color_map_entry_index;
    struct sail_palette* palette_local;

    SAIL_TRY(sail_alloc_palette_for_data(palette_pixel_format, palette_elements, &palette_local));

    unsigned char* palette_data = palette_local->data;

    for (int i = file_header->first_color_map_entry_index; i < file_header->color_map_elements; i++)
    {
        SAIL_ALIGNAS(uint16_t) unsigned char data[4];

        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, data, element_size_in_bytes),
                            /* cleanup */ sail_destroy_palette(palette_local));

        switch (file_header->color_map_entry_size)
        {
        case 15:
        case 16:
        {
            const uint16_t* data_as_word = (const uint16_t*)data;

            *palette_data++ = (unsigned char)(((*data_as_word & 0x1f) << 3) << 0);
            *palette_data++ = (unsigned char)(((*data_as_word & 0x3e0) >> 5) << 3);
            *palette_data++ = (unsigned char)(((*data_as_word & 0x7c00) >> 10) << 3);
            break;
        }

        case 24:
        {
            *palette_data++ = data[2];
            *palette_data++ = data[1];
            *palette_data++ = data[0];
            break;
        }

        case 32:
        {
            *palette_data++ = data[2];
            *palette_data++ = data[1];
            *palette_data++ = data[0];
            *palette_data++ = data[3];
            break;
        }

        default:
        {
            SAIL_LOG_ERROR("TGA: Internal error: Unhandled palette pixel format");
            sail_destroy_palette(palette_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
        }
    }

    *palette = palette_local;

    return SAIL_OK;
}

sail_status_t tga_private_write_file_header(struct sail_io* io, const struct TgaFileHeader* file_header)
{
    SAIL_TRY(io->strict_write(io->stream, &file_header->id_length, sizeof(file_header->id_length)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->color_map_type, sizeof(file_header->color_map_type)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->image_type, sizeof(file_header->image_type)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->first_color_map_entry_index,
                              sizeof(file_header->first_color_map_entry_index)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->color_map_elements, sizeof(file_header->color_map_elements)));
    SAIL_TRY(
        io->strict_write(io->stream, &file_header->color_map_entry_size, sizeof(file_header->color_map_entry_size)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->x, sizeof(file_header->x)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->y, sizeof(file_header->y)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->width, sizeof(file_header->width)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->height, sizeof(file_header->height)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->bpp, sizeof(file_header->bpp)));
    SAIL_TRY(io->strict_write(io->stream, &file_header->descriptor, sizeof(file_header->descriptor)));

    return SAIL_OK;
}

sail_status_t tga_private_write_file_footer(struct sail_io* io, const struct TgaFooter* footer)
{
    SAIL_TRY(io->strict_write(io->stream, &footer->extension_area_offset, sizeof(footer->extension_area_offset)));
    SAIL_TRY(io->strict_write(io->stream, &footer->developer_area_offset, sizeof(footer->developer_area_offset)));
    SAIL_TRY(io->strict_write(io->stream, footer->signature, sizeof(footer->signature)));

    return SAIL_OK;
}

sail_status_t tga_private_write_extension_area(struct sail_io* io,
                                               double gamma,
                                               const struct sail_meta_data_node* meta_data_node)
{
    /* Extension area size (495 bytes for TGA 2.0). */
    uint16_t extension_size = TGA2_EXTENSION_AREA_LENGTH;
    SAIL_TRY(io->strict_write(io->stream, &extension_size, sizeof(extension_size)));

    /* Author Name (41 bytes). */
    char author[41] = {0};
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_AUTHOR && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* author_str = sail_variant_to_string(node->meta_data->value);
            if (author_str != NULL)
            {
#ifdef _MSC_VER
                strncpy_s(author, sizeof(author), author_str, 40);
#else
                strncpy(author, author_str, 40);
#endif
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, author, sizeof(author)));

    /* Comments (324 bytes = 4 lines x 81 bytes). */
    char comments[324] = {0};
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_COMMENT && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* comment_str = sail_variant_to_string(node->meta_data->value);
            if (comment_str != NULL)
            {
#ifdef _MSC_VER
                strncpy_s(comments, sizeof(comments), comment_str, 323);
#else
                strncpy(comments, comment_str, 323);
#endif
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, comments, sizeof(comments)));

    /* Date/Time Stamp (12 bytes). */
    uint16_t month = 0, day = 0, year = 0, hour = 0, minute = 0, second = 0;
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_CREATION_TIME
            && node->meta_data->value->type == SAIL_VARIANT_TYPE_UNSIGNED_LONG)
        {
            time_t timestamp = (time_t)sail_variant_to_unsigned_long(node->meta_data->value);
            struct tm time_tm;

#ifdef _MSC_VER
            if (localtime_s(&time_tm, &timestamp) == 0)
            {
#else
            if (localtime_r(&timestamp, &time_tm) != NULL)
            {
#endif
                month  = (uint16_t)(time_tm.tm_mon + 1);
                day    = (uint16_t)time_tm.tm_mday;
                year   = (uint16_t)(time_tm.tm_year + 1900);
                hour   = (uint16_t)time_tm.tm_hour;
                minute = (uint16_t)time_tm.tm_min;
                second = (uint16_t)time_tm.tm_sec;
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, &month, sizeof(month)));
    SAIL_TRY(io->strict_write(io->stream, &day, sizeof(day)));
    SAIL_TRY(io->strict_write(io->stream, &year, sizeof(year)));
    SAIL_TRY(io->strict_write(io->stream, &hour, sizeof(hour)));
    SAIL_TRY(io->strict_write(io->stream, &minute, sizeof(minute)));
    SAIL_TRY(io->strict_write(io->stream, &second, sizeof(second)));

    /* Job Name/ID (41 bytes). */
    char job[41] = {0};
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_JOB && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* job_str = sail_variant_to_string(node->meta_data->value);
            if (job_str != NULL)
            {
#ifdef _MSC_VER
                strncpy_s(job, sizeof(job), job_str, 40);
#else
                strncpy(job, job_str, 40);
#endif
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, job, sizeof(job)));

    /* Job Time (6 bytes). */
    uint16_t job_hour = 0, job_minute = 0, job_second = 0;
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_TIME_CONSUMED
            && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* time_str = sail_variant_to_string(node->meta_data->value);
            if (time_str != NULL)
            {
#ifdef _MSC_VER
                sscanf_s(time_str, "%hu:%hu:%hu", &job_hour, &job_minute, &job_second);
#else
                sscanf(time_str, "%hu:%hu:%hu", &job_hour, &job_minute, &job_second);
#endif
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, &job_hour, sizeof(job_hour)));
    SAIL_TRY(io->strict_write(io->stream, &job_minute, sizeof(job_minute)));
    SAIL_TRY(io->strict_write(io->stream, &job_second, sizeof(job_second)));

    /* Software ID (41 bytes). */
    char software[41] = {0};
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_SOFTWARE && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* software_str = sail_variant_to_string(node->meta_data->value);
            if (software_str != NULL)
            {
#ifdef _MSC_VER
                strncpy_s(software, sizeof(software), software_str, 40);
#else
                strncpy(software, software_str, 40);
#endif
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, software, sizeof(software)));

    /* Software Version (3 bytes). */
    uint16_t version       = 0;
    uint8_t version_letter = ' ';
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_SOFTWARE_VERSION
            && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* version_str = sail_variant_to_string(node->meta_data->value);
            if (version_str != NULL)
            {
                double version_double;
                char letter;
#ifdef _MSC_VER
                if (sscanf_s(version_str, "%lf.%c", &version_double, &letter, 1) == 2)
                {
#else
                if (sscanf(version_str, "%lf.%c", &version_double, &letter) == 2)
                {
#endif
                    version        = (uint16_t)(version_double * 100);
                    version_letter = (uint8_t)letter;
#ifdef _MSC_VER
                }
                else if (sscanf_s(version_str, "%lf", &version_double) == 1)
                {
#else
                }
                else if (sscanf(version_str, "%lf", &version_double) == 1)
                {
#endif
                    version = (uint16_t)(version_double * 100);
                }
            }
            break;
        }
    }
    SAIL_TRY(io->strict_write(io->stream, &version, sizeof(version)));
    SAIL_TRY(io->strict_write(io->stream, &version_letter, sizeof(version_letter)));

    /* Key Color (4 bytes - ARGB). */
    uint32_t key_color = 0;
    SAIL_TRY(io->strict_write(io->stream, &key_color, sizeof(key_color)));

    /* Pixel Aspect Ratio (4 bytes). */
    uint16_t pixel_aspect_num   = 0;
    uint16_t pixel_aspect_denom = 0;
    SAIL_TRY(io->strict_write(io->stream, &pixel_aspect_num, sizeof(pixel_aspect_num)));
    SAIL_TRY(io->strict_write(io->stream, &pixel_aspect_denom, sizeof(pixel_aspect_denom)));

    /* Gamma Value (4 bytes). */
    uint16_t gamma_num   = 0;
    uint16_t gamma_denom = 0;
    if (gamma > 0.0)
    {
        gamma_num   = (uint16_t)(gamma * 1000);
        gamma_denom = 1000;
    }
    SAIL_TRY(io->strict_write(io->stream, &gamma_num, sizeof(gamma_num)));
    SAIL_TRY(io->strict_write(io->stream, &gamma_denom, sizeof(gamma_denom)));

    /* Color Correction Offset (4 bytes). */
    uint32_t color_correction_offset = 0;
    SAIL_TRY(io->strict_write(io->stream, &color_correction_offset, sizeof(color_correction_offset)));

    /* Postage Stamp Offset (4 bytes). */
    uint32_t postage_stamp_offset = 0;
    SAIL_TRY(io->strict_write(io->stream, &postage_stamp_offset, sizeof(postage_stamp_offset)));

    /* Scan Line Offset (4 bytes). */
    uint32_t scan_line_offset = 0;
    SAIL_TRY(io->strict_write(io->stream, &scan_line_offset, sizeof(scan_line_offset)));

    /* Attributes Type (1 byte). */
    uint8_t attributes_type = 3; /* 3 = useful alpha channel data. */
    SAIL_TRY(io->strict_write(io->stream, &attributes_type, sizeof(attributes_type)));

    return SAIL_OK;
}

void tga_private_pixel_format_to_tga_format(enum SailPixelFormat pixel_format, uint8_t* image_type, uint8_t* bpp)
{
    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
    {
        *image_type = TGA_INDEXED;
        *bpp        = 8;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
    {
        *image_type = TGA_GRAY;
        *bpp        = 8;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP16_BGR555:
    {
        *image_type = TGA_TRUE_COLOR;
        *bpp        = 16;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP24_BGR:
    {
        *image_type = TGA_TRUE_COLOR;
        *bpp        = 24;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
    {
        *image_type = TGA_TRUE_COLOR;
        *bpp        = 32;
        break;
    }
    default:
    {
        *image_type = TGA_NO_IMAGE;
        *bpp        = 0;
    }
    }
}

sail_status_t tga_private_write_palette(struct sail_io* io,
                                        const struct sail_palette* palette,
                                        struct TgaFileHeader* file_header)
{
    /* Write palette data converting RGB(A) to BGR(A). */
    const unsigned bytes_per_entry    = (file_header->color_map_entry_size + 7) / 8;
    const unsigned char* palette_data = palette->data;

    for (unsigned i = 0; i < palette->color_count; i++)
    {
        unsigned char entry[4];

        if (bytes_per_entry == 3)
        {
            /* RGB -> BGR. */
            entry[0] = palette_data[2];
            entry[1] = palette_data[1];
            entry[2] = palette_data[0];
        }
        else if (bytes_per_entry == 4)
        {
            /* RGBA -> BGRA. */
            entry[0] = palette_data[2];
            entry[1] = palette_data[1];
            entry[2] = palette_data[0];
            entry[3] = palette_data[3];
        }

        SAIL_TRY(io->strict_write(io->stream, entry, bytes_per_entry));
        palette_data += bytes_per_entry;
    }

    return SAIL_OK;
}
