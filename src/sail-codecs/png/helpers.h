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

#ifndef SAIL_PNG_HELPERS_H
#define SAIL_PNG_HELPERS_H

#include <stdbool.h>
#include <stdio.h>

#include <png.h>

#include "common.h"
#include "error.h"
#include "export.h"

struct sail_iccp;
struct sail_meta_data_node;
struct sail_palette;
struct sail_resolution;
struct sail_variant;

SAIL_HIDDEN void png_private_my_error_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN void png_private_my_warning_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN enum SailPixelFormat png_private_png_color_type_to_pixel_format(int color_type, int bit_depth);

SAIL_HIDDEN sail_status_t png_private_pixel_format_to_png_color_type(enum SailPixelFormat pixel_format, int *color_type, int *bit_depth);

SAIL_HIDDEN sail_status_t png_private_fetch_meta_data(png_structp png_ptr, png_infop info_ptr, struct sail_meta_data_node **target_meta_data_node);

SAIL_HIDDEN sail_status_t png_private_write_meta_data(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_data_node *meta_data_node);

SAIL_HIDDEN sail_status_t png_private_fetch_iccp(png_structp png_ptr, png_infop info_ptr, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t png_private_fetch_palette(png_structp png_ptr, png_infop info_ptr, struct sail_palette **palette);

#ifdef PNG_APNG_SUPPORTED
SAIL_HIDDEN sail_status_t png_private_blend_source(void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned src_length, unsigned bytes_per_pixel);

SAIL_HIDDEN sail_status_t png_private_blend_over(void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned width, unsigned bytes_per_pixel);

SAIL_HIDDEN sail_status_t png_private_skip_hidden_frame(unsigned bytes_per_line, unsigned height, png_structp png_ptr, png_infop info_ptr, void **row);

SAIL_HIDDEN sail_status_t png_private_alloc_rows(png_bytep **A, unsigned row_length, unsigned height);

SAIL_HIDDEN void png_private_destroy_rows(png_bytep **A, unsigned height);
#endif

SAIL_HIDDEN sail_status_t png_private_fetch_resolution(png_structp png_ptr, png_infop info_ptr, struct sail_resolution **resolution);

SAIL_HIDDEN sail_status_t png_private_write_resolution(png_structp png_ptr, png_infop info_ptr, const struct sail_resolution *resolution);

SAIL_HIDDEN bool png_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);

#endif
