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

/*
 * sail_read_options represents options to modify reading operations.
 */
struct sail_read_options {

    /* Or-ed I/O manipulation options for reading operations. See SailIoOption. */
    int io_options;
};

typedef struct sail_read_options sail_read_options_t;

/*
 * Allocates read options. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_read_options(struct sail_read_options **read_options);

/*
 * Destroys the specified read options object and all its internal allocated memory buffers. The read options
 * MUST NOT be used anymore after calling this function. Does nothing if the read options is NULL.
 */
SAIL_EXPORT void sail_destroy_read_options(struct sail_read_options *read_options);

/*
 * Builds default read options from read features.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options *read_options);

/*
 * Allocates and builds default read options from read features. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options **read_options);

/*
 * Makes a deep copy of the specified read options object. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_read_options(const struct sail_read_options *source, struct sail_read_options **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
