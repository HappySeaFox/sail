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

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_image;

/*
 * Rotates the image by 90, 180, or 270 degrees clockwise and saves the result in the output image.
 *
 * For 90° and 270° rotations, the output image dimensions are swapped (width <-> height).
 * For 180° rotation, the dimensions remain the same.
 *
 * The rotation is optimized using OpenMP for parallel processing when available.
 * All pixel formats with byte-aligned pixels (bits_per_pixel % 8 == 0) are supported.
 *
 * Supported angles:
 *   - SAIL_ORIENTATION_ROTATED_90   - Rotate 90° clockwise
 *   - SAIL_ORIENTATION_ROTATED_180  - Rotate 180°
 *   - SAIL_ORIENTATION_ROTATED_270  - Rotate 270° clockwise (same as 90° counter-clockwise)
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_rotate_image(const struct sail_image* image,
                                            enum SailOrientation angle,
                                            struct sail_image** image_output);

/*
 * Rotates the image by 180 degrees in-place (modifies the original image).
 *
 * This is an optimized in-place operation that doesn't require additional memory
 * for a new image.
 *
 * All pixel formats with byte-aligned pixels (bits_per_pixel % 8 == 0) are supported.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_rotate_image_180_inplace(struct sail_image* image);

/* extern "C" */
#ifdef __cplusplus
}
#endif
