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
 * A mapping to describe which pixel formats are accepted as an input for writing operations,
 * and which pixel formats are allowed to be written from them.
 *
 * It's not just a flat list as not every input pixel format maps to every output pixel format.
 * For example, the JPEG plugin cannot accept YCBCR pixels and output CMYK pixels from them.
 *
 * That's why we need a more complex structure.
 */
struct sail_pixel_formats_mapping_node {

    /*
     * A supported input pixel format that can be accepted as input. See SailPixelFormat.
     *
     * For example: BPP24-RGB.
     */
    enum SailPixelFormat input_pixel_format;

    /*
     * A list of supported pixel formats that can be output from the input pixel format.
     *
     * If the array contains the SOURCE pixel format, then the codec is able to output pixel data as is.
     * It is a caller's responsibility to convert it to a suitable pixel format in this case. Refer to
     * sail_image.pixel_format to detect the actual pixel format of the pixel data.
     *
     * Outputting SOURCE pixels is always supported. Some plugins may provide even more
     * pixel formats to output.
     *
     * For example: SOURCE, BPP32-RGBA.
     */
    enum SailPixelFormat *output_pixel_formats;

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
