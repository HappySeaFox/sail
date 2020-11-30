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

#ifndef SAIL_IO_COMMON_H
#define SAIL_IO_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

typedef sail_status_t (*sail_io_read_t)(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count);
typedef sail_status_t (*sail_io_seek_t)(void *stream, long offset, int whence);
typedef sail_status_t (*sail_io_tell_t)(void *stream, size_t *offset);
typedef sail_status_t (*sail_io_write_t)(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count);
typedef sail_status_t (*sail_io_flush_t)(void *stream);
typedef sail_status_t (*sail_io_close_t)(void *stream);
typedef sail_status_t (*sail_io_eof_t)(void *stream, bool *result);

/*
 * Well-known I/O ids used in libsail for file and memory I/O classes.
 *
 * You MUST use your own unique id for custom I/O classes. For example, you can use sail_hash()
 * to generate a unique id and store it in the source code.
 *
 * SAIL_FILE_IO_ID   = sail_hash("sail-file-io-id")
 * SAIL_MEMORY_IO_ID = sail_hash("sail-memory-io-id")
 */
static const uint64_t SAIL_FILE_IO_ID   = UINT64_C(5820790535323209114);
static const uint64_t SAIL_MEMORY_IO_ID = UINT64_C(11955407548648566675);

/*
 * A structure representing an input/output abstraction. Use sail_alloc_io_read_file() and brothers to
 * allocate I/O objects.
 */
struct sail_io {

    /*
     * Unique I/O class id. The same I/O classes (file, memory etc.) share the same ids. This way
     * a client can known the exact type of the I/O object. For example, a client can distinguish between
     * file and memory I/O objects.
     *
     * You MUST use your own unique id for custom I/O classes. For example, you can use sail_hash()
     * to generate a unique id and assign it to this field.
     */
    uint64_t id;

    /*
     * I/O specific data object. For example, a pointer to FILE.
     */
    void *stream;

    /*
     * Reads from the underlying I/O object into the specified buffer. Assigns the number of objects
     * actually read to read_objects_count.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_read_t read;

    /*
     * Sets the I/O position in the underlying I/O object.
     *
     * whence possible values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <stdio.h>.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_seek_t seek;

    /*
     * Assigns the current I/O position in the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_tell_t tell;

    /*
     * Writes the specified buffer to the underlying I/O object. Assigns the number of objects
     * actually written to written_objects_count.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_write_t write;

    /*
     * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
     * is opened for reading.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_flush_t flush;

    /*
     * Closes the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_close_t close;

    /*
     * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
     *
     * Returns SAIL_OK on success.
     */
    sail_io_eof_t eof;
};

typedef struct sail_io sail_io_t;

/*
 * Allocates a new I/O object. The assigned I/O object MUST be destroyed later with sail_destroy_io().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_io(struct sail_io **io);

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
