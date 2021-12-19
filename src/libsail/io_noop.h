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

#ifndef SAIL_IO_NOOP_H
#define SAIL_IO_NOOP_H

#include <stdbool.h>
#include <stddef.h>

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
 * No-op callbacks for I/O streams when specific operations cannot be executed.
 * For example, when we open a file for reading, its tolerant_write callback is set to
 * sail_io_noop_tolerant_write.
 */

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_tolerant_read(void *stream, void *buf, size_t size_to_read, size_t *read_size);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_strict_read(void *stream, void *buf, size_t size_to_read);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_seek(void *stream, long offset, int whence);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_tell(void *stream, size_t *offset);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_tolerant_write(void *stream, const void *buf, size_t size_to_write, size_t *written_size);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_strict_write(void *stream, const void *buf, size_t size_to_write);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_flush(void *stream);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_close(void *stream);

/*
 * No-op callback that just returns SAIL_ERROR_NOT_IMPLEMENTED.
 */
SAIL_EXPORT sail_status_t sail_io_noop_eof(void *stream, bool *result);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
