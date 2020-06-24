/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

void my_error_fn(png_structp png_ptr, png_const_charp text) {

    (void)png_ptr;

    SAIL_LOG_ERROR("PNG: %s", text);
}

void my_warning_fn(png_structp png_ptr, png_const_charp text) {

    (void)png_ptr;

    SAIL_LOG_WARNING("PNG: %s", text);
}

int png_color_type_to_pixel_format(int color_type, int bit_depth) {

    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY: {
            switch (bit_depth) {
                case 1:  return SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;
                case 2:  return SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;
                case 4:  return SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;
                case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
            }
            break;
        }

        case PNG_COLOR_TYPE_GRAY_ALPHA: {
            switch (bit_depth) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
            }
            break;
        }

        case PNG_COLOR_TYPE_PALETTE: {
            switch (bit_depth) {
                case 1: return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
                case 2: return SAIL_PIXEL_FORMAT_BPP2_INDEXED;
                case 4: return SAIL_PIXEL_FORMAT_BPP4_INDEXED;
                case 8: return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
            }
            break;
        }

        case PNG_COLOR_TYPE_RGB: {
            switch (bit_depth) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;
            }
            break;
        }

        case PNG_COLOR_TYPE_RGB_ALPHA: {
            switch (bit_depth) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                case 16: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
            }
            break;
        }
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

sail_error_t pixel_format_to_png_color_type(int pixel_format, int *color_type, int *bit_depth) {

    SAIL_CHECK_PTR(color_type);
    SAIL_CHECK_PTR(bit_depth);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 1;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP2_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 2;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP4_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 4;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP8_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 8;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR: {
            *color_type = PNG_COLOR_TYPE_RGB;
            *bit_depth = 8;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR: {
            *color_type = PNG_COLOR_TYPE_RGB;
            *bit_depth = 16;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB: {
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:
            *color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            *bit_depth = 8;
            return 0;
        }

        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB: {
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:
            *color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            *bit_depth = 16;
            return 0;
        }
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

sail_error_t supported_read_output_pixel_format(int pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_SOURCE:
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: {
            return 0;
        }
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t supported_write_input_pixel_format(int pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
            return 0;
        }
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t supported_write_output_pixel_format(int pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_SOURCE: {
            return 0;
        }
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t read_png_text(png_structp png_ptr, png_infop info_ptr, struct sail_meta_entry_node **target_meta_entry_node) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(target_meta_entry_node);

#ifdef PNG_TEXT_SUPPORTED
    struct sail_meta_entry_node **last_meta_entry_node = target_meta_entry_node;

    png_textp lines;
    int num_text;

    png_get_text(png_ptr, info_ptr, &lines, &num_text);

    for (int i = 0; i < num_text; i++) {
        struct sail_meta_entry_node *meta_entry_node;

        SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup(lines[i].key, &meta_entry_node->key),
                            /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup_length(lines[i].text, strlen(lines[i].text), &meta_entry_node->value),
                            /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));

        *last_meta_entry_node = meta_entry_node;
        last_meta_entry_node = &meta_entry_node->next;
    }
#endif

    return 0;
}

sail_error_t write_png_text(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_entry_node *meta_entry_node) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);

#ifdef PNG_TEXT_SUPPORTED
    /* To avoid allocating dynamic arrays, allow only 32 text pairs. */
    png_text lines[32];
    int count = 0;

    /* Build PNG lines. */
    while (meta_entry_node != NULL && count < 32) {
        lines[count].compression = PNG_TEXT_COMPRESSION_zTXt;
        lines[count].key         = meta_entry_node->key;
        lines[count].text        = meta_entry_node->value;

        count++;
        meta_entry_node = meta_entry_node->next;
    }

    png_set_text(png_ptr, info_ptr, lines, count);
#else
    (void)meta_entry_node;
#endif

    return 0;
}

sail_error_t fetch_iccp(png_structp png_ptr, png_infop info_ptr, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(iccp);

    char *name;
    int compression;
    png_bytep data;
    unsigned data_length;

    bool ok = png_get_iCCP(png_ptr,
                           info_ptr,
                           &name,
                           &compression,
                           &data,
                           &data_length) == PNG_INFO_iCCP;

    if (ok) {
        SAIL_TRY(sail_alloc_iccp(iccp));

        (*iccp)->data = malloc(data_length);

        if ((*iccp)->data == NULL) {
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        memcpy((*iccp)->data, data, data_length);
        (*iccp)->data_length = data_length;

        SAIL_LOG_DEBUG("PNG: Found ICC profile '%s' %u bytes long", name, data_length);
    } else {
        SAIL_LOG_DEBUG("PNG: ICC profile is not found");
    }

    return 0;
}

#ifdef PNG_APNG_SUPPORTED

sail_error_t blend_source(unsigned bytes_per_pixel, void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned src_length) {

    SAIL_CHECK_PTR(dst_raw);
    SAIL_CHECK_PTR(src_raw);

    if (bytes_per_pixel == 4) {
        memcpy((uint8_t*)dst_raw + dst_offset, src_raw, src_length);
    } else if (bytes_per_pixel == 8) {
        memcpy((uint16_t*)dst_raw + dst_offset, src_raw, src_length);
    } else {
        return SAIL_UNSUPPORTED_BIT_DEPTH;
    }

    return 0;
}

sail_error_t blend_over(unsigned bytes_per_pixel, unsigned width, const void *src_raw, void *dst_raw, unsigned dst_offset) {

    SAIL_CHECK_PTR(src_raw);
    SAIL_CHECK_PTR(dst_raw);

    if (bytes_per_pixel == 4) {
        const uint8_t *src = src_raw;
        uint8_t *dst = (uint8_t *)dst_raw + dst_offset;

        while (width--) {
            const double src_a = *(src+3) / 255.0;
            const double dst_a = *(dst+3) / 255.0;

            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)((src_a + (1-src_a) * dst_a) * 255);           src++; dst++;
        }
    } else if (bytes_per_pixel == 8) {
        const uint16_t *src = src_raw;
        uint16_t *dst = (uint16_t *)dst_raw + dst_offset;

        while (width--) {
            const double src_a = *(src+3) / 65535.0;
            const double dst_a = *(dst+3) / 65535.0;

            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)((src_a + (1-src_a) * dst_a) * 65535);         src++; dst++;
        }
    } else {
        return SAIL_UNSUPPORTED_BIT_DEPTH;
    }

    return 0;
}

sail_error_t skip_hidden_frame(unsigned bytes_per_line, unsigned height, png_structp png_ptr, png_infop info_ptr, void **row) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(row);

    *row = malloc(bytes_per_line);

    if (*row == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    png_read_frame_head(png_ptr, info_ptr);

    for (unsigned i = 0; i < height; i++) {
        png_read_row(png_ptr, (png_bytep)(*row), NULL);
    }

    free(*row);
    *row = NULL;

    return 0;
}

sail_error_t alloc_rows(png_bytep **A, unsigned row_length, unsigned height) {

    *A = (png_bytep*)malloc(height * sizeof(png_bytep*));

    if (*A == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    for (unsigned row = 0; row < height; row++) {
        (*A)[row] = NULL;
    }

    for (unsigned row = 0; row < height; row++) {
        (*A)[row] = (png_bytep)malloc(row_length);

        if ((*A)[row] == NULL) {
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        memset((*A)[row], 0, row_length);
    }

    return 0;
}

void destroy_rows(png_bytep **A, unsigned height) {

    if (*A == NULL) {
        return;
    }

    for (unsigned row = 0; row < height; row++) {
        free((*A)[row]);
    }

    free(*A);
    *A = NULL;
}

#endif
