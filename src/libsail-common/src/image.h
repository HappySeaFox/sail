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

#ifndef SAIL_IMAGE_H
#define SAIL_IMAGE_H

#include <stdbool.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_palette;
struct sail_meta_entry_node;
struct sail_iccp;
struct sail_source_image;

/*
 * A structure representing an image. Fields set by SAIL when reading images are marked with READ.
 * Fields that must be set by a caller when writing images are marked with WRITE.
 */
struct sail_image {

    /*
     * Image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    unsigned width;

    /*
     * Image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    unsigned height;

    /*
     * Bytes per line. Some image formats (like BMP) pad rows of pixels to some boundary.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *        it to sail_bytes_per_line() if scan lines are not padded to a certain boundary.
     */
    unsigned bytes_per_line;

    /*
     * Image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by a certain plugin can be obtained from sail_read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *        to the plugin by a caller later. The list of supported input pixel formats by a certain plugin
     *        can be obtained from sail_write_features.output_pixel_formats.
     */
    enum SailPixelFormat pixel_format;

    /*
     * Number of passes needed to read or write an entire image frame if it's interlaced. 1 by default.
     *
     * This field is used internally by SAIL. DO NOT alter its value.
     *
     * READ:  N/A.
     * WRITE: N/A.
     */
    int interlaced_passes;

    /*
     * Is the image a frame in an animation.
     *
     * READ:  Set by SAIL to true if the image is a frame in an animation.
     * WRITE: Must be set by a caller to true if the image is a frame in an animation.
     *        Codecs may need to know if they write a static or an animated image.
     */
    bool animated;

    /*
     * Delay in milliseconds to display the image on the screen if the image is a frame
     * in an animation or 0 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds.
     */
    int delay;

    /*
     * Palette if the image has a palette and the requested pixel format assumes having a palette.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid palette if the image is indexed.
     * WRITE: Must be allocated and set by a caller to a valid palette if the image is indexed.
     */
    struct sail_palette *palette;

    /*
     * Image meta information. See sail_meta_entry_node. Plugins guarantee that keys and values are non-NULL.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid linked list with simple meta information (like JPEG comments) or to NULL.
     * WRITE: Must be allocated and set by a caller to a valid linked list with simple meta information
     *        (like JPEG comments) if necessary.
     */
    struct sail_meta_entry_node *meta_entry_node;

    /*
     * Embedded ICC profile.
     *
     * Note for animated/multi-paged images: only the first image in an animated/multi-paged
     * sequence might have an ICC profile.
     *
     * READ:  Set by SAIL to a valid ICC profile if any.
     * WRITE: Must be allocated and set by a caller to a valid ICC profile if necessary.
     */
    struct sail_iccp *iccp;

    /*
     * Or-ed decoded image properties. See SailImageProperty.
     *
     * READ:  Set by SAIL to valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g., flip back etc.).
     * WRITE: Ignored.
     */
    int properties;

    /*
     * Source image properties.
     *
     * READ:  Set by SAIL to valid source image properties of the original image.
     * WRITE: Ignored.
     */
    struct sail_source_image *source_image;
};

typedef struct sail_image sail_image_t;

/*
 * Allocates a new image. The assigned image MUST be destroyed later with sail_destroy_image().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_image(struct sail_image **image);

/*
 * Destroys the specified image and all its internal allocated memory buffers. The image MUST NOT be used anymore
 * after calling this function. Does nothing if the image is NULL.
 */
SAIL_EXPORT void sail_destroy_image(struct sail_image *image);

/*
 * Makes a deep copy of the specified image. The assigned image MUST be destroyed later with sail_destroy_image().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_image(const struct sail_image *source, struct sail_image **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
