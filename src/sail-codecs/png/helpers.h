/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#pragma once

#include <stdbool.h>

#include <png.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef PNG_APNG_SUPPORTED
/*
 * APNG fcTL dispose/blend constants.
 *
 * Upstream libpng (PNGv3, i.e. libpng 1.6.45+ / 1.8) exposes them as PNG_fcTL_DISPOSE_OP_* and
 * PNG_fcTL_BLEND_OP_*. The older unofficial SourceForge APNG patch named them PNG_DISPOSE_OP_* and
 * PNG_BLEND_OP_*. We use our own SAIL_PNG_* names at the call sites and bind them here to whichever
 * set the installed libpng headers actually provide, so both old and new libpng build unchanged.
 */
#ifdef PNG_fcTL_DISPOSE_OP_NONE
#define SAIL_PNG_DISPOSE_OP_NONE       PNG_fcTL_DISPOSE_OP_NONE
#define SAIL_PNG_DISPOSE_OP_BACKGROUND PNG_fcTL_DISPOSE_OP_BACKGROUND
#define SAIL_PNG_DISPOSE_OP_PREVIOUS   PNG_fcTL_DISPOSE_OP_PREVIOUS
#define SAIL_PNG_BLEND_OP_SOURCE       PNG_fcTL_BLEND_OP_SOURCE
#define SAIL_PNG_BLEND_OP_OVER         PNG_fcTL_BLEND_OP_OVER
#else
#define SAIL_PNG_DISPOSE_OP_NONE       PNG_DISPOSE_OP_NONE
#define SAIL_PNG_DISPOSE_OP_BACKGROUND PNG_DISPOSE_OP_BACKGROUND
#define SAIL_PNG_DISPOSE_OP_PREVIOUS   PNG_DISPOSE_OP_PREVIOUS
#define SAIL_PNG_BLEND_OP_SOURCE       PNG_BLEND_OP_SOURCE
#define SAIL_PNG_BLEND_OP_OVER         PNG_BLEND_OP_OVER
#endif
#endif

struct sail_hash_map;
struct sail_iccp;
struct sail_meta_data_node;
struct sail_palette;
struct sail_resolution;
struct sail_variant;

SAIL_HIDDEN void png_private_my_error_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN void png_private_my_warning_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN void* png_private_my_malloc_fn(png_structp png_ptr, png_size_t size);

SAIL_HIDDEN void png_private_my_free_fn(png_structp png_ptr, void* ptr);

SAIL_HIDDEN enum SailPixelFormat png_private_png_color_type_to_pixel_format(int color_type, int bit_depth);

SAIL_HIDDEN sail_status_t png_private_pixel_format_to_png_color_type(enum SailPixelFormat pixel_format,
                                                                     int* color_type,
                                                                     int* bit_depth);

SAIL_HIDDEN sail_status_t png_private_fetch_meta_data(png_structp png_ptr,
                                                      png_infop info_ptr,
                                                      struct sail_meta_data_node** target_meta_data_node);

SAIL_HIDDEN sail_status_t png_private_write_meta_data(png_structp png_ptr,
                                                      png_infop info_ptr,
                                                      const struct sail_meta_data_node* meta_data_node);

SAIL_HIDDEN sail_status_t png_private_fetch_iccp(png_structp png_ptr, png_infop info_ptr, struct sail_iccp** iccp);

SAIL_HIDDEN sail_status_t png_private_fetch_palette(png_structp png_ptr,
                                                    png_infop info_ptr,
                                                    struct sail_palette** palette);

#ifdef PNG_APNG_SUPPORTED
SAIL_HIDDEN sail_status_t png_private_blend_source(
    void* dst_raw, unsigned dst_offset, const void* src_raw, unsigned src_width, unsigned bytes_per_pixel);

SAIL_HIDDEN sail_status_t png_private_blend_over(
    void* dst_raw, unsigned dst_offset, const void* src_raw, unsigned width, enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_status_t png_private_skip_hidden_frame(
    unsigned bytes_per_line, unsigned height, png_structp png_ptr, png_infop info_ptr, void** row);

SAIL_HIDDEN sail_status_t png_private_alloc_rows(png_bytep** A, unsigned row_length, unsigned height);

SAIL_HIDDEN void png_private_destroy_rows(png_bytep** A, unsigned height);

SAIL_HIDDEN sail_status_t png_private_store_num_frames_and_plays(png_structp png_ptr,
                                                                 png_infop info_ptr,
                                                                 struct sail_hash_map* special_properties);
#endif

SAIL_HIDDEN sail_status_t png_private_fetch_resolution(png_structp png_ptr,
                                                       png_infop info_ptr,
                                                       struct sail_resolution** resolution);

SAIL_HIDDEN sail_status_t png_private_write_resolution(png_structp png_ptr,
                                                       png_infop info_ptr,
                                                       const struct sail_resolution* resolution);

SAIL_HIDDEN bool png_private_tuning_key_value_callback(const char* key,
                                                       const struct sail_variant* value,
                                                       void* user_data);

SAIL_HIDDEN unsigned png_private_read_variant_uint(const struct sail_variant* value);
