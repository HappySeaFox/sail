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
 * For example, the JPEG codec cannot accept YCBCR pixels and output CMYK pixels from them.
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
     * If the array contains the SOURCE pixel format, then the codec is able to output pixels as is.
     * It is a caller's responsibility to convert it to a suitable pixel format in this case. Refer to
     * sail_image.pixel_format to detect the actual pixel format of the pixels.
     *
     * Outputting SOURCE pixels is always supported. Some codecs may provide even more
     * pixel formats to output.
     *
     * For example: SOURCE, BPP32-RGBA.
     */
    enum SailPixelFormat *output_pixel_formats;

    /* The length of output_pixel_formats. */
    unsigned output_pixel_formats_length;

    struct sail_pixel_formats_mapping_node *next;
};

/*
 * Allocates a new pixel formats mapping node. The assigned node MUST be destroyed later with sail_destroy_pixel_formats_mapping_node().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_alloc_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node **node);

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
