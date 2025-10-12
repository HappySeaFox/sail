/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#pragma once

#include <stddef.h> /* size_t */

#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_io;

/*
 * Allocates a new I/O object for an automatically expanding memory buffer.
 * The buffer starts with the specified initial capacity and grows automatically
 * using sail_realloc() when writing beyond the current capacity. The growth factor
 * is 1.25x.
 *
 * The actual data size written can be retrieved with sail_io_expanding_buffer_size().
 * The underlying buffer can be extracted with sail_io_expanding_buffer_extract() or
 * sail_io_expanding_buffer_data().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_io_write_expanding_buffer(size_t initial_capacity, struct sail_io** io);

/*
 * Returns the current size of data written to the expanding buffer.
 * This is different from the buffer capacity.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_io_expanding_buffer_size(struct sail_io* io, size_t* size);

/* extern "C" */
#ifdef __cplusplus
}
#endif
