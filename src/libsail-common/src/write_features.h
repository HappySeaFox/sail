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

#ifndef SAIL_WRITE_FEATURES_H
#define SAIL_WRITE_FEATURES_H

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

struct sail_pixel_formats_mapping_node;

/*
 * Write features. Use this structure to determine what a plugin can actually write.
 */
struct sail_write_features {

    /*
     * A mapping of supported pixel formats that can be written by this plugin.
     *
     * Outputting SOURCE pixels is always supported. Some plugins may provide even more
     * pixel formats to output.
     */
    struct sail_pixel_formats_mapping_node *pixel_formats_mapping_node;

    /* Supported plugin features of writing operations. See SailPluginFeatures. */
    int features;

    /*
     * Required image properties. For example, an input image must be flipped by a caller before writing
     * it with SAIL (or supply scan lines in a reverse order). See SailImageProperties.
     */
    int properties;

    /* Number of passes to write an interlaced image or 0. */
    int interlaced_passes;

    /*
     * A list of supported pixels compression types by this plugin. NULL if no compression types are available.
     * In most cases plugins support compression levels or compression types, but not both.
     *
     * For example:
     *
     *     1. The JPEG plugin supports only compression levels (compression_min, compression_max, compression_default).
     *     2. The TIFF plugin supports only compression types (RLE or no compression at all).
     */
    int *compression_types;

    /* The length of compression_types. */
    int compression_types_length;

    /* Preferred compression type to use by default. */
    int preferred_compression_type;

    /*
     * Minimum compression value. For lossy codecs, more compression means less quality and vice versa.
     * For lossless codecs, more compression means nothing but a smaller file size. This field is
     * plugin-specific. If compression_min == compression_max == 0, no compression tuning is available.
     * For example: 0.
     */
    int compression_min;

    /*
     * Maximum compression value. This field is plugin-specific. If compression_min == compression_max == 0,
     * no compression tuning is available. For example: 100.
     */
    int compression_max;

    /* Default compression value. For example: 15. */
    int compression_default;
};

typedef struct sail_write_features sail_write_features_t;

/*
 * Allocates write features. The assigned write features MUST be destroyed later
 * with sail_destroy_write_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_features(struct sail_write_features **write_features);

/*
 * Destroys the specified write features object and all its internal allocated memory buffers.
 * The write features MUST NOT be used after calling this function. It does nothing if the write features is NULL.
 */
SAIL_EXPORT void sail_destroy_write_features(struct sail_write_features *write_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
