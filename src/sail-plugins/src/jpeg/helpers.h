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

struct my_error_context {
    struct jpeg_error_mgr jpeg_error_mgr;
    jmp_buf setjmp_buffer;
};

SAIL_HIDDEN void my_output_message(j_common_ptr cinfo);

SAIL_HIDDEN void my_error_exit(j_common_ptr cinfo);

SAIL_HIDDEN enum SailPixelFormat color_space_to_pixel_format(J_COLOR_SPACE color_space);

SAIL_HIDDEN J_COLOR_SPACE pixel_format_to_color_space(enum SailPixelFormat pixel_format);

SAIL_HIDDEN bool jpeg_supported_pixel_format(enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_error_t convert_cmyk(unsigned char *pixels_source, unsigned char *pixels_target, unsigned width, int target_pixel_format);

#endif
