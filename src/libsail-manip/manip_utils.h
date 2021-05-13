/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_MANIP_UTILS_H
#define SAIL_MANIP_UTILS_H

#include <stdint.h>

#include "error.h"
#include "export.h"

struct sail_conversion_options;
struct sail_palette;

SAIL_HIDDEN sail_status_t get_palette_rgba32(const struct sail_palette *palette, unsigned index, sail_rgba32_t *rgba32);

SAIL_HIDDEN void spread_gray8_to_rgba32(uint8_t value, sail_rgba32_t *rgba32);

SAIL_HIDDEN void spread_gray16_to_rgba32(uint16_t value, sail_rgba32_t *rgba32);

SAIL_HIDDEN void spread_gray8_to_rgba64(uint8_t value, sail_rgba64_t *rgba64);

SAIL_HIDDEN void spread_gray16_to_rgba64(uint16_t value, sail_rgba64_t *rgba64);

SAIL_HIDDEN void fill_gray8_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_gray8_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_gray16_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_gray16_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgb24_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, int r, int g, int b, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgb24_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, int r, int g, int b, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgb48_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, int r, int g, int b, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgb48_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, int r, int g, int b, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgba32_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgba32_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgba64_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_rgba64_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_ycbcr_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, const struct sail_conversion_options *options);

SAIL_HIDDEN void fill_ycbcr_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, const struct sail_conversion_options *options);

#endif
