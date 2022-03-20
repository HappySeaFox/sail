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

#ifndef SAIL_JPEG_HELPERS_H
#define SAIL_JPEG_HELPERS_H

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>

#include <jpeglib.h>

#include "common.h"
#include "export.h"

struct sail_meta_data_node;
struct sail_resolution;

struct jpeg_private_my_error_context {
    struct jpeg_error_mgr jpeg_error_mgr;
    jmp_buf setjmp_buffer;
};

SAIL_HIDDEN void jpeg_private_my_output_message(j_common_ptr cinfo);

SAIL_HIDDEN void jpeg_private_my_error_exit(j_common_ptr cinfo);

SAIL_HIDDEN enum SailPixelFormat jpeg_private_color_space_to_pixel_format(J_COLOR_SPACE color_space);

SAIL_HIDDEN J_COLOR_SPACE jpeg_private_pixel_format_to_color_space(enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_status_t jpeg_private_fetch_meta_data(struct jpeg_decompress_struct *decompress_context, struct sail_meta_data_node **last_meta_data_node);

SAIL_HIDDEN sail_status_t jpeg_private_write_meta_data(struct jpeg_compress_struct *compress_context, const struct sail_meta_data_node *meta_data_node);

#ifdef SAIL_HAVE_JPEG_ICCP
SAIL_HIDDEN sail_status_t jpeg_private_fetch_iccp(struct jpeg_decompress_struct *decompress_context, struct sail_iccp **iccp);
#endif

SAIL_HIDDEN sail_status_t jpeg_private_fetch_resolution(struct jpeg_decompress_struct *decompress_context, struct sail_resolution **resolution);

SAIL_HIDDEN sail_status_t jpeg_private_write_resolution(struct jpeg_compress_struct *compress_context, const struct sail_resolution *resolution);

SAIL_HIDDEN bool jpeg_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);

#endif
