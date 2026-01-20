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

#include <sail-common/common.h>
#include <sail-common/config.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_image;

/*
 * Scaling algorithms available for image scaling.
 * Only algorithms supported by both swscale (when available) and manual scaling are included.
 */
enum SailScaling
{
    /* Nearest Neighbor (Point): Very fast, but blocky, no smoothing. */
    SAIL_SCALING_NEAREST_NEIGHBOR = 0,
    /* Standard Bilinear: Good balance of speed and quality. */
    SAIL_SCALING_BILINEAR,
    /* Bicubic: High quality, tunable parameters. */
    SAIL_SCALING_BICUBIC,
    /* Lanczos: Excellent quality and sharpness, moderate performance. */
    SAIL_SCALING_LANCZOS
};

/*
 * Scales the image to the specified dimensions using the specified algorithm
 * and saves the result in the output image.
 *
 * The scaling procedure converts the image to RGBA format internally for processing,
 * then converts back to the original pixel format.
 * All pixel formats with byte-aligned pixels (bits_per_pixel % 8 == 0) are supported.
 *
 * Uses libswscale for scaling with SIMD optimizations when available, otherwise falls back to manual scaling.
 *
 * The resulting image gets updated width, height, and bytes per line. Other properties are copied from the original
 * image.
 */
SAIL_EXPORT sail_status_t sail_scale_image(const struct sail_image* image,
                                           unsigned new_width,
                                           unsigned new_height,
                                           enum SailScaling algorithm,
                                           struct sail_image** image_output);

/* extern "C" */
#ifdef __cplusplus
}
#endif
