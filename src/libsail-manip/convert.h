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

#ifndef SAIL_CONVERT_H
#define SAIL_CONVERT_H

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

struct sail_conversion_options;
struct sail_image;

/*
 * Converts the input image to the pixel format and saves the result in the output image.
 * The output image MUST be destroyed later with sail_destroy_image().
 *
 * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
 * when converting RGBA pixels to RGB. If you need to control this behavior,
 * use sail_convert_image_with_options().
 *
 * Allowed input pixel formats:
 *   - Anything except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_RGB
 *   - SAIL_PIXEL_FORMAT_BPP24_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP48_RGB
 *   - SAIL_PIXEL_FORMAT_BPP48_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP32_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP32_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP32_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP32_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_YCBCR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_convert_image(const struct sail_image *image_input,
                                             enum SailPixelFormat output_pixel_format,
                                             struct sail_image **image_output);

/*
 * Converts the input image to the pixel format and saves the result in the output image.
 * The output image MUST be destroyed later with sail_destroy_image().
 *
 * Options (which may be NULL) control the conversion behavior.
 *
 * Allowed input pixel formats:
 *   - Anything except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_RGB
 *   - SAIL_PIXEL_FORMAT_BPP24_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP48_RGB
 *   - SAIL_PIXEL_FORMAT_BPP48_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP32_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP32_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP32_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP32_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_YCBCR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_convert_image_with_options(const struct sail_image *image_input,
                                                          enum SailPixelFormat output_pixel_format,
                                                          const struct sail_conversion_options *options,
                                                          struct sail_image **image_output);

/*
 * Updates the image to the pixel format. If the function fails, the image pixels
 * may be left partially converted.
 *
 * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
 * when converting RGBA pixels to RGB. If you need to control this behavior,
 * use sail_update_image_with_options().
 *
 * Doesn't reallocate pixels. For example, when updating 100x100 BPP32-RGBA image
 * to BPP24-RGB, the resulting pixel data will have 10'000 unused bytes at the end.
 *
 * Allowed input pixel formats:
 *   - Anything that produces equal or smaller image except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_RGB
 *   - SAIL_PIXEL_FORMAT_BPP24_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP48_RGB
 *   - SAIL_PIXEL_FORMAT_BPP48_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP32_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP32_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP32_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP32_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_YCBCR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_update_image(struct sail_image *image, enum SailPixelFormat output_pixel_format);

/*
 * Updates the image to the pixel format. If the function fails, the image pixels
 * may be left partially converted.
 *
 * Options (which may be NULL) control the conversion behavior.
 *
 * Doesn't reallocate pixels. For example, when updating 100x100 BPP32-RGBA image
 * to BPP24-RGB, the resulting pixel data will have 10'000 unused bytes at the end.
 *
 * Allowed input pixel formats:
 *   - Anything that produces equal or smaller image except YCCK, LUV, and LAB
 *
 * Allowed output pixel formats:
 *   - SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_RGB
 *   - SAIL_PIXEL_FORMAT_BPP24_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP48_RGB
 *   - SAIL_PIXEL_FORMAT_BPP48_BGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP32_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP32_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP32_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP32_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP32_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP32_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBX
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRX
 *   - SAIL_PIXEL_FORMAT_BPP64_XRGB
 *   - SAIL_PIXEL_FORMAT_BPP64_XBGR
 *   - SAIL_PIXEL_FORMAT_BPP64_RGBA
 *   - SAIL_PIXEL_FORMAT_BPP64_BGRA
 *   - SAIL_PIXEL_FORMAT_BPP64_ARGB
 *   - SAIL_PIXEL_FORMAT_BPP64_ABGR
 *
 *   - SAIL_PIXEL_FORMAT_BPP24_YCBCR
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_update_image_with_options(struct sail_image *image,
                                                         enum SailPixelFormat output_pixel_format,
                                                         const struct sail_conversion_options *options);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
