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

struct sail_meta_entry_node;

/*
 * A structure representing an ICC profile.
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
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_palette(struct sail_palette **palette);

/*
 * Destroys the specified palette and all its internal allocated memory buffers.
 * Does nothing if the palette is NULL.
 */
SAIL_EXPORT void sail_destroy_palette(struct sail_palette *palette);

/*
 * Makes a deep copy of the specified palette. The assigned palette MUST be destroyed later
 * with sail_destroy_palette().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_palette(const struct sail_palette *source_palette, struct sail_palette **target_palette);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
