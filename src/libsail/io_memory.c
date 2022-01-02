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

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail.h"

struct mem_io_buffer_info {

    /* Total buffer size. */
    size_t length;

    /*
     * The length of accessible buffer length.
     *
     * For example:
     *
     *   1. Reading
     *      - open buffer of 10 Mb for reading
     *      - accessible length now is 10 Mb
     *
     *   2. Writing
     *      - open buffer of 10 Mb for writing
     *      - write 100 Kb
     *      - accessible length now is 100 Kb
     */
    size_t accessible_length;

    /* Current stream position. */
    size_t pos;
};

struct mem_io_read_stream {
    struct mem_io_buffer_info mem_io_buffer_info;
    const void *buffer;
};

struct mem_io_write_stream {
    struct mem_io_buffer_info mem_io_buffer_info;
    void *buffer;
};

/*
 * Private functions.
 */

static sail_status_t io_memory_tolerant_read(void *stream, void *buf, size_t size_to_read, size_t *read_size) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(read_size);

    struct mem_io_read_stream *mem_io_read_stream = (struct mem_io_read_stream *)stream;
    struct mem_io_buffer_info *mem_io_buffer_info = &mem_io_read_stream->mem_io_buffer_info;

    *read_size = 0;

    if (mem_io_buffer_info->pos >= mem_io_buffer_info->accessible_length) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EOF);
    }

    size_t actual_size_to_read = (mem_io_buffer_info->pos + size_to_read > mem_io_buffer_info->accessible_length)
                                 ? mem_io_buffer_info->accessible_length - mem_io_buffer_info->pos
                                 : size_to_read;

    memcpy(buf, (const char *)mem_io_read_stream->buffer + mem_io_buffer_info->pos, actual_size_to_read);
    mem_io_buffer_info->pos += actual_size_to_read;

    *read_size = actual_size_to_read;

    return SAIL_OK;
}

static sail_status_t io_memory_strict_read(void *stream, void *buf, size_t size_to_read) {

    size_t read_size;

    SAIL_TRY(io_memory_tolerant_read(stream, buf, size_to_read, &read_size));

    if (read_size != size_to_read) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_memory_tolerant_write(void *stream, const void *buf, size_t size_to_write, size_t *written_size) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(written_size);

    struct mem_io_write_stream *mem_io_write_stream = (struct mem_io_write_stream *)stream;
    struct mem_io_buffer_info *mem_io_buffer_info = &mem_io_write_stream->mem_io_buffer_info;

    *written_size = 0;

    if (mem_io_buffer_info->pos >= mem_io_buffer_info->length) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EOF);
    }

    size_t actual_size_to_write = (mem_io_buffer_info->pos + size_to_write > mem_io_buffer_info->length)
                                  ? mem_io_buffer_info->length - mem_io_buffer_info->pos
                                  : size_to_write;

    memcpy((char *)mem_io_write_stream->buffer + mem_io_buffer_info->pos, buf, actual_size_to_write);
    mem_io_buffer_info->pos += actual_size_to_write;

    *written_size = actual_size_to_write;

    /* Update the accessible length in case of overflow. */
    if (mem_io_buffer_info->pos >= mem_io_buffer_info->accessible_length) {
        mem_io_buffer_info->accessible_length = mem_io_buffer_info->pos;
    }

    return SAIL_OK;
}

static sail_status_t io_memory_strict_write(void *stream, const void *buf, size_t size_to_write) {

    size_t written_size;

    SAIL_TRY(io_memory_tolerant_write(stream, buf, size_to_write, &written_size));

    if (written_size != size_to_write) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_WRITE_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_memory_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_PTR(stream);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    size_t new_pos;

    switch (whence) {
        case SEEK_SET: {
            new_pos = offset;
            break;
        }

        case SEEK_CUR: {
            new_pos = mem_io_buffer_info->pos + offset;
            break;
        }

        case SEEK_END: {
            new_pos = mem_io_buffer_info->accessible_length + offset;
            break;
        }

        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_SEEK_WHENCE);
        }
    }

    /* Correct the value. */
    if (new_pos >= mem_io_buffer_info->length) {
        new_pos = mem_io_buffer_info->length;
        mem_io_buffer_info->accessible_length = mem_io_buffer_info->length;
    } else if (new_pos >= mem_io_buffer_info->accessible_length) {
        mem_io_buffer_info->accessible_length = new_pos + 1;
    }

    mem_io_buffer_info->pos = new_pos;

    return SAIL_OK;
}

static sail_status_t io_memory_tell(void *stream, size_t *offset) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(offset);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    *offset = mem_io_buffer_info->pos;

    return SAIL_OK;
}

static sail_status_t io_memory_flush(void *stream) {

    SAIL_CHECK_PTR(stream);

    return SAIL_OK;
}

static sail_status_t io_memory_close(void *stream) {

    SAIL_CHECK_PTR(stream);

    sail_free(stream);

    return SAIL_OK;
}

static sail_status_t io_memory_eof(void *stream, bool *result) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(result);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    *result = mem_io_buffer_info->pos >= mem_io_buffer_info->accessible_length;

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_alloc_io_read_memory(const void *buffer, size_t length, struct sail_io **io) {

    SAIL_CHECK_PTR(buffer);
    SAIL_CHECK_PTR(io);

    SAIL_LOG_DEBUG("Opening memory buffer of size %lu for reading", length);

    struct sail_io *io_local;
    SAIL_TRY(sail_alloc_io(&io_local));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct mem_io_read_stream), &ptr),
                        /* cleanup */ sail_destroy_io(io_local));
    struct mem_io_read_stream *mem_io_read_stream = ptr;

    mem_io_read_stream->mem_io_buffer_info.length            = length;
    mem_io_read_stream->mem_io_buffer_info.accessible_length = length;
    mem_io_read_stream->mem_io_buffer_info.pos               = 0;
    mem_io_read_stream->buffer                               = buffer;

    io_local->id             = SAIL_MEMORY_IO_ID;
    io_local->stream         = mem_io_read_stream;
    io_local->tolerant_read  = io_memory_tolerant_read;
    io_local->strict_read    = io_memory_strict_read;
    io_local->tolerant_write = sail_io_noop_tolerant_write;
    io_local->strict_write   = sail_io_noop_strict_write;
    io_local->seek           = io_memory_seek;
    io_local->tell           = io_memory_tell;
    io_local->flush          = sail_io_noop_flush;
    io_local->close          = io_memory_close;
    io_local->eof            = io_memory_eof;

    *io = io_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_io_read_write_memory(void *buffer, size_t length, struct sail_io **io) {

    SAIL_CHECK_PTR(buffer);
    SAIL_CHECK_PTR(io);

    SAIL_LOG_DEBUG("Opening memory buffer of size %lu for writing", length);

    struct sail_io *io_local;
    SAIL_TRY(sail_alloc_io(&io_local));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct mem_io_write_stream), &ptr),
                        /* cleanup */ sail_destroy_io(io_local));
    struct mem_io_write_stream *mem_io_write_stream = ptr;

    mem_io_write_stream->mem_io_buffer_info.length            = length;
    mem_io_write_stream->mem_io_buffer_info.accessible_length = 0;
    mem_io_write_stream->mem_io_buffer_info.pos               = 0;
    mem_io_write_stream->buffer                               = buffer;

    io_local->id             = SAIL_MEMORY_IO_ID;
    io_local->features       = SAIL_IO_FEATURE_SEEKABLE;
    io_local->stream         = mem_io_write_stream;
    io_local->tolerant_read  = io_memory_tolerant_read;
    io_local->strict_read    = io_memory_strict_read;
    io_local->tolerant_write = io_memory_tolerant_write;
    io_local->strict_write   = io_memory_strict_write;
    io_local->seek           = io_memory_seek;
    io_local->tell           = io_memory_tell;
    io_local->flush          = io_memory_flush;
    io_local->close          = io_memory_close;
    io_local->eof            = io_memory_eof;

    *io = io_local;

    return SAIL_OK;
}
