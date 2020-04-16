/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#ifndef SAIL_FILE_H
#define SAIL_FILE_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A structure representing a file object.
 */
struct sail_file {

    /* File descriptor. */
    FILE *fptr;

    /*
     * Plugin-specific data. A plugin could set pimpl to its plugin-specific data storage and access it
     * in read or write functions. Will be destroyed automatically in sail_destroy_file().
     */
    void *pimpl;
};

typedef struct sail_file sail_file_t;

/*
 * Opens the specified image file using the specified mode (as in fopen). The assigned file MUST be destroyed later
 * with sail_destroy_file().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_file(const char *path, const char *mode, struct sail_file **file);

/*
 * Opens the specified image file for reading. The assigned file MUST be destroyed later
 * with sail_destroy_file().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_file_for_reading(const char *path, struct sail_file **file);

/*
 * Opens the specified image file for writiing. The assigned file MUST be destroyed later
 * with sail_destroy_file().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_file_for_writing(const char *path, struct sail_file **file);

/*
 * Closes the specified file and destroys all its internal memory buffers. The file MUST NOT be used anymore
 * after calling this function. Does nothing if the file is NULL.
 */
SAIL_EXPORT void sail_destroy_file(struct sail_file *file);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
