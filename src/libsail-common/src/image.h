/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_IMAGE_H
#define SAIL_IMAGE_H

#include <stdbool.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_meta_entry_node;

/*
 * A structure representing an image. Fields set when reading images are marked with READ.
 * Fields that must be set by a caller when writing images are marked with WRITE.
 */
struct sail_image {

    /*
     * Image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    int width;

    /*
     * Image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    int height;

    /*
     * Bytes per line. Some image formats (like BMP) pad rows of pixels to some boundary.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *        it to sail_bytes_per_line() if scan lines do not need padding to a certain booundary.
     */
    int bytes_per_line;

    /*
     * Image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by this plugin could be obtained from sail_read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *        to the plugin by a caller later. The list of supported input pixel formats by this plugin
     *        could be obtained from sail_write_features.output_pixel_formats.
     */
    int pixel_format;

    /*
     * Number of passes needed to read or write an entire image frame. 1 by default.
     *
     * READ:  Set by SAIL to a positive number of passes needed to read an image. For example, interlaced PNGs
     *        have 8 passes.
     * WRITE: Ignored. Use sail_write_features.passes to determine the actual number of passes needed to write
     *        an interlaced image.
     */
    int passes;

    /*
     * Is the image a frame in an animation.
     *
     * READ:  Set by SAIL to true if the image is a frame in an animation.
     * WRITE: Ignored.
     */
    bool animated;

    /*
     * Delay in milliseconds if the image is a frame in an animation or 0 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds.
     */
    int delay;

    /*
     * Palette pixel format.
     *
     * READ:  Set by SAIL to a valid palette pixel format if the image is indexed (palette is not NULL).
     * WRITE: Must be set by a caller to a valid palette pixel format if the image is indexed
     *        (palette is not NULL).
     */
    int palette_pixel_format;

    /*
     * Palette if the image has a palette and the requested pixel format assumes having a palette.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid pixel array if the image is indexed.
     * WRITE: Must be allocated and set by a caller to a valid pixel array if the image is indexed.
     */
    void *palette;

    /*
     * Size of the palette data in bytes.
     *
     * READ:  Set by SAIL to a valid palette size in bytes if the image is indexed or to 0.
     * WRITE: Must be set by a caller to a valid palette size in bytes if the image is indexed.
     */
    int palette_size;

    /*
     * Image meta information. See sail_meta_entry_node. Plugins guarantee that keys and values are non-NULL.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid linked list with simple meta information (like JPEG comments) or to NULL.
     * WRITE: Must be allocated and set by a caller to a valid linked list with simple meta information
     *        like JPEG comments if the image have it.
     */
    struct sail_meta_entry_node *meta_entry_node;

    /*
     * Decoded image properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g. flip back etc.).
     * WRITE: Ignored.
     */
    int properties;

    /*
     * Image source pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid source image pixel format before converting it to a requested pixel format
     *        with sail_read_options.pixel_format.
     * WRITE: Ignored.
     */
    int source_pixel_format;

    /*
     * Image source properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid source image properties or to 0.
     * WRITE: Ignored.
     */
    int source_properties;
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

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
