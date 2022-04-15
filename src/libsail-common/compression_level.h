/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#ifndef SAIL_COMPRESSION_LEVEL_H
#define SAIL_COMPRESSION_LEVEL_H

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
 * Compression level.
 */
struct sail_compression_level {

    /*
     * Minimum compression value. For lossy codecs, more compression means less quality
     * and vice versa. For lossless codecs, more compression means nothing but a smaller
     * file size.
     */
    double min_level;

    /*
     * Maximum compression value. For lossy codecs, more compression means less quality
     * and vice versa. For lossless codecs, more compression means nothing but a smaller
     * file size.
     */
    double max_level;

    /*
     * Default compression value within the min/max range.
     */
    double default_level;

    /*
     * Step to increase or decrease compression levels in the range.
     * Can be used in UI to build a compression level selection component.
     */
    double step;
};

typedef struct sail_compression_level sail_compression_level_t;

/*
 * Allocates a new compression level.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_compression_level(struct sail_compression_level **compression_level);

/*
 * Destroys the specified compression level. Does nothing if the compression level is NULL.
 */
SAIL_EXPORT void sail_destroy_compression_level(struct sail_compression_level *compression_level);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
