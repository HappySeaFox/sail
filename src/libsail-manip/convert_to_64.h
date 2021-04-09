/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#ifndef SAIL_CONVERT_TO_64_H
#define SAIL_CONVERT_TO_64_H

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_image;

/*
 * Converts the specified input image to the BPP64-RGBA-ish format and saves the result in the output image.
 * The output image MUST be destroyed later with sail_destroy_image().
 *
 * Allowed input pixel formats:
 *   - Anything except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_convert_image_to_rgba64_kind(const struct sail_image *image_input,
                                                            enum SailPixelFormat output_pixel_format,
                                                            struct sail_image **image_output);

/*
 * Converts the specified input image to the BPP64-RGBA-ish format. If the function fails, the image pixels may be left partially converted.
 *
 * Allowed input pixel formats:
 *   - Anything with 64 bits per pixel except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_convert_image_to_rgba64_kind_in_place(struct sail_image *image, enum SailPixelFormat output_pixel_format);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
