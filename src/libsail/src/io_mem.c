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

#include "sail-common.h"
#include "sail.h"

struct mem_io_buffer_info {
    size_t buffer_length;
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

static sail_error_t io_mem_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(read_objects_count);

    struct mem_io_read_stream *mem_io_read_stream = (struct mem_io_read_stream *)stream;
    struct mem_io_buffer_info *mem_io_buffer_info = &mem_io_read_stream->mem_io_buffer_info;

    *read_objects_count = 0;

    if (mem_io_buffer_info->pos >= mem_io_buffer_info->buffer_length) {
        return SAIL_IO_EOF;
    }

    while (mem_io_buffer_info->pos <= mem_io_buffer_info->buffer_length - object_size && objects_count > 0) {
        memcpy(buf, (const char *)mem_io_read_stream->buffer + mem_io_buffer_info->pos, object_size);

        buf = (char *)buf + object_size;
        mem_io_buffer_info->pos += object_size;

        (*read_objects_count)++;
        objects_count--;
    }

    return 0;
}

static sail_error_t io_mem_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_STREAM_PTR(stream);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    switch (whence) {
        case SEEK_SET: {
            /* Cannot seek to -2. Seek to the beginning. */
            if (offset < 0) {
                mem_io_buffer_info->pos = 0;
            }
            /* Cannot seek past buffer. Seek to the EOF. */
            else if ((size_t)offset > mem_io_buffer_info->buffer_length) {
                mem_io_buffer_info->pos = mem_io_buffer_info->buffer_length;
            } else {
                mem_io_buffer_info->pos = offset;
            }
            break;
        }

        case SEEK_CUR: {
            /* Cannot seek to -2. Seek to the beginning. */
            if (offset < 0 && (size_t)(-offset) > mem_io_buffer_info->pos) {
                mem_io_buffer_info->pos = 0;
            }
            /* Cannot seek past buffer. Seek to the EOF. */
            else if (mem_io_buffer_info->pos + offset > mem_io_buffer_info->buffer_length) {
                mem_io_buffer_info->pos = mem_io_buffer_info->buffer_length;
            } else {
                mem_io_buffer_info->pos += offset;
            }
            break;
        }

        case SEEK_END: {
            /* Cannot seek past buffer. Seek to the EOF. */
            if (offset >= 0) {
                mem_io_buffer_info->pos = mem_io_buffer_info->buffer_length;
            }
            /* Cannot seek to -2. Seek to the beginning. */
            else if (offset < 0 && (size_t)(-offset) > mem_io_buffer_info->buffer_length) {
                mem_io_buffer_info->pos = 0;
            } else {
                mem_io_buffer_info->pos = mem_io_buffer_info->buffer_length + offset;
            }
            break;
        }
    }

    return 0;
}

static sail_error_t io_mem_tell(void *stream, size_t *offset) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_PTR(offset);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    *offset = mem_io_buffer_info->pos;

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(written_objects_count);

    struct mem_io_write_stream *mem_io_write_stream = (struct mem_io_write_stream *)stream;
    struct mem_io_buffer_info *mem_io_buffer_info = &mem_io_write_stream->mem_io_buffer_info;

    *written_objects_count = 0;

    if (mem_io_buffer_info->pos >= mem_io_buffer_info->buffer_length) {
        return SAIL_IO_EOF;
    }

    while (mem_io_buffer_info->pos <= mem_io_buffer_info->buffer_length - object_size && objects_count > 0) {
        memcpy((char *)mem_io_write_stream->buffer + mem_io_buffer_info->pos, buf, object_size);

        buf = (const char *)buf + object_size;
        mem_io_buffer_info->pos += object_size;

        (*written_objects_count)++;
        objects_count--;
    }

    return 0;
}

static sail_error_t io_mem_flush(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    return 0;
}

static sail_error_t io_mem_close(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    sail_free(stream);

    return 0;
}

static sail_error_t io_mem_eof(void *stream, bool *result) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_RESULT_PTR(result);

    struct mem_io_buffer_info *mem_io_buffer_info = (struct mem_io_buffer_info *)stream;

    *result = mem_io_buffer_info->pos >= mem_io_buffer_info->buffer_length;

    return 0;
}

/*
 * Public functions.
 */

sail_error_t alloc_io_read_mem(const void *buffer, size_t buffer_length, struct sail_io **io) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IO_PTR(io);

    SAIL_LOG_DEBUG("Opening memory buffer of size %lu for reading", buffer_length);

    SAIL_TRY(sail_alloc_io(io));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(&ptr, sizeof(struct mem_io_read_stream)),
                        /* cleanup */ sail_destroy_io(*io));
    struct mem_io_read_stream *mem_io_read_stream = ptr;

    mem_io_read_stream->mem_io_buffer_info.buffer_length = buffer_length;
    mem_io_read_stream->mem_io_buffer_info.pos           = 0;
    mem_io_read_stream->buffer                           = buffer;

    (*io)->stream = mem_io_read_stream;
    (*io)->read   = io_mem_read;
    (*io)->seek   = io_mem_seek;
    (*io)->tell   = io_mem_tell;
    (*io)->write  = io_noop_write;
    (*io)->flush  = io_noop_flush;
    (*io)->close  = io_mem_close;
    (*io)->eof    = io_mem_eof;

    return 0;
}

sail_error_t alloc_io_write_mem(void *buffer, size_t buffer_length, struct sail_io **io) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IO_PTR(io);

    SAIL_LOG_DEBUG("Opening memory buffer of size %lu for writing", buffer_length);

    SAIL_TRY(sail_alloc_io(io));

    struct mem_io_write_stream *mem_io_write_stream = (struct mem_io_write_stream *)malloc(sizeof(struct mem_io_write_stream));

    if (mem_io_write_stream == NULL) {
        sail_destroy_io(*io);
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    mem_io_write_stream->mem_io_buffer_info.buffer_length = buffer_length;
    mem_io_write_stream->mem_io_buffer_info.pos           = 0;
    mem_io_write_stream->buffer                           = buffer;

    (*io)->stream = mem_io_write_stream;
    (*io)->read   = io_noop_read;
    (*io)->seek   = io_mem_seek;
    (*io)->tell   = io_mem_tell;
    (*io)->write  = io_mem_write;
    (*io)->flush  = io_mem_flush;
    (*io)->close  = io_mem_close;
    (*io)->eof    = io_mem_eof;

    return 0;
}
