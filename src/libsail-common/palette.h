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

#ifndef SAIL_PALETTE_H
#define SAIL_PALETTE_H

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

/*
 * sail_palette represents an image palette used in indexed images.
 */
struct sail_palette {

    /*
     * Palette pixel format.
     *
     * READ:  Set by SAIL to a valid palette pixel format if the image is indexed (palette is not NULL).
     *        SAIL guarantees the palette pixel format is byte-aligned.
     * WRITE: Must be set by a caller to a valid palette pixel format if the image is indexed.
     */
    enum SailPixelFormat pixel_format;

    /*
     * Palette data.
     *
     * READ:  Set by SAIL to a valid pixel array if the image is indexed.
     * WRITE: Must be allocated and set by a caller to a valid pixel array if the image is indexed.
     */
    void *data;

    /*
     * Number of colors in the palette.
     *
     * READ:  Set by SAIL to a valid number of colors if the image is indexed or to 0.
     * WRITE: Must be set by a caller to a valid number of colors if the image is indexed.
     */
    unsigned color_count;
};

typedef struct sail_palette sail_palette_t;

/*
 * Allocates a new palette. The assigned palette MUST be destroyed later with sail_destroy_palette().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_palette(struct sail_palette **palette);

/*
 * Destroys the specified palette and all its internal allocated memory buffers.
 * Does nothing if the palette is NULL.
 */
SAIL_EXPORT void sail_destroy_palette(struct sail_palette *palette);

/*
 * Makes a deep copy of the specified palette. The assigned palette MUST be destroyed later
 * with sail_destroy_palette().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_palette(const struct sail_palette *source_palette, struct sail_palette **target_palette);

/*
 * Allocates a new palette to be filled later with data. The allocated palette MUST be destroyed later
 * with sail_destroy_palette().
 *
 * Use this function to allocate a palette and fill its data later with some algorithm (memcpy or for-loop, for example).
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_palette_for_data(enum SailPixelFormat pixel_format, unsigned color_count, struct sail_palette **palette);

/*
 * Allocates a new palette and deep copies the specified data. The allocated palette MUST be destroyed later
 * with sail_destroy_palette().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_palette_from_data(enum SailPixelFormat pixel_format, const void *data, unsigned color_count, struct sail_palette **palette);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
