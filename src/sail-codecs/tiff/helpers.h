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

#ifndef SAIL_TIFF_HELPERS_H
#define SAIL_TIFF_HELPERS_H

#include <stdarg.h>
#include <stdio.h>

#include <tiffio.h>

#include "common.h"
#include "error.h"
#include "export.h"

struct sail_meta_data_node;
struct sail_resolution;

SAIL_HIDDEN void tiff_private_my_error_fn(const char *module, const char *format, va_list ap);

SAIL_HIDDEN void tiff_private_my_warning_fn(const char *module, const char *format, va_list ap);

SAIL_HIDDEN enum SailCompression tiff_private_compression_to_sail_compression(int compression);

SAIL_HIDDEN sail_status_t tiff_private_sail_compression_to_compression(enum SailCompression compression, int *tiff_compression);

SAIL_HIDDEN enum SailPixelFormat tiff_private_bpp_to_pixel_format(int bpp);

SAIL_HIDDEN void tiff_private_zero_tiff_image(TIFFRGBAImage *img);

SAIL_HIDDEN sail_status_t tiff_private_fetch_iccp(TIFF *tiff, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t tiff_private_fetch_meta_data(TIFF *tiff, struct sail_meta_data_node ***last_meta_data_node);

SAIL_HIDDEN sail_status_t tiff_private_write_meta_data(TIFF *tiff, const struct sail_meta_data_node *meta_data_node);

SAIL_HIDDEN sail_status_t tiff_private_fetch_resolution(TIFF *tiff, struct sail_resolution **resolution);

SAIL_HIDDEN sail_status_t tiff_private_write_resolution(TIFF *tiff, const struct sail_resolution *resolution);

#endif
