/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-common.h"

#include "helpers.h"

static const uint16_t TGA2_EXTENSION_AREA_LENGTH = 495;

sail_status_t tga_private_read_file_header(struct sail_io *io, struct TgaFileHeader *file_header) {

    SAIL_TRY(io->strict_read(io->stream, &file_header->id_length,                   sizeof(file_header->id_length)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_type,              sizeof(file_header->color_map_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->image_type,                  sizeof(file_header->image_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->first_color_map_entry_index, sizeof(file_header->first_color_map_entry_index)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_elements,          sizeof(file_header->color_map_elements)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_entry_size,        sizeof(file_header->color_map_entry_size)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->x,                           sizeof(file_header->x)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->y,                           sizeof(file_header->y)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->width,                       sizeof(file_header->width)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->height,                      sizeof(file_header->height)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->bpp,                         sizeof(file_header->bpp)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->descriptor,                  sizeof(file_header->descriptor)));

    return SAIL_OK;
}

sail_status_t tga_private_read_file_footer(struct sail_io *io, struct TgaFooter *footer) {

    SAIL_TRY(io->strict_read(io->stream, &footer->extension_area_offset, sizeof(footer->extension_area_offset)));
    SAIL_TRY(io->strict_read(io->stream, &footer->developer_area_offset, sizeof(footer->developer_area_offset)));
    SAIL_TRY(io->strict_read(io->stream, &footer->signature,             sizeof(footer->signature)));

    return SAIL_OK;
}

enum SailPixelFormat tga_private_sail_pixel_format(int image_type, int bpp) {

    switch (image_type) {
        case TGA_INDEXED:
        case TGA_INDEXED_RLE: {
            return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        }

        case TGA_TRUE_COLOR:
        case TGA_TRUE_COLOR_RLE: {
            switch (bpp) {
                case 16: return SAIL_PIXEL_FORMAT_BPP16_BGR555;
                case 24: return SAIL_PIXEL_FORMAT_BPP24_BGR;
                case 32: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        case TGA_GRAY:
        case TGA_GRAY_RLE: {
            switch (bpp) {
                case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

enum SailPixelFormat tga_private_palette_bpp_to_sail_pixel_format(int bpp) {

    switch (bpp) {
        case 15:
        case 16:
        case 24: return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case 32: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        default: {
            SAIL_LOG_ERROR("TGA: Palette bit depth %d is not supported", bpp);
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

sail_status_t tga_private_fetch_id(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_meta_data_node **meta_data_node) {

    struct sail_meta_data_node *meta_data_node_local;
    SAIL_TRY(sail_alloc_meta_data_node_and_value(&meta_data_node_local));

    struct sail_meta_data *meta_data = meta_data_node_local->meta_data;

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

sail_status_t tga_private_fetch_extension(struct sail_io *io, double *gamma, struct sail_meta_data_node **meta_data_node) {

    /* Find the last node. */
    struct sail_meta_data_node **last_meta_data_node = meta_data_node;

    while (*last_meta_data_node != NULL) {
        last_meta_data_node = &(*last_meta_data_node)->next;
    }

    /* Extension area length. */
    {
        uint16_t length;
        SAIL_TRY(io->strict_read(io->stream, &length, sizeof(length)));

        if (length != TGA2_EXTENSION_AREA_LENGTH) {
            SAIL_LOG_WARNING("TGA: Don't know how to handle extension area length of %u bytes (expected %u)", length, TGA2_EXTENSION_AREA_LENGTH);
            return SAIL_OK;
        }
    }

    /* Author. */
    {
        char author[41];
        SAIL_TRY(io->strict_read(io->stream, author, sizeof(author)));
        author[40] = '\0';

        if (strlen(author) > 0) {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_AUTHOR, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, author));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Comments. */
    {
        char comments[80*4 + 1];
        SAIL_TRY(io->strict_read(io->stream, comments,                    81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        SAIL_TRY(io->strict_read(io->stream, comments + strlen(comments), 81));
        comments[80*4] = '\0';

        if (strlen(comments) > 0) {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_COMMENT, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, comments));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Date/time stamp. YYYY.MM.DD hh:mm:ss. */
    {
        uint16_t month, day, year, hour, minute, second;
        SAIL_TRY(io->strict_read(io->stream, &month,  sizeof(month)));
        SAIL_TRY(io->strict_read(io->stream, &day,    sizeof(day)));
        SAIL_TRY(io->strict_read(io->stream, &year,   sizeof(year)));
        SAIL_TRY(io->strict_read(io->stream, &hour,   sizeof(hour)));
        SAIL_TRY(io->strict_read(io->stream, &minute, sizeof(minute)));
        SAIL_TRY(io->strict_read(io->stream, &second, sizeof(second)));

        if (month != 0 || day != 0 || year != 0 || hour != 0 || minute != 0 || second != 0) {
            struct tm time_tm = {
                .tm_sec   = second,
                .tm_min	  = minute,
                .tm_hour  = hour,
                .tm_mday  = day,
                .tm_mon	  = month - 1,
                .tm_year  = year - 1900,
                .tm_wday  = 0,
                .tm_yday  = 0,
                .tm_isdst = 0
            };
            const unsigned long timestamp = (unsigned long)mktime(&time_tm);

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_CREATION_TIME, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
            SAIL_TRY(sail_set_variant_unsigned_long((*last_meta_data_node)->meta_data->value, timestamp));

            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Job Id. */
    {
        char job[41];
        SAIL_TRY(io->strict_read(io->stream, job, sizeof(job)));
        job[40] = '\0';

        if (strlen(job) > 0) {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_JOB, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, job));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Job Time. */
    {
        uint16_t hour, minute, second;
        SAIL_TRY(io->strict_read(io->stream, &hour,   sizeof(hour)));
        SAIL_TRY(io->strict_read(io->stream, &minute, sizeof(minute)));
        SAIL_TRY(io->strict_read(io->stream, &second, sizeof(second)));

        if (hour != 0 || minute != 0 || second != 0) {
            char timestamp[20];
            snprintf(timestamp, sizeof(timestamp), "%05d:%02d:%02d", hour, minute, second);

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_TIME_CONSUMED, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
            SAIL_TRY(sail_set_variant_string((*last_meta_data_node)->meta_data->value, timestamp));
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    /* Software. */
    {
        char software[41];
        SAIL_TRY(io->strict_read(io->stream, software, sizeof(software)));
        software[40] = '\0';

        if (strlen(software) > 0) {
            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_SOFTWARE, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
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

        if (version != 0) {
            char version_string[10];
            const double version_divided = version / 100.0;

            if (version_letter == ' ') {
                snprintf(version_string, sizeof(version_string), "%.2f", version_divided);
            } else {
                snprintf(version_string, sizeof(version_string), "%.2f.%c", version_divided, version_letter);
            }

            SAIL_TRY(sail_alloc_meta_data_node(last_meta_data_node));
            SAIL_TRY(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_SOFTWARE_VERSION, &(*last_meta_data_node)->meta_data));
            SAIL_TRY(sail_alloc_variant(&(*last_meta_data_node)->meta_data->value));
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

        if (gamma_denom != 0) {
            *gamma = (double)gamma_num / gamma_denom;
        }
    }

    return SAIL_OK;
}

sail_status_t tga_private_fetch_palette(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_palette **palette) {

    size_t element_size_in_bytes = ((size_t)file_header->color_map_entry_size + 7) / 8;
    size_t bytes_to_skip = file_header->first_color_map_entry_index * element_size_in_bytes;

    if (bytes_to_skip > 0) {
        SAIL_TRY(io->seek(io->stream, (long)bytes_to_skip, SEEK_CUR));
    }

    enum SailPixelFormat palette_pixel_format = tga_private_palette_bpp_to_sail_pixel_format(file_header->color_map_entry_size);

    if (palette_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    unsigned palette_elements = (unsigned)file_header->color_map_elements - file_header->first_color_map_entry_index;
    struct sail_palette *palette_local;

    SAIL_TRY(sail_alloc_palette_for_data(palette_pixel_format, palette_elements, &palette_local));

    unsigned char *palette_data = palette_local->data;

    for (int i = file_header->first_color_map_entry_index; i < file_header->color_map_elements; i++) {
        SAIL_ALIGNAS(uint16_t) unsigned char data[4];

        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, data, element_size_in_bytes),
                            /* cleanup */ sail_destroy_palette(palette_local));

        switch (file_header->color_map_entry_size) {
            case 15:
            case 16: {
                const uint16_t *data_as_word = (const uint16_t *)data;

                *palette_data++ = (unsigned char)(((*data_as_word & 0x1f)   <<  3) << 0);
                *palette_data++ = (unsigned char)(((*data_as_word & 0x3e0)  >>  5) << 3);
                *palette_data++ = (unsigned char)(((*data_as_word & 0x7c00) >> 10) << 3);
                break;
            }

            case 24: {
                *palette_data++ = data[2];
                *palette_data++ = data[1];
                *palette_data++ = data[0];
                break;
            }

            case 32: {
                *palette_data++ = data[2];
                *palette_data++ = data[1];
                *palette_data++ = data[0];
                *palette_data++ = data[3];
                break;
            }

            default: {
                SAIL_LOG_ERROR("TGA: Internal error: Unhandled palette pixel format");
                sail_destroy_palette(palette_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }
    }

    *palette = palette_local;

    return SAIL_OK;
}
