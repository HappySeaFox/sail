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

#ifndef SAIL_RESOLUTION_H
#define SAIL_RESOLUTION_H

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Image resolution unit and values.
 */
struct sail_resolution {

    enum SailResolutionUnit unit;

    double x;
    double y;
};

/*
 * Allocates a new resolution. The assigned resolution MUST be destroyed later with sail_destroy_resolution().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_resolution(struct sail_resolution **resolution);

/*
 * Allocates a new resolution and initializes its fields with the specified values.
 * The assigned resolution MUST be destroyed later with sail_destroy_resolution().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_resolution_from_data(enum SailResolutionUnit unit, double x, double y, struct sail_resolution **resolution);

/*
 * Destroys the specified resolution.
 */
SAIL_EXPORT void sail_destroy_resolution(struct sail_resolution *resolution);

/*
 * Makes a deep copy of the specified resolution. The assigned resolution MUST be destroyed
 * later with sail_destroy_resolution().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_resolution(struct sail_resolution *source, struct sail_resolution **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
