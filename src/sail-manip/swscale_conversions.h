/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)
 *
 *  Copyright (c) 2026 Dmitry Baryshev
 *
 *  The MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include <stdbool.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_image;

#ifdef SAIL_MANIP_USE_SWSCALE

/*
 * Fast pixel format conversion using libswscale (FFmpeg).
 *
 * This function provides optimized conversion paths for formats supported by swscale,
 * using SIMD optimizations for better performance.
 *
 * Supported conversions:
 *   - RGB <=> BGR (RGB24 <=> BGR24, RGBA <=> BGRA, etc.)
 *   - RGBA variants (RGBA <=> ARGB <=> ABGR <=> BGRA)
 *   - RGBX <=> RGBA conversions
 *   - Grayscale <=> RGB conversions
 *   - YUV <=> RGB conversions
 *   - 8-bit <=> 16-bit conversions for RGB/Grayscale
 *
 * Returns true if swscale conversion is available and executed successfully.
 * Returns false if swscale doesn't support this conversion pair or is disabled.
 */
SAIL_HIDDEN bool sail_try_swscale_conversion(const struct sail_image* image_input,
                                             struct sail_image* image_output,
                                             enum SailPixelFormat output_pixel_format);

#else

/* Stub when swscale is not available */
static inline bool sail_try_swscale_conversion(const struct sail_image* image_input,
                                               struct sail_image* image_output,
                                               enum SailPixelFormat output_pixel_format)
{
    (void)image_input;
    (void)image_output;
    (void)output_pixel_format;
    return false;
}

#endif /* SAIL_MANIP_USE_SWSCALE */
