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
#include <stddef.h> /* size_t */
#include <stdint.h>
#include <stdio.h>  /* SEEK_CUR */

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
 * Reads from the underlying I/O object into the specified buffer. In contrast to sail_io_strict_read_t,
 * doesn't fail when the actual number of bytes read is smaller than requested.
 * Assigns the number of bytes actually read to the 'read_size' argument.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_tolerant_read_t)(void *stream, void *buf, size_t size_to_read, size_t *read_size);

/*
 * Reads from the underlying I/O object into the specified buffer. In contrast to sail_io_tolerant_read_t,
 * fails when the actual number of bytes read is smaller than requested.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_strict_read_t)(void *stream, void *buf, size_t size_to_read);

/*
 * Writes the specified buffer to the underlying I/O object. In contrast to sail_io_strict_write_t,
 * doesn't fail when the actual number of bytes written is smaller than requested.
 * Assigns the number of bytes actually written to the 'written_size' argument.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_tolerant_write_t)(void *stream, const void *buf, size_t size_to_write, size_t *written_size);

/*
 * Writes the specified buffer to the underlying I/O object. In contrast to sail_io_tolerant_write_t,
 * fails when the actual number of bytes written is smaller than requested.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_strict_write_t)(void *stream, const void *buf, size_t size_to_write);

/*
 * Sets the I/O position in the underlying I/O object.
 *
 * Possible 'whence' values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <stdio.h>.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_seek_t)(void *stream, long offset, int whence);

/*
 * Assigns the current I/O position in the underlying I/O object.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_tell_t)(void *stream, size_t *offset);

/*
 * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
 * is opened for reading.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_flush_t)(void *stream);

/*
 * Closes the underlying I/O object.
 *
 * Returns SAIL_OK on success.
 */
typedef sail_status_t (*sail_io_close_t)(void *stream);

/*
 * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
 *
 * Returns SAIL_OK on success.
 */
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

/* I/O features. */
enum SailIoFeature {

    /*
     * The I/O object is seekable. When this flag is off, the seek callback
     * must return SAIL_ERROR_NOT_IMPLEMENTED.
     */
    SAIL_IO_FEATURE_SEEKABLE = 1 << 0,
};

/*
 * sail_io represents an input/output abstraction. Use sail_alloc_io_read_file() and brothers to
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
     * Or-ed I/O features. See SailIoFeature.
     */
    int features;

    /*
     * I/O-specific data object. For example, a pointer to a FILE.
     */
    void *stream;

    /*
     * Tolerant read callback.
     */
    sail_io_tolerant_read_t tolerant_read;

    /*
     * Strict read callback.
     */
    sail_io_strict_read_t strict_read;

    /*
     * Tolerant write callback.
     */
    sail_io_tolerant_write_t tolerant_write;

    /*
     * Strict write callback.
     */
    sail_io_strict_write_t strict_write;

    /*
     * Seek callback.
     */
    sail_io_seek_t seek;

    /*
     * Tell callback.
     */
    sail_io_tell_t tell;

    /*
     * Flush callback.
     */
    sail_io_flush_t flush;

    /*
     * Close callback.
     */
    sail_io_close_t close;

    /*
     * EOF callback.
     */
    sail_io_eof_t eof;
};

typedef struct sail_io sail_io_t;

/*
 * Allocates a new I/O object.
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

/*
 * Returns SAIL_OK if the given I/O object has valid callbacks and a non-zero id.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_check_io_valid(const struct sail_io *io);

/*
 * Retrieves the I/O stream size. The stream must be seekable.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_io_size(struct sail_io *io, size_t *size);

/*
 * Reads the specified I/O stream until EOF into the memory buffer. Reads the stream
 * from the current position and then rewinds it back (i.e. the stream must be seekable).
 * The buffer must be large enough.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_io_contents_into_data(struct sail_io *io, void *data);

/*
 * Allocates a memory buffer and reads the specified I/O stream until EOF into it.
 * Reads the stream from the current position and then rewinds it back (i.e. the stream
 * must be seekable).
 *
 * The size of the memory buffer is stored in 'data_size'.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_data_from_io_contents(struct sail_io *io, void **data, size_t *data_size);

/*
 * Reads a string ended with '\n' from the I/O stream. Trailing new line characters
 * are not stripped. The string buffer size must be >= 2 to hold at least "\n".
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_read_string_from_io(struct sail_io *io, char *str, size_t str_size);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
