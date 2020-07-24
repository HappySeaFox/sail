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
struct sail_meta_entry_node;
struct sail_palette;

SAIL_HIDDEN void my_error_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN void my_warning_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN enum SailPixelFormat png_color_type_to_pixel_format(int color_type, int bit_depth);

SAIL_HIDDEN sail_error_t pixel_format_to_png_color_type(enum SailPixelFormat pixel_format, int *color_type, int *bit_depth);

SAIL_HIDDEN sail_error_t supported_read_output_pixel_format(enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_error_t read_png_text(png_structp png_ptr, png_infop info_ptr, struct sail_meta_entry_node **target_meta_entry_node);

SAIL_HIDDEN sail_error_t write_png_text(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_entry_node *meta_entry_node);

SAIL_HIDDEN sail_error_t fetch_iccp(png_structp png_ptr, png_infop info_ptr, struct sail_iccp **iccp);

SAIL_HIDDEN sail_error_t fetch_palette(png_structp png_ptr, png_infop info_ptr, struct sail_palette **palette);

#ifdef PNG_APNG_SUPPORTED
SAIL_HIDDEN sail_error_t blend_source(unsigned bytes_per_pixel, void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned src_length);

SAIL_HIDDEN sail_error_t blend_over(unsigned bytes_per_pixel, unsigned width, const void *src_raw, void *dst_raw, unsigned dst_offset);

SAIL_HIDDEN sail_error_t skip_hidden_frame(unsigned bytes_per_line, unsigned height, png_structp png_ptr, png_infop info_ptr, void **row);

SAIL_HIDDEN sail_error_t alloc_rows(png_bytep **A, unsigned row_length, unsigned height);

SAIL_HIDDEN void destroy_rows(png_bytep **A, unsigned height);
#endif

#endif
