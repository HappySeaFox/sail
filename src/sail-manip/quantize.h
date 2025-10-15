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
 * Quantizes the input RGB/RGBA image to indexed format with specified output pixel format.
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
 * The output image will be in the specified indexed format:
 *   - SAIL_PIXEL_FORMAT_BPP1_INDEXED (2 colors max)
 *   - SAIL_PIXEL_FORMAT_BPP2_INDEXED (4 colors max)
 *   - SAIL_PIXEL_FORMAT_BPP4_INDEXED (16 colors max)
 *   - SAIL_PIXEL_FORMAT_BPP8_INDEXED (256 colors max)
 *
 * The output image will have a palette attached (BPP24_RGB format).
 * The palette may have fewer colors than the maximum for the format, but the
 * pixel data will always be in the requested format.
 *
 * output_pixel_format: The desired indexed pixel format for the output image.
 *                      Must be one of the indexed formats listed above.
 *
 * dither: If true, apply Floyd-Steinberg dithering to reduce color banding.
 *         Currently only supported for BPP8_INDEXED output.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_quantize_image(const struct sail_image* source_image,
                                              enum SailPixelFormat output_pixel_format,
                                              bool dither,
                                              struct sail_image** target_image);

#ifdef __cplusplus
}
#endif
