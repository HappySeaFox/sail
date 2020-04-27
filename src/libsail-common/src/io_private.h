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

#ifndef SAIL_IO_PRIVATE_H
#define SAIL_IO_PRIVATE_H

#include <stdbool.h>
#include <stdio.h>

#include "error.h"

/*
 * A structure representing an input/output abstraction. Use sail_alloc_io_read_file() and brothers to
 * allocate I/O objects.
 */
struct sail_io {

    /*
     * I/O specific data object. For example, a pointer to FILE.
     */
    void *stream;

    /*
     * Reads from the underlying I/O object into the specified buffer. Assigns the number of objects
     * actually read to read_objects_count.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*read)(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count);

    /*
     * Sets the I/O position in the underlying I/O object.
     *
     * whence possible values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <stdio.h>.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*seek)(void *stream, long offset, int whence);

    /*
     * Assigns the current I/O position in the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*tell)(void *stream, long *offset);

    /*
     * Writes the specified buffer to the underlying I/O object. Assigns the number of objects
     * actually written to written_objects_count.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*write)(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count);

    /*
     * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
     * is opened for reading.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*flush)(void *stream);

    /*
     * Closes the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*close)(void *stream);

    /*
     * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*eof)(void *stream, bool *result);
};

typedef struct sail_io sail_io_t;

/*
 * Allocates a new I/O object. The assigned I/O object MUST be destroyed later with sail_destroy_io().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_HIDDEN sail_error_t sail_alloc_io(struct sail_io **io);

#endif
