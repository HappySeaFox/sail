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

#ifndef SAIL_WRITE_OPTIONS_H
#define SAIL_WRITE_OPTIONS_H

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

struct sail_write_features;

/* Options to modify writing operations. */
struct sail_write_options {

    /*
     * Request to modify the output pixel format. The list of possible output pixel formats
     * can be obtained from sail_write_features.pixel_formats_mapping_node.
     */
    int output_pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int io_options;

    /*
     * Compression type or 0. For example: SAIL_COMPRESSION_RLE. See SailCompressionTypes.
     * In most cases, plugins support compression levels or compression types, but not both.
     * Use sail_write_features to determine what compression types or values are supported by a particular plugin.
     *
     * For example:
     *
     *     1. The JPEG plugin supports only compression levels (compression_min, compression_max, compression_default).
     *     2. The TIFF plugin supports only compression types (RLE or no compression at all).
     */
    int compression_type;

    /*
     * Requested compression value. Must be in the range specified by compression_min and compression_max
     * in sail_write_features. If compression < compression_min, compression_default will be used.
     */
    int compression;
};

typedef struct sail_write_options sail_write_options_t;

/*
 * Allocates write options. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_options(struct sail_write_options **write_options);

/*
 * Destroys the specified write options object and all its internal allocated memory buffers.
 * The write options MUST NOT be used anymore after calling this function. It does nothing
 * if the write options is NULL.
 */
SAIL_EXPORT void sail_destroy_write_options(struct sail_write_options *write_options);

/*
 * Builds default write options from write features.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options);

/*
 * Allocates and builds default write options from write features.
 * The assigned write options MUST be destroyed later with sail_destroy_write_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options);

/*
 * Makes a deep copy of the specified write options object. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_write_options(const struct sail_write_options *write_options_source, struct sail_write_options **write_options_target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
