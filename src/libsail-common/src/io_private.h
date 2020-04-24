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

#ifndef SAIL_IO_PRIVATE_H
#define SAIL_IO_PRIVATE_H

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
     * Plugin-specific data. A plugin could set pimpl to its plugin-specific data storage and access it
     * in its read or write functions. Will be destroyed automatically in sail_destroy_io().
     */
    void *pimpl;

    /*
     * Reads from the underlying I/O object into the specified buffer.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*read)(void *stream, void *buf, size_t object_size, size_t objects_count);

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
     * Writes the specified buffer to the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*write)(void *stream, const void *buf, size_t object_size, size_t objects_count);

    /*
     * Closes the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t (*close)(void *stream);
};

typedef struct sail_io sail_io_t;

#endif
