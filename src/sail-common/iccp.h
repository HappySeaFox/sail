/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#ifndef SAIL_ICCP_H
#define SAIL_ICCP_H

#include <stddef.h> /* size_t */

#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ICCP represents an ICC profile.
 */
struct sail_iccp {

    /* ICC profile binary data. */
    void *data;

    /* The size of the data. */
    size_t size;
};

typedef struct sail_iccp sail_iccp_t;

/*
 * Allocates a new ICC profile.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_iccp(struct sail_iccp **iccp);

/*
 * Allocates a new ICC profile and deep copies the specified ICC profile data into it.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_iccp_from_data(const void *data, size_t data_size, struct sail_iccp **iccp);

/*
 * Allocates a new ICC profile and copies the external pointer to another ICC data into it.
 * Do not free the external pointer. It will be freed in sail_destroy_iccp().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_iccp_from_shallow_data(void *data, size_t data_size, struct sail_iccp **iccp);

/*
 * Allocates a new ICC profile for the specified ICC profile data size.
 *
 * Use this function to allocate an ICC profile and fill its data later with
 * some algorithm (memcpy or for-loop, for example).
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_iccp_for_data(size_t data_size, struct sail_iccp **iccp);

/*
 * Destroys the specified ICC profile and all its internal allocated memory buffers.
 * Does nothing if the profile is NULL.
 */
SAIL_EXPORT void sail_destroy_iccp(struct sail_iccp *iccp);

/*
 * Makes a deep copy of the specified ICC profile.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_iccp(const struct sail_iccp *source_iccp, struct sail_iccp **target_iccp);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
