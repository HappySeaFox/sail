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

#ifndef SAIL_READ_OPTIONS_H
#define SAIL_READ_OPTIONS_H

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

struct sail_read_features;

/* Options to modify reading operations. */
struct sail_read_options {

    /*
     * Request to modify the output pixel format. Plugin (or an underlying codec) may reject
     * the requested pixel format with an error. If the codec supports source pixels
     * (sail_read_features.output_pixel_formats contains SAIL_PIXEL_FORMAT_SOURCE), one may
     * set this field to SAIL_PIXEL_FORMAT_SOURCE to copy pixel data as is.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       SAIL returns an error in this case.
     */
    int output_pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int io_options;
};

typedef struct sail_read_options sail_read_options_t;

/*
 * Allocates read options. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_options(struct sail_read_options **read_options);

/*
 * Destroys the specified read options object and all its internal allocated memory buffers. The read options
 * MUST NOT be used anymore after calling this function. Does nothing if the read options is NULL.
 */
SAIL_EXPORT void sail_destroy_read_options(struct sail_read_options *read_options);

/*
 * Builds default read options from read features.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options *read_options);

/*
 * Allocates and builds default read options from read features. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options **read_options);

/*
 * Makes a deep copy of the specified read options object. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_read_options(const struct sail_read_options *read_options_source, struct sail_read_options **read_options_target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
