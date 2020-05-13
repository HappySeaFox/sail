/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

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
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
                case 32: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;
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
    struct sail_meta_entry_node *last_meta_entry_node = NULL;

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

        if (*target_meta_entry_node == NULL) {
            *target_meta_entry_node = last_meta_entry_node = meta_entry_node;
        } else {
            last_meta_entry_node->next = meta_entry_node;
            last_meta_entry_node = meta_entry_node;
        }
    }
#endif

    return 0;
}

sail_error_t write_png_text(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_entry_node *meta_entry_node) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);

    /* To avoid allocating arrays for MSVC (which still doesn't support VLAs), allow only 32 text pairs. */
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

    return 0;
}
