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
     *
     * The SOURCE output pixel format is always supported.
     */
    enum SailPixelFormat output_pixel_format;

    /* Or-ed IO manipulation options. See SailIoOption. */
    int io_options;

    /*
     * Compression type or 0. For example: SAIL_COMPRESSION_RLE. See SailCompressionType.
     * In most cases, plugins support compression levels or compression types, but not both.
     * Use sail_write_features to determine what compression types or values are supported by a particular plugin.
     *
     * For example:
     *
     *     1. The JPEG plugin supports only compression levels (compression_min, compression_max, compression_default).
     *     2. The TIFF plugin supports only compression types (RLE or no compression at all).
     */
    enum SailCompressionType compression_type;

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
