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

#ifndef SAIL_PIXEL_FORMATS_MAPPING_NODE_H
#define SAIL_PIXEL_FORMATS_MAPPING_NODE_H

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
 * A simple key-pair structure representing meta information like a JPEG comment.
 */
struct sail_pixel_formats_mapping_node {

    /*
     * A supported input pixel format that can be read by this plugin. See SailPixelFormat.
     *
     * For example: BPP24-RGB.
     */
    int input_pixel_format;

    /*
     * A list of supported pixel formats that can be output from the input pixel format by this plugin.
     *
     * If the array contains SAIL_PIXEL_FORMAT_SOURCE, then the codec is able to output raw pixel data.
     * It is a caller's responsibility to convert it to a suitable pixel format. Refer to
     * sail_image.pixel_format to detect the actual pixel format of the raw pixel data in that case.
     *
     * For example: SOURCE, BPP32-RGBA.
     */
    int *output_pixel_formats;

    /* The length of output_pixel_formats. */
    int output_pixel_formats_length;

    struct sail_pixel_formats_mapping_node *next;
};

/*
 * Allocates a new pixel formats mapping node. The assigned node MUST be destroyed later with sail_destroy_pixel_formats_mapping_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node **node);

/*
 * Destroys the specified pixel formats mapping node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node *node);

/*
 * Destroys the specified pixel formats mapping node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_pixel_formats_mapping_node_chain(struct sail_pixel_formats_mapping_node *node);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
