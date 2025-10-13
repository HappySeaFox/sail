/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

bool hdr_private_is_hdr(const void* data, size_t size)
{
    if (data == NULL || size < 11)
    {
        return false;
    }

    const char* str = data;

    /* Check for "#?RADIANCE" or "#?RGBE" signature. */
    if (strncmp(str, "#?RADIANCE", 10) == 0 || strncmp(str, "#?RGBE", 6) == 0)
    {
        return true;
    }

    return false;
}

static sail_status_t read_line(struct sail_io* io, char* buffer, size_t buffer_size)
{
    size_t pos = 0;
    char ch;

    while (pos < buffer_size - 1)
    {
        size_t bytes_read;
        SAIL_TRY(io->tolerant_read(io->stream, &ch, 1, &bytes_read));

        if (bytes_read == 0)
        {
            break;
        }

        if (ch == '\n')
        {
            break;
        }

        if (ch != '\r')
        {
            buffer[pos++] = ch;
        }
    }

    buffer[pos] = '\0';

    return SAIL_OK;
}

sail_status_t hdr_private_read_header(struct sail_io* io, struct hdr_header* header)
{
    char line[1024];

    /* Initialize header. */
    memset(header, 0, sizeof(*header));
    header->exposure     = 1.0f;
    header->gamma        = 1.0f;
    header->colorcorr[0] = 1.0f;
    header->colorcorr[1] = 1.0f;
    header->colorcorr[2] = 1.0f;

    /* Read and verify signature. */
    SAIL_TRY(read_line(io, line, sizeof(line)));

    if (strncmp(line, "#?RADIANCE", 10) != 0 && strncmp(line, "#?RGBE", 6) != 0)
    {
        SAIL_LOG_ERROR("HDR: Invalid signature");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Read header lines until we find the empty line. */
    while (true)
    {
        SAIL_TRY(read_line(io, line, sizeof(line)));

        /* Empty line marks end of header, resolution line follows. */
        if (line[0] == '\0')
        {
            break;
        }

        /* Parse header variables. */
        if (strncmp(line, "EXPOSURE=", 9) == 0)
        {
            header->exposure = (float)atof(line + 9);
        }
        else if (strncmp(line, "GAMMA=", 6) == 0)
        {
            header->gamma = (float)atof(line + 6);
        }
        else if (strncmp(line, "VIEW=", 5) == 0)
        {
            SAIL_TRY(sail_strdup(line + 5, &header->view));
        }
        else if (strncmp(line, "PRIMARIES=", 10) == 0)
        {
            SAIL_TRY(sail_strdup(line + 10, &header->primaries));
        }
        else if (strncmp(line, "COLORCORR=", 10) == 0)
        {
#ifdef _MSC_VER
            sscanf_s(line + 10, "%f %f %f", &header->colorcorr[0], &header->colorcorr[1], &header->colorcorr[2]);
#else
            sscanf(line + 10, "%f %f %f", &header->colorcorr[0], &header->colorcorr[1], &header->colorcorr[2]);
#endif
        }
        else if (line[0] == '#' && line[1] == ' ')
        {
            /* Comment line - could be software. */
            if (header->software == NULL)
            {
                SAIL_TRY(sail_strdup(line + 2, &header->software));
            }
        }
    }

    /* Read dimensions (e.g., "-Y 512 +X 768"). */
    SAIL_TRY(read_line(io, line, sizeof(line)));

    char y_sign, x_sign;
    char y_axis, x_axis;
    int height, width;

#ifdef _MSC_VER
    if (sscanf_s(line, "%c%c %d %c%c %d", &y_sign, 1, &y_axis, 1, &height, &x_sign, 1, &x_axis, 1, &width) == 6)
    {
#else
    if (sscanf(line, "%c%c %d %c%c %d", &y_sign, &y_axis, &height, &x_sign, &x_axis, &width) == 6)
    {
#endif
        if (y_axis == 'Y' && x_axis == 'X')
        {
            header->height       = height;
            header->width        = width;
            header->y_increasing = (y_sign == '+');
            header->x_increasing = (x_sign == '+');

            if (header->width <= 0 || header->height <= 0)
            {
                SAIL_LOG_ERROR("HDR: Invalid dimensions %dx%d", header->width, header->height);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
            }

            return SAIL_OK;
        }
    }

    SAIL_LOG_ERROR("HDR: Invalid resolution line");
    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
}

sail_status_t hdr_private_write_header(struct sail_io* io,
                                       const struct hdr_header* header,
                                       const struct sail_meta_data_node* meta_data_node)
{
    /* Write signature. */
    const char* signature = "#?RADIANCE\n";
    SAIL_TRY(io->strict_write(io->stream, signature, strlen(signature)));

    /* Write software from meta_data if present. */
    const char* software = NULL;
    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data != NULL && node->meta_data->key == SAIL_META_DATA_SOFTWARE && node->meta_data->value != NULL
            && node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
        {
            software = sail_variant_to_string(node->meta_data->value);
            break;
        }
    }

    if (software != NULL)
    {
        char comment[1024];
        int len = snprintf(comment, sizeof(comment), "# %s\n", software);
        SAIL_TRY(io->strict_write(io->stream, comment, len));
    }
    else if (header->software != NULL)
    {
        char comment[1024];
        int len = snprintf(comment, sizeof(comment), "# %s\n", header->software);
        SAIL_TRY(io->strict_write(io->stream, comment, len));
    }

    /* Write format line. */
    const char* format = "FORMAT=32-bit_rle_rgbe\n";
    SAIL_TRY(io->strict_write(io->stream, format, strlen(format)));

    /* Write EXPOSURE if not default. */
    if (header->exposure != 1.0f)
    {
        char line[64];
        int len = snprintf(line, sizeof(line), "EXPOSURE=%20.10f\n", header->exposure);
        SAIL_TRY(io->strict_write(io->stream, line, len));
    }

    /* Write GAMMA if not default. */
    if (header->gamma != 1.0f)
    {
        char line[64];
        int len = snprintf(line, sizeof(line), "GAMMA=%f\n", header->gamma);
        SAIL_TRY(io->strict_write(io->stream, line, len));
    }

    /* Write VIEW if present. */
    if (header->view != NULL)
    {
        char line[1024];
        int len = snprintf(line, sizeof(line), "VIEW=%s\n", header->view);
        SAIL_TRY(io->strict_write(io->stream, line, len));
    }

    /* Write PRIMARIES if present. */
    if (header->primaries != NULL)
    {
        char line[1024];
        int len = snprintf(line, sizeof(line), "PRIMARIES=%s\n", header->primaries);
        SAIL_TRY(io->strict_write(io->stream, line, len));
    }

    /* Write COLORCORR if not default. */
    if (header->colorcorr[0] != 1.0f || header->colorcorr[1] != 1.0f || header->colorcorr[2] != 1.0f)
    {
        char line[128];
        int len = snprintf(line, sizeof(line), "COLORCORR=%f %f %f\n", header->colorcorr[0], header->colorcorr[1],
                           header->colorcorr[2]);
        SAIL_TRY(io->strict_write(io->stream, line, len));
    }

    /* Empty line marks end of header. */
    SAIL_TRY(io->strict_write(io->stream, "\n", 1));

    /* Write resolution line. */
    char resolution_line[64];
    int len = snprintf(resolution_line, sizeof(resolution_line), "%cY %d %cX %d\n", header->y_increasing ? '+' : '-',
                       header->height, header->x_increasing ? '+' : '-', header->width);

    SAIL_TRY(io->strict_write(io->stream, resolution_line, len));

    return SAIL_OK;
}

void hdr_private_rgbe_to_float(const uint8_t* rgbe, float* rgb)
{
    if (rgbe[3] == 0)
    {
        rgb[0] = rgb[1] = rgb[2] = 0.0f;
        return;
    }

    float f = ldexpf(1.0f, rgbe[3] - (128 + 8));
    rgb[0]  = rgbe[0] * f;
    rgb[1]  = rgbe[1] * f;
    rgb[2]  = rgbe[2] * f;
}

void hdr_private_float_to_rgbe(const float* rgb, uint8_t* rgbe)
{
    float max_val = rgb[0];
    if (rgb[1] > max_val)
        max_val = rgb[1];
    if (rgb[2] > max_val)
        max_val = rgb[2];

    if (max_val < 1e-32f)
    {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
        return;
    }

    int exponent;
    float mantissa = frexpf(max_val, &exponent) * 256.0f / max_val;

    rgbe[0] = (uint8_t)(rgb[0] * mantissa);
    rgbe[1] = (uint8_t)(rgb[1] * mantissa);
    rgbe[2] = (uint8_t)(rgb[2] * mantissa);
    rgbe[3] = (uint8_t)(exponent + 128);
}

static sail_status_t read_old_rle_scanline(struct sail_io* io, int width, uint8_t* scanline)
{
    uint8_t rgbe[4];
    int rshift = 0;
    int pos    = 0;

    while (pos < width)
    {
        size_t bytes_read;
        SAIL_TRY(io->tolerant_read(io->stream, rgbe, 4, &bytes_read));

        if (bytes_read != 4)
        {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
        }

        if (rgbe[0] == 1 && rgbe[1] == 1 && rgbe[2] == 1)
        {
            /* Run length encoded. */
            int count = ((int)rgbe[3]) << rshift;
            if (pos + count > width)
            {
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
            }

            for (int i = 0; i < count; i++)
            {
                memcpy(&scanline[pos * 4], &scanline[(pos - 1) * 4], 4);
                pos++;
            }

            rshift += 8;
        }
        else
        {
            memcpy(&scanline[pos * 4], rgbe, 4);
            pos++;
            rshift = 0;
        }
    }

    return SAIL_OK;
}

static sail_status_t read_new_rle_scanline(struct sail_io* io, int width, uint8_t* scanline)
{
    if (width < 8 || width > 32767)
    {
        return read_old_rle_scanline(io, width, scanline);
    }

    /* Read RLE header. */
    uint8_t header[4];
    size_t bytes_read;
    SAIL_TRY(io->tolerant_read(io->stream, header, 4, &bytes_read));

    if (bytes_read != 4)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Check for new RLE format. */
    if (header[0] != 2 || header[1] != 2 || (header[2] & 0x80))
    {
        /* Old format - seek back and read as old RLE. */
        SAIL_TRY(io->seek(io->stream, -4, SEEK_CUR));
        return read_old_rle_scanline(io, width, scanline);
    }

    /* Decode width from header. */
    int scanline_width = ((int)header[2] << 8) | header[3];

    if (scanline_width != width)
    {
        SAIL_LOG_ERROR("HDR: Scanline width mismatch");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Read each channel. */
    for (int channel = 0; channel < 4; channel++)
    {
        int pos = 0;

        while (pos < width)
        {
            uint8_t code;
            SAIL_TRY(io->tolerant_read(io->stream, &code, 1, &bytes_read));

            if (bytes_read != 1)
            {
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
            }

            if (code > 128)
            {
                /* Run length. */
                int count = code & 0x7F;
                uint8_t value;
                SAIL_TRY(io->tolerant_read(io->stream, &value, 1, &bytes_read));

                if (bytes_read != 1)
                {
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
                }

                for (int i = 0; i < count; i++)
                {
                    if (pos >= width)
                    {
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
                    }
                    scanline[pos * 4 + channel] = value;
                    pos++;
                }
            }
            else
            {
                /* Literal run. */
                int count = code;
                for (int i = 0; i < count; i++)
                {
                    if (pos >= width)
                    {
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
                    }

                    uint8_t value;
                    SAIL_TRY(io->tolerant_read(io->stream, &value, 1, &bytes_read));

                    if (bytes_read != 1)
                    {
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
                    }

                    scanline[pos * 4 + channel] = value;
                    pos++;
                }
            }
        }
    }

    return SAIL_OK;
}

sail_status_t hdr_private_read_scanline(struct sail_io* io, int width, float* scanline)
{
    uint8_t* rgbe_scanline = NULL;
    void* ptr;

    SAIL_TRY(sail_malloc(width * 4, &ptr));
    rgbe_scanline = ptr;

    sail_status_t status = read_new_rle_scanline(io, width, rgbe_scanline);

    if (status != SAIL_OK)
    {
        sail_free(rgbe_scanline);
        return status;
    }

    /* Convert RGBE to float RGB. */
    for (int x = 0; x < width; x++)
    {
        hdr_private_rgbe_to_float(&rgbe_scanline[x * 4], &scanline[x * 3]);
    }

    sail_free(rgbe_scanline);

    return SAIL_OK;
}

static sail_status_t write_new_rle_scanline(struct sail_io* io, int width, const uint8_t* scanline)
{
    /* Write RLE header. */
    uint8_t header[4] = {2, 2, (uint8_t)(width >> 8), (uint8_t)(width & 0xFF)};

    SAIL_TRY(io->strict_write(io->stream, header, 4));

    /* Encode each channel separately. */
    for (int channel = 0; channel < 4; channel++)
    {
        int pos = 0;

        while (pos < width)
        {
            /* Find run. */
            uint8_t value  = scanline[pos * 4 + channel];
            int run_length = 1;

            while (pos + run_length < width && run_length < 127)
            {
                if (scanline[(pos + run_length) * 4 + channel] != value)
                {
                    break;
                }
                run_length++;
            }

            if (run_length >= 4)
            {
                /* Write run length encoding. */
                uint8_t code = (uint8_t)(128 | run_length);
                SAIL_TRY(io->strict_write(io->stream, &code, sizeof(code)));
                SAIL_TRY(io->strict_write(io->stream, &value, sizeof(value)));
                pos += run_length;
            }
            else
            {
                /* Find literal run. */
                int literal_start  = pos;
                int literal_length = 0;

                while (pos < width && literal_length < 128)
                {
                    int next_run = 1;
                    if (pos + 1 < width)
                    {
                        value = scanline[pos * 4 + channel];
                        while (pos + next_run < width && next_run < 4)
                        {
                            if (scanline[(pos + next_run) * 4 + channel] != value)
                            {
                                break;
                            }
                            next_run++;
                        }
                    }

                    if (next_run >= 4)
                    {
                        break;
                    }

                    literal_length++;
                    pos++;
                }

                /* Write literal run. */
                uint8_t code = literal_length;
                SAIL_TRY(io->strict_write(io->stream, &code, sizeof(code)));

                for (int i = 0; i < literal_length; i++)
                {
                    uint8_t val = scanline[(literal_start + i) * 4 + channel];
                    SAIL_TRY(io->strict_write(io->stream, &val, sizeof(val)));
                }
            }
        }
    }

    return SAIL_OK;
}

sail_status_t hdr_private_write_scanline(struct sail_io* io, int width, const float* scanline, bool use_rle)
{
    uint8_t* rgbe_scanline = NULL;
    void* ptr;

    SAIL_TRY(sail_malloc(width * 4, &ptr));
    rgbe_scanline = ptr;

    /* Convert float RGB to RGBE. */
    for (int x = 0; x < width; x++)
    {
        hdr_private_float_to_rgbe(&scanline[x * 3], &rgbe_scanline[x * 4]);
    }

    sail_status_t status;

    if (use_rle && width >= 8 && width <= 32767)
    {
        status = write_new_rle_scanline(io, width, rgbe_scanline);
    }
    else
    {
        /* Write uncompressed. */
        status = io->strict_write(io->stream, rgbe_scanline, width * 4);
    }

    sail_free(rgbe_scanline);

    return status;
}

void hdr_private_destroy_header(struct hdr_header* header)
{
    if (header == NULL)
    {
        return;
    }

    sail_free(header->software);
    sail_free(header->view);
    sail_free(header->primaries);

    header->software  = NULL;
    header->view      = NULL;
    header->primaries = NULL;
}

sail_status_t hdr_private_store_properties(const struct hdr_header* header, struct sail_hash_map* special_properties)
{
    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    /* Store exposure (always, as it's important for HDR). */
    SAIL_LOG_TRACE("HDR: Storing exposure=%f", header->exposure);
    sail_set_variant_float(variant, header->exposure);
    sail_put_hash_map(special_properties, "hdr-exposure", variant);

    /* Store gamma (always, as it's important for HDR). */
    SAIL_LOG_TRACE("HDR: Storing gamma=%f", header->gamma);
    sail_set_variant_float(variant, header->gamma);
    sail_put_hash_map(special_properties, "hdr-gamma", variant);

    /* Store view. */
    if (header->view != NULL)
    {
        sail_set_variant_string(variant, header->view);
        sail_put_hash_map(special_properties, "hdr-view", variant);
    }

    /* Store primaries. */
    if (header->primaries != NULL)
    {
        sail_set_variant_string(variant, header->primaries);
        sail_put_hash_map(special_properties, "hdr-primaries", variant);
    }

    /* Store color correction. */
    if (header->colorcorr[0] != 1.0f)
    {
        sail_set_variant_float(variant, header->colorcorr[0]);
        sail_put_hash_map(special_properties, "hdr-colorcorr-1", variant);
    }
    if (header->colorcorr[1] != 1.0f)
    {
        sail_set_variant_float(variant, header->colorcorr[1]);
        sail_put_hash_map(special_properties, "hdr-colorcorr-2", variant);
    }
    if (header->colorcorr[2] != 1.0f)
    {
        sail_set_variant_float(variant, header->colorcorr[2]);
        sail_put_hash_map(special_properties, "hdr-colorcorr-3", variant);
    }

    sail_destroy_variant(variant);

    return SAIL_OK;
}

sail_status_t hdr_private_fetch_properties(const struct sail_hash_map* special_properties, struct hdr_header* header)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    const struct sail_variant* variant;

    /* Get exposure. */
    variant = sail_hash_map_value(special_properties, "hdr-exposure");
    if (variant != NULL && (variant->type == SAIL_VARIANT_TYPE_FLOAT || variant->type == SAIL_VARIANT_TYPE_DOUBLE))
    {
        header->exposure = sail_variant_to_float(variant);
        /* Range check: exposure should be positive. */
        if (header->exposure <= 0.0f)
        {
            header->exposure = 1.0f;
        }
    }

    /* Get gamma. */
    variant = sail_hash_map_value(special_properties, "hdr-gamma");
    if (variant != NULL && (variant->type == SAIL_VARIANT_TYPE_FLOAT || variant->type == SAIL_VARIANT_TYPE_DOUBLE))
    {
        header->gamma = sail_variant_to_float(variant);
        /* Range check: gamma should be positive. */
        if (header->gamma <= 0.0f)
        {
            header->gamma = 1.0f;
        }
    }

    /* Get view. */
    variant = sail_hash_map_value(special_properties, "hdr-view");
    if (variant != NULL && variant->type == SAIL_VARIANT_TYPE_STRING)
    {
        SAIL_TRY(sail_strdup(sail_variant_to_string(variant), &header->view));
    }

    /* Get primaries. */
    variant = sail_hash_map_value(special_properties, "hdr-primaries");
    if (variant != NULL && variant->type == SAIL_VARIANT_TYPE_STRING)
    {
        SAIL_TRY(sail_strdup(sail_variant_to_string(variant), &header->primaries));
    }

    /* Get color correction. */
    variant = sail_hash_map_value(special_properties, "hdr-colorcorr-1");
    if (variant != NULL && (variant->type == SAIL_VARIANT_TYPE_FLOAT || variant->type == SAIL_VARIANT_TYPE_DOUBLE))
    {
        header->colorcorr[0] = sail_variant_to_float(variant);
    }

    variant = sail_hash_map_value(special_properties, "hdr-colorcorr-2");
    if (variant != NULL && (variant->type == SAIL_VARIANT_TYPE_FLOAT || variant->type == SAIL_VARIANT_TYPE_DOUBLE))
    {
        header->colorcorr[1] = sail_variant_to_float(variant);
    }

    variant = sail_hash_map_value(special_properties, "hdr-colorcorr-3");
    if (variant != NULL && (variant->type == SAIL_VARIANT_TYPE_FLOAT || variant->type == SAIL_VARIANT_TYPE_DOUBLE))
    {
        header->colorcorr[2] = sail_variant_to_float(variant);
    }

    return SAIL_OK;
}

bool hdr_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct hdr_write_context* write_ctx = user_data;

    if (strcmp(key, "hdr-rle-compression") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val       = (value->type == SAIL_VARIANT_TYPE_INT) ? (unsigned)sail_variant_to_int(value)
                                                                        : sail_variant_to_unsigned_int(value);
            write_ctx->use_rle = (val != 0);
            SAIL_LOG_TRACE("HDR: rle-compression=%d", write_ctx->use_rle);
        }
        else
        {
            SAIL_LOG_ERROR("HDR: 'hdr-rle-compression' must be an integer");
        }
    }
    else if (strcmp(key, "hdr-y-direction") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "increasing") == 0 || strcmp(str_value, "+") == 0)
            {
                write_ctx->header->y_increasing = true;
            }
            else if (strcmp(str_value, "decreasing") == 0 || strcmp(str_value, "-") == 0)
            {
                write_ctx->header->y_increasing = false;
            }
            SAIL_LOG_TRACE("HDR: y-direction=%s", str_value);
        }
        else
        {
            SAIL_LOG_ERROR("HDR: 'hdr-y-direction' must be a string");
        }
    }
    else if (strcmp(key, "hdr-x-direction") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "increasing") == 0 || strcmp(str_value, "+") == 0)
            {
                write_ctx->header->x_increasing = true;
            }
            else if (strcmp(str_value, "decreasing") == 0 || strcmp(str_value, "-") == 0)
            {
                write_ctx->header->x_increasing = false;
            }
            SAIL_LOG_TRACE("HDR: x-direction=%s", str_value);
        }
        else
        {
            SAIL_LOG_ERROR("HDR: 'hdr-x-direction' must be a string");
        }
    }
    else if (strcmp(key, "hdr-exposure") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            float val = sail_variant_to_float(value);
            /* Range check: exposure should be positive. */
            write_ctx->header->exposure = (val > 0.0f) ? val : 1.0f;
            SAIL_LOG_TRACE("HDR: exposure=%f", write_ctx->header->exposure);
        }
        else
        {
            SAIL_LOG_ERROR("HDR: 'hdr-exposure' must be a float or double");
        }
    }
    else if (strcmp(key, "hdr-gamma") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            float val = sail_variant_to_float(value);
            /* Range check: gamma should be positive. */
            write_ctx->header->gamma = (val > 0.0f) ? val : 1.0f;
            SAIL_LOG_TRACE("HDR: gamma=%f", write_ctx->header->gamma);
        }
        else
        {
            SAIL_LOG_ERROR("HDR: 'hdr-gamma' must be a float or double");
        }
    }

    return true;
}
