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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sail/sail.h>

struct expanding_buffer_stream
{
    size_t capacity;
    size_t size;
    size_t pos;
    void* buffer;
    double growth_factor; /* Growth factor (1.5x by default). */
};

/*
 * Private functions.
 */

static sail_status_t io_expanding_buffer_tolerant_read(void* stream, void* buf, size_t size_to_read, size_t* read_size)
{
    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(read_size);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    *read_size = 0;

    if (expanding_buffer_stream->pos >= expanding_buffer_stream->size)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EOF);
    }

    size_t actual_size_to_read = (expanding_buffer_stream->pos + size_to_read > expanding_buffer_stream->size)
                                     ? expanding_buffer_stream->size - expanding_buffer_stream->pos
                                     : size_to_read;

    memcpy(buf, (const char*)expanding_buffer_stream->buffer + expanding_buffer_stream->pos, actual_size_to_read);
    expanding_buffer_stream->pos += actual_size_to_read;

    *read_size = actual_size_to_read;

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_strict_read(void* stream, void* buf, size_t size_to_read)
{
    size_t read_size;

    SAIL_TRY(io_expanding_buffer_tolerant_read(stream, buf, size_to_read, &read_size));

    if (read_size != size_to_read)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_tolerant_write(void* stream, const void* buf, size_t size_to_write, size_t* written_size)
{
    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(written_size);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    *written_size = 0;

    /* Calculate required capacity. */
    size_t required_capacity = expanding_buffer_stream->pos + size_to_write;

    /* Expand buffer if necessary. */
    if (required_capacity > expanding_buffer_stream->capacity)
    {
        size_t new_capacity = expanding_buffer_stream->capacity;

        /* Grow by growth_factor until we have enough space. */
        while (new_capacity < required_capacity)
        {
            new_capacity = (size_t)(new_capacity * expanding_buffer_stream->growth_factor);

            /* Ensure minimum growth. */
            if (new_capacity <= expanding_buffer_stream->capacity)
            {
                new_capacity = expanding_buffer_stream->capacity + size_to_write;
            }
        }

        SAIL_LOG_DEBUG("Expanding buffer from %zu to %zu bytes", expanding_buffer_stream->capacity, new_capacity);

        SAIL_TRY(sail_realloc(new_capacity, &expanding_buffer_stream->buffer));
        expanding_buffer_stream->capacity = new_capacity;
    }

    /* Write data. */
    memcpy((char*)expanding_buffer_stream->buffer + expanding_buffer_stream->pos, buf, size_to_write);
    expanding_buffer_stream->pos += size_to_write;

    /* Update the actual data size. */
    if (expanding_buffer_stream->pos > expanding_buffer_stream->size)
    {
        expanding_buffer_stream->size = expanding_buffer_stream->pos;
    }

    *written_size = size_to_write;

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_strict_write(void* stream, const void* buf, size_t size_to_write)
{
    size_t written_size;

    SAIL_TRY(io_expanding_buffer_tolerant_write(stream, buf, size_to_write, &written_size));

    if (written_size != size_to_write)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_WRITE_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_seek(void* stream, long offset, int whence)
{
    SAIL_CHECK_PTR(stream);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    size_t new_pos;

    switch (whence)
    {
    case SEEK_SET:
    {
        new_pos = offset;
        break;
    }

    case SEEK_CUR:
    {
        new_pos = expanding_buffer_stream->pos + offset;
        break;
    }

    case SEEK_END:
    {
        new_pos = expanding_buffer_stream->size + offset;
        break;
    }

    default:
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_SEEK_WHENCE);
    }
    }

    /* Allow seeking beyond the current size (it's valid for writing). */
    expanding_buffer_stream->pos = new_pos;

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_tell(void* stream, size_t* offset)
{
    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(offset);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    *offset = expanding_buffer_stream->pos;

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_flush(void* stream)
{
    SAIL_CHECK_PTR(stream);

    /* Nothing to flush for memory buffer. */
    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_close(void* stream)
{
    SAIL_CHECK_PTR(stream);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    sail_free(expanding_buffer_stream->buffer);
    sail_free(expanding_buffer_stream);

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_eof(void* stream, bool* result)
{
    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(result);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    *result = expanding_buffer_stream->pos >= expanding_buffer_stream->size;

    return SAIL_OK;
}

static sail_status_t io_expanding_buffer_size(void* stream, size_t* size)
{
    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(size);

    struct expanding_buffer_stream* expanding_buffer_stream = stream;

    *size = expanding_buffer_stream->size;

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_alloc_io_write_expanding_buffer(size_t initial_capacity, struct sail_io** io)
{
    SAIL_CHECK_PTR(io);

    if (initial_capacity == 0)
    {
        SAIL_LOG_ERROR("Initial capacity must be greater than 0");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_LOG_DEBUG("Creating expanding buffer with initial capacity %zu bytes", initial_capacity);

    struct sail_io* io_local;
    SAIL_TRY(sail_alloc_io(&io_local));

    void* ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct expanding_buffer_stream), &ptr),
                        /* cleanup */ sail_destroy_io(io_local));
    struct expanding_buffer_stream* expanding_buffer_stream = ptr;

    expanding_buffer_stream->capacity      = initial_capacity;
    expanding_buffer_stream->size          = 0;
    expanding_buffer_stream->pos           = 0;
    expanding_buffer_stream->buffer        = NULL;
    expanding_buffer_stream->growth_factor = 1.5;

    SAIL_TRY_OR_CLEANUP(sail_malloc(initial_capacity, &expanding_buffer_stream->buffer),
                        /* cleanup */ sail_free(expanding_buffer_stream), sail_destroy_io(io_local));

    io_local->features       = SAIL_IO_FEATURE_SEEKABLE;
    io_local->stream         = expanding_buffer_stream;
    io_local->tolerant_read  = io_expanding_buffer_tolerant_read;
    io_local->strict_read    = io_expanding_buffer_strict_read;
    io_local->tolerant_write = io_expanding_buffer_tolerant_write;
    io_local->strict_write   = io_expanding_buffer_strict_write;
    io_local->seek           = io_expanding_buffer_seek;
    io_local->tell           = io_expanding_buffer_tell;
    io_local->flush          = io_expanding_buffer_flush;
    io_local->close          = io_expanding_buffer_close;
    io_local->eof            = io_expanding_buffer_eof;
    io_local->size           = io_expanding_buffer_size;

    *io = io_local;

    return SAIL_OK;
}

sail_status_t sail_io_expanding_buffer_size(struct sail_io* io, size_t* size)
{
    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(io->stream);
    SAIL_CHECK_PTR(size);

    struct expanding_buffer_stream* expanding_buffer_stream = io->stream;

    *size = expanding_buffer_stream->size;

    return SAIL_OK;
}
