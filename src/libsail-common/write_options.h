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

    /* Or-ed I/O manipulation options. See SailIoOption. */
    int io_options;

    /*
     * Compression type. For example: SAIL_COMPRESSION_RLE. See SailCompression.
     * Use sail_write_features to determine what compression types or values are supported by a particular codec.
     *
     * If a codec supports more than two compression types, compression levels are ignored in this case.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. compression_level_min, compression_level_max,
     *        compression_level_default can be used to select a compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.). Compression levels
     *        are ignored.
     */
    enum SailCompression compression;

    /*
     * Requested compression level. Must be in the range specified by compression_level_min and compression_level_max
     * in sail_write_features. If compression_level < compression_level_min, compression_level_default will be used.
     */
    double compression_level;
};

typedef struct sail_write_options sail_write_options_t;

/*
 * Allocates write options. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_write_options(struct sail_write_options **write_options);

/*
 * Destroys the specified write options object and all its internal allocated memory buffers.
 * The write options MUST NOT be used anymore after calling this function. It does nothing
 * if the write options is NULL.
 */
SAIL_EXPORT void sail_destroy_write_options(struct sail_write_options *write_options);

/*
 * Builds default write options from write features.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options);

/*
 * Allocates and builds default write options from write features.
 * The assigned write options MUST be destroyed later with sail_destroy_write_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options);

/*
 * Makes a deep copy of the specified write options object. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_write_options(const struct sail_write_options *write_options_source, struct sail_write_options **write_options_target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
