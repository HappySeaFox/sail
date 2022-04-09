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

/*
 * Private functions.
 */

static sail_status_t skip_raw_profile_header(const char *data, const char **start) {

    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(start);

    char key[16];
    int n;
    char c;
    int bytes_consumed;

    /* Skip "\nexif\n    1234 " before the actual HEX-encoded data. */
#ifdef _MSC_VER
    if (sscanf_s(data, "%s %d %c %n", key, (unsigned)sizeof(key), &n, &c, 1, &bytes_consumed) != 3) {
#else
    if (sscanf(data, "%s %d %c %n", key, &n, &c, &bytes_consumed) != 3) {
#endif
        SAIL_LOG_ERROR("PNG: Failed to parse raw profile header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    *start = data + bytes_consumed - 1;

    return SAIL_OK;
}

static sail_status_t write_raw_profile_header(char *str, size_t str_size, enum SailMetaData key, size_t hex_data_length) {

    SAIL_CHECK_PTR(str);

    const char *key_str;

    switch (key) {
        case SAIL_META_DATA_EXIF: key_str = "exif"; break;
        case SAIL_META_DATA_IPTC: key_str = "iptc"; break;
        case SAIL_META_DATA_XMP:  key_str = "xmp";  break;

        default: {
            SAIL_LOG_ERROR("PNG: Cannot save '%s' meta data key as a raw profile", sail_meta_data_to_string(key));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
        }
    }

    /* Write "\nexif\n    1234\n" before the actual HEX-encoded data. */
#ifdef _MSC_VER
    if (sprintf_s(str, str_size, "\n%s\n    %u\n", key_str, (unsigned)hex_data_length) < 0) {
#else
    (void)str_size;
    if (sprintf(str, "\n%s\n    %u\n", key_str, (unsigned)hex_data_length) < 0) {
#endif
        SAIL_LOG_ERROR("PNG: Failed to save raw profile header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    return SAIL_OK;
}

static sail_status_t hex_string_to_meta_data_node(const char *hex_str, enum SailMetaData key, struct sail_meta_data_node **meta_data_node) {

    const char *start;
    SAIL_TRY(skip_raw_profile_header(hex_str, &start));

    void *data;
    size_t data_size;
    SAIL_TRY(sail_hex_string_to_data(start, &data, &data_size));

    struct sail_meta_data_node *meta_data_node_local;

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                        /* cleanup */ sail_free(data));
    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(key, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_free(data));
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node_local->meta_data->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local),
                                      free(data));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node_local->meta_data->value, data, data_size),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local),
                                      free(data));

    meta_data_node_local->meta_data->key = key;

    sail_free(data);

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

/*
 * Public functions.
 */

void png_private_my_error_fn(png_structp png_ptr, png_const_charp text) {

    (void)png_ptr;

    SAIL_LOG_ERROR("PNG: %s", text);
}

void png_private_my_warning_fn(png_structp png_ptr, png_const_charp text) {

    (void)png_ptr;

    SAIL_LOG_WARNING("PNG: %s", text);
}

enum SailPixelFormat png_private_png_color_type_to_pixel_format(int color_type, int bit_depth) {

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
                case 8:  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
                case 16: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;
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

sail_status_t png_private_pixel_format_to_png_color_type(enum SailPixelFormat pixel_format, int *color_type, int *bit_depth) {

    SAIL_CHECK_PTR(color_type);
    SAIL_CHECK_PTR(bit_depth);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 1;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 2;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 4;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED: {
            *color_type = PNG_COLOR_TYPE_PALETTE;
            *bit_depth = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE: {
            *color_type = PNG_COLOR_TYPE_GRAY;
            *bit_depth = 1;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE: {
            *color_type = PNG_COLOR_TYPE_GRAY;
            *bit_depth = 2;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE: {
            *color_type = PNG_COLOR_TYPE_GRAY;
            *bit_depth = 4;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: {
            *color_type = PNG_COLOR_TYPE_GRAY;
            *bit_depth = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: {
            *color_type = PNG_COLOR_TYPE_GRAY;
            *bit_depth = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: {
            *color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
            *bit_depth = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
            *color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
            *bit_depth = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR: {
            *color_type = PNG_COLOR_TYPE_RGB;
            *bit_depth = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR: {
            *color_type = PNG_COLOR_TYPE_RGB;
            *bit_depth = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: {
            *color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            *bit_depth = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
            *color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            *bit_depth = 16;
            return SAIL_OK;
        }
        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }
}

sail_status_t png_private_fetch_meta_data(png_structp png_ptr, png_infop info_ptr, struct sail_meta_data_node **target_meta_data_node) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(target_meta_data_node);

    struct sail_meta_data_node **last_meta_data_node = target_meta_data_node;
    png_textp lines;
    int num_text;

    png_get_text(png_ptr, info_ptr, &lines, &num_text);

    for (int i = 0; i < num_text; i++) {
        struct sail_meta_data_node *meta_data_node;

        /* Legacy EXIF and friends. */
        if (strcmp(lines[i].key, "Raw profile type exif") == 0) {
            SAIL_TRY(hex_string_to_meta_data_node(lines[i].text, SAIL_META_DATA_EXIF, &meta_data_node));
        } else if (strcmp(lines[i].key, "Raw profile type iptc") == 0) {
            SAIL_TRY(hex_string_to_meta_data_node(lines[i].text, SAIL_META_DATA_IPTC, &meta_data_node));
        } else if (strcmp(lines[i].key, "Raw profile type xmp") == 0) {
            SAIL_TRY(hex_string_to_meta_data_node(lines[i].text, SAIL_META_DATA_XMP, &meta_data_node));
        } else {
            enum SailMetaData meta_data;

            if (strcmp(lines[i].key, "XML:com.adobe.xmp") == 0) {
                meta_data = SAIL_META_DATA_XMP;
            } else {
                meta_data = sail_meta_data_from_string(lines[i].key);
            }

            SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

            if (meta_data == SAIL_META_DATA_UNKNOWN) {
                SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_unknown_key(lines[i].key, &meta_data_node->meta_data),
                                    /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            } else {
                SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(meta_data, &meta_data_node->meta_data),
                                    /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            }

            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, lines[i].text),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
        }

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;
    }

    png_bytep exif;
    png_uint_32 exif_length;

    if (png_get_eXIf_1(png_ptr, info_ptr, &exif_length, &exif) != 0) {
        struct sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_EXIF, &meta_data_node->meta_data),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value, exif, exif_length),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;
    }

    return SAIL_OK;
}

sail_status_t png_private_write_meta_data(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_data_node *meta_data_node) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);

    /* Count PNG lines. */
    unsigned count = 0;

    for (const struct sail_meta_data_node *meta_data_node_it = meta_data_node; meta_data_node_it != NULL; meta_data_node_it = meta_data_node_it->next) {
        count++;
    }

    if (count > 0) {
        void *ptr;
        SAIL_TRY(sail_malloc(count * sizeof(png_text), &ptr));
        png_text *lines = ptr;

        /* Indexes in 'lines' that must be freed. 1 = free, 0 = don't free. */
        SAIL_TRY(sail_malloc(count * sizeof(bool), &ptr));
        int *lines_to_free = ptr;
        memset(lines_to_free, 0, count);

        unsigned index = 0;

        /* Build PNG lines. */
        for (; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
            const struct sail_meta_data *meta_data = meta_data_node->meta_data;

            if (meta_data->key == SAIL_META_DATA_EXIF) {
                if (meta_data->value->type == SAIL_VARIANT_TYPE_DATA) {
                    /* Skip "Exif\0\0" if any. */
                    if (meta_data->value->size >= 4 && memcmp(sail_variant_to_data(meta_data->value), "Exif", 4) == 0) {
                        SAIL_LOG_DEBUG("PNG: Saving raw EXIF %u bytes long w/o header", (unsigned)meta_data->value->size - 6);
                        png_set_eXIf_1(png_ptr, info_ptr, (png_uint_32)meta_data->value->size - 6,
                                        ((png_bytep)sail_variant_to_data(meta_data->value)) + 6);
                    } else {
                        SAIL_LOG_DEBUG("PNG: Saving raw EXIF %u bytes long", (unsigned)meta_data->value->size);
                        png_set_eXIf_1(png_ptr, info_ptr, (png_uint_32)meta_data->value->size, meta_data->value->value);
                    }
                } else {
                    SAIL_LOG_ERROR("PNG: EXIF meta data must have DATA type");
                }
            } else {
                const char *meta_data_key = NULL;
                char *meta_data_value = NULL;

                if (meta_data->key == SAIL_META_DATA_UNKNOWN) {
                    meta_data_key = meta_data->key_unknown;
                    meta_data_value = (char *)meta_data->value;
                } else {
                    if (meta_data->key == SAIL_META_DATA_IPTC) {
                        meta_data_key = "Raw profile type iptc";

                        char raw_profile_header[64];
                        SAIL_TRY_OR_EXECUTE(write_raw_profile_header(raw_profile_header,
                                                                        sizeof(raw_profile_header),
                                                                        meta_data->key,
                                                                        (meta_data->value->size - 1) * 2),
                                            /* on error */ continue);

                        char *hex_string;
                        SAIL_TRY_OR_EXECUTE(sail_data_to_hex_string(sail_variant_to_data(meta_data->value), meta_data->value->size, &hex_string),
                                            /* on error */ continue);

                        SAIL_TRY_OR_EXECUTE(sail_concat(&meta_data_value, 2, raw_profile_header, hex_string),
                                            /* on error */ sail_free(hex_string); continue);
                        sail_free(hex_string);

                        lines_to_free[index] = 1;
                    } else {
                        meta_data_key   = sail_meta_data_to_string(meta_data->key);
                        meta_data_value = sail_variant_to_string(meta_data->value);
                    }
                }

                lines[index].compression = PNG_TEXT_COMPRESSION_zTXt;
                lines[index].key         = (char *)meta_data_key;
                lines[index].text        = meta_data_value;

                index++;
            }
        }

        png_set_text(png_ptr, info_ptr, lines, index);

        /* Cleanup. */
        for (unsigned i = 0; i < index; i++) {
            if (lines_to_free[i] == 1) {
                sail_free(lines[i].text);
            }
        }

        sail_free(lines_to_free);
        sail_free(lines);
    }

    return SAIL_OK;
}

sail_status_t png_private_fetch_iccp(png_structp png_ptr, png_infop info_ptr, struct sail_iccp **iccp) {

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
        SAIL_TRY(sail_alloc_iccp_from_data(data, data_length, iccp));
        SAIL_LOG_DEBUG("PNG: Found ICC profile '%s' %u bytes long", name, data_length);
    } else {
        SAIL_LOG_DEBUG("PNG: ICC profile is not found");
    }

    return SAIL_OK;
}

sail_status_t png_private_fetch_palette(png_structp png_ptr, png_infop info_ptr, struct sail_palette **palette) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(palette);

    png_colorp png_palette;
    int png_palette_color_count;

    if (png_get_PLTE(png_ptr, info_ptr, &png_palette, &png_palette_color_count) == 0) {
        SAIL_LOG_ERROR("PNG: The indexed image has no palette");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
    }

    png_bytep transparency = NULL;
    int transparency_length = 0;

#ifdef PNG_tRNS_SUPPORTED
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0) {
        if (png_get_tRNS(png_ptr, info_ptr, &transparency, &transparency_length, NULL) == 0) {
            SAIL_LOG_ERROR("PNG: The image has invalid transparency block");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
        }
    }
#endif

    if (transparency == NULL) {
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, png_palette_color_count, palette));
    } else {
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP32_RGBA, png_palette_color_count, palette));
    }

    unsigned char *palette_ptr = (*palette)->data;

    for (int i = 0; i < png_palette_color_count; i++) {
        *palette_ptr++ = png_palette[i].red;
        *palette_ptr++ = png_palette[i].green;
        *palette_ptr++ = png_palette[i].blue;

        if (transparency != NULL) {
            *palette_ptr++ = (i < transparency_length) ? transparency[i] : 255;
        }
    }

    return SAIL_OK;
}

#ifdef PNG_APNG_SUPPORTED
sail_status_t png_private_blend_source(void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned src_length, unsigned bytes_per_pixel) {

    SAIL_CHECK_PTR(dst_raw);
    SAIL_CHECK_PTR(src_raw);

    memcpy((uint8_t *)dst_raw + dst_offset * bytes_per_pixel,
            src_raw,
            (size_t)src_length * bytes_per_pixel);

    return SAIL_OK;
}

sail_status_t png_private_blend_over(void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned width, unsigned bytes_per_pixel) {

    SAIL_CHECK_PTR(src_raw);
    SAIL_CHECK_PTR(dst_raw);

    if (bytes_per_pixel == 4) {
        const uint8_t *src = src_raw;
        uint8_t *dst = (uint8_t *)dst_raw + dst_offset * bytes_per_pixel;

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
        uint16_t *dst = (uint16_t *)((uint8_t *)dst_raw + dst_offset * bytes_per_pixel);

        while (width--) {
            const double src_a = *(src+3) / 65535.0;
            const double dst_a = *(dst+3) / 65535.0;

            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint16_t)((src_a + (1-src_a) * dst_a) * 65535);         src++; dst++;
        }
    } else {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    return SAIL_OK;
}

sail_status_t png_private_skip_hidden_frame(unsigned bytes_per_line, unsigned height, png_structp png_ptr, png_infop info_ptr, void **row) {

    SAIL_CHECK_PTR(png_ptr);
    SAIL_CHECK_PTR(info_ptr);
    SAIL_CHECK_PTR(row);

    SAIL_TRY(sail_malloc(bytes_per_line, row));

    png_read_frame_head(png_ptr, info_ptr);

    for (unsigned i = 0; i < height; i++) {
        png_read_row(png_ptr, (png_bytep)(*row), NULL);
    }

    sail_free(*row);
    *row = NULL;

    return SAIL_OK;
}

sail_status_t png_private_alloc_rows(png_bytep **A, unsigned row_length, unsigned height) {

    void *ptr;
    SAIL_TRY(sail_malloc(height * sizeof(png_bytep), &ptr));

    *A = ptr;

    for (unsigned row = 0; row < height; row++) {
        (*A)[row] = NULL;
    }

    for (unsigned row = 0; row < height; row++) {
        SAIL_TRY(sail_malloc(row_length, &ptr));
        (*A)[row] = ptr;

        memset((*A)[row], 0, row_length);
    }

    return SAIL_OK;
}

void png_private_destroy_rows(png_bytep **A, unsigned height) {

    if (*A == NULL) {
        return;
    }

    for (unsigned row = 0; row < height; row++) {
        sail_free((*A)[row]);
    }

    sail_free(*A);
    *A = NULL;
}
#endif

sail_status_t png_private_fetch_resolution(png_structp png_ptr, png_infop info_ptr, struct sail_resolution **resolution) {

    SAIL_CHECK_PTR(resolution);

    int unit = PNG_RESOLUTION_UNKNOWN;
    unsigned x = 0, y = 0;

    png_get_pHYs(png_ptr, info_ptr, &x, &y, &unit);

    /* Resolution information is not valid. */
    if (x == 0 && y == 0) {
        return SAIL_OK;
    }

    SAIL_TRY(sail_alloc_resolution(resolution));

    switch (unit) {
        case PNG_RESOLUTION_METER: {
            (*resolution)->unit = SAIL_RESOLUTION_UNIT_METER;
            break;
        }
    }

    (*resolution)->x = (float)x;
    (*resolution)->y = (float)y;

    return SAIL_OK;
}

sail_status_t png_private_write_resolution(png_structp png_ptr, png_infop info_ptr, const struct sail_resolution *resolution) {

    /* Not an error. */
    if (resolution == NULL) {
        return SAIL_OK;
    }

    int unit;

    /* PNG supports just meters. */
    switch (resolution->unit) {
        case SAIL_RESOLUTION_UNIT_METER: {
            unit = PNG_RESOLUTION_METER;
            break;
        }
        default: {
            unit = PNG_RESOLUTION_UNKNOWN;
            break;
        }
    }

    png_set_pHYs(png_ptr, info_ptr, (unsigned)resolution->x, (unsigned)resolution->y, unit);

    return SAIL_OK;
}

bool png_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    png_structp png_ptr = user_data;

    if (strcmp(key, "png-filter") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_STRING) {
            const char *str_value = sail_variant_to_string(value);

            int filters = 0;

            struct sail_string_node *string_node_filters;
            SAIL_TRY_OR_EXECUTE(sail_split_into_string_node_chain(str_value, &string_node_filters),
                                /* on error */ return true);

            for (const struct sail_string_node *node = string_node_filters; node != NULL; node = node->next) {
                if (strcmp(node->string, "none") == 0) {
                    SAIL_LOG_TRACE("PNG: Adding NONE filter");
                    filters |= PNG_FILTER_NONE;
                } else if (strcmp(node->string, "sub") == 0) {
                    SAIL_LOG_TRACE("PNG: Adding SUB filter");
                    filters |= PNG_FILTER_SUB;
                } else if (strcmp(node->string, "up") == 0) {
                    SAIL_LOG_TRACE("PNG: Adding UP filter");
                    filters |= PNG_FILTER_UP;
                } else if (strcmp(node->string, "avg") == 0) {
                    SAIL_LOG_TRACE("PNG: Adding AVG filter");
                    filters |= PNG_FILTER_AVG;
                } else if (strcmp(node->string, "paeth") == 0) {
                    SAIL_LOG_TRACE("PNG: Adding PAETH filter");
                    filters |= PNG_FILTER_PAETH;
                }
            }

            sail_destroy_string_node_chain(string_node_filters);

            png_set_filter(png_ptr, 0, filters);
        }
    }

    return true;
}
