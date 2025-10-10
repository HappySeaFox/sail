/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <sail-common/sail-common.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_image;

/*
 * Quantizes the input RGB/RGBA image to indexed format with specified number of colors.
 * Uses Xiaolin Wu's color quantization algorithm (1992).
 *
 * The input image must be in one of the following pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP24_RGB
 *   - SAIL_PIXEL_FORMAT_BPP24_BGR
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRX
 *
 * The output image will be in indexed format:
 *   - SAIL_PIXEL_FORMAT_BPP8_INDEXED for 3-256 colors
 *   - SAIL_PIXEL_FORMAT_BPP4_INDEXED for 2-16 colors (rounded up)
 *   - SAIL_PIXEL_FORMAT_BPP1_INDEXED for 2 colors
 *
 * The output image will have a palette attached (BPP24_RGB format).
 *
 * max_colors: Maximum number of colors in the output palette (2-256).
 *             The actual number may be less if the image has fewer unique colors.
 *
 * dither: If true, apply Floyd-Steinberg dithering to reduce color banding.
 *         Currently only supported for BPP8_INDEXED output.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_quantize_image(const struct sail_image* source_image,
                                              unsigned max_colors,
                                              bool dither,
                                              struct sail_image** target_image);

#ifdef __cplusplus
}
#endif
