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

#ifndef SAIL_READ_FEATURES_H
#define SAIL_READ_FEATURES_H

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
 * Read features. Use this structure to determine what a plugin can actually read.
 */
struct sail_read_features {

    /*
     * A list of supported pixel formats that can be output by this plugin.
     * The SOURCE, BPP24-RGB, and BPP32-RGBA output pixel formats are always supported. Some plugins may
     * provide even more output pixel formats.
     */
    int *output_pixel_formats;

    /* The length of output_pixel_formats. */
    int output_pixel_formats_length;

    /*
     * Output pixel format to use by default when no specific output pixel format was requested by a user.
     * It's always BPP24-RGB for image formats without transparency support and BPP32-RGBA otherwise.
     */
    int preferred_output_pixel_format;

    /* Supported features of reading operations. See SailPluginFeatures. */
    int features;
};

typedef struct sail_read_features sail_read_features_t;

/*
 * Allocates read features. The assigned read features MUST be destroyed later
 * with sail_destroy_read_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_features(struct sail_read_features **read_features);

/*
 * Destroys the specified read features object and all its internal allocated memory buffers. The read features
 * MUST NOT be used anymore after calling this function. Does nothing if the read features is NULL.
 */
SAIL_EXPORT void sail_destroy_read_features(struct sail_read_features *read_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
