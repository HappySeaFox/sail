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

#ifndef SAIL_IO_COMMON_H
#define SAIL_IO_COMMON_H

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

typedef sail_error_t (*sail_io_read_t)(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count);
typedef sail_error_t (*sail_io_seek_t)(void *stream, long offset, int whence);
typedef sail_error_t (*sail_io_tell_t)(void *stream, unsigned long *offset);
typedef sail_error_t (*sail_io_write_t)(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count);
typedef sail_error_t (*sail_io_flush_t)(void *stream);
typedef sail_error_t (*sail_io_close_t)(void *stream);
typedef sail_error_t (*sail_io_eof_t)(void *stream, bool *result);

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
    sail_io_read_t read;

    /*
     * Sets the I/O position in the underlying I/O object.
     *
     * whence possible values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <stdio.h>.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_seek_t seek;

    /*
     * Assigns the current I/O position in the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_tell_t tell;

    /*
     * Writes the specified buffer to the underlying I/O object. Assigns the number of objects
     * actually written to written_objects_count.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_write_t write;

    /*
     * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
     * is opened for reading.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_flush_t flush;

    /*
     * Closes the underlying I/O object.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_close_t close;

    /*
     * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_io_eof_t eof;
};

typedef struct sail_io sail_io_t;

/*
 * Allocates a new I/O object. The assigned I/O object MUST be destroyed later with sail_destroy_io().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_io(struct sail_io **io);

/*
 * Closes and destroys the specified I/O object and all its internal allocated memory buffers.
 * The I/O object MUST NOT be used anymore after calling this function.
 *
 * Note for technical divers: sail_destroy_io() DOES NOT destroy the underlying I/O stream.
 *                            It must be destroyed (if necessary) in the sail_io.close callback.
 *
 * Does nothing if the I/O object is NULL.
 */
SAIL_EXPORT void sail_destroy_io(struct sail_io *io);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
