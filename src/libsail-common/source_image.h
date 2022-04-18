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

#ifndef SAIL_SOURCE_IMAGE_H
#define SAIL_SOURCE_IMAGE_H

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

struct sail_hash_map;

/*
 * sail_source_image represents source image properties. The structure is used in loading
 * operations only to preserve the source image properties which are usually lost during decoding.
 * For example, one might want to know the source image pixel format.
 * It's ignored in saving operations.
 */
struct sail_source_image {

    /*
     * Source image pixel format. See SailPixelFormat.
     *
     * LOAD: Set by SAIL to a source image pixel format of the original image.
     * SAVE: Ignored.
     */
    enum SailPixelFormat pixel_format;

    /*
     * Source image chroma subsampling. See SailChromaSubsampling.
     *
     * LOAD: Set by SAIL to a source image chroma subsampling of the original image.
     * SAVE: Ignored.
     */
    enum SailChromaSubsampling chroma_subsampling;

    /*
     * Source image orientation.
     *
     * LOAD: Set by SAIL to the source image orientation.
     * SAVE: Ignored.
     */
    enum SailOrientation orientation;

    /*
     * Source image compression type. See SailCompression.
     *
     * LOAD: Set by SAIL to a valid source image compression type.
     * SAVE: Ignored.
     */
    enum SailCompression compression;

    /*
     * Source image interlacing.
     *
     * LOAD: Set by SAIL to true if the source image is interlaced.
     * SAVE: Ignored.
     */
    bool interlaced;

    /*
     * Image format-specific properties that cannot be expressed
     * in a common way. For example, a cursor hot spot.
     *
     * Can be NULL.
     *
     * Special properties' names start with the codec name to avoid confusing.
     * For example, "cur-hotspot-x".
     *
     * LOAD: Set by SAIL to valid source image special properties.
     * SAVE: Ignored.
     */
    struct sail_hash_map *special_properties;
};

typedef struct sail_source_image sail_source_image_t;

/*
 * Allocates a new source image.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_source_image(struct sail_source_image **source_image);

/*
 * Destroys the specified source image and all its internal allocated memory buffers. The source image MUST NOT be used
 * anymore after calling this function. Does nothing if the source image is NULL.
 */
SAIL_EXPORT void sail_destroy_source_image(struct sail_source_image *source_image);

/*
 * Makes a deep copy of the specified source image.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_source_image(const struct sail_source_image *source, struct sail_source_image **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
