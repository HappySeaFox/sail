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

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

/*
 * Private functions.
 */

static sail_error_t io_mem_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(read_objects_count);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_STREAM_PTR(stream);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_tell(void *stream, long *offset) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_PTR(offset);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(written_objects_count);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_flush(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_close(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t io_mem_eof(void *stream, bool *result) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_RESULT_PTR(result);

    return SAIL_NOT_IMPLEMENTED;
}

static sail_error_t assign_io_callbacks(struct sail_io *io) {

    SAIL_CHECK_IO_PTR(io);

    /* FIXME */
    io->stream = NULL;

    io->read  = io_mem_read;
    io->seek  = io_mem_seek;
    io->tell  = io_mem_tell;
    io->write = io_mem_write;
    io->flush = io_mem_flush;
    io->close = io_mem_close;
    io->eof   = io_mem_eof;

    return 0;
}

/*
 * Public functions.
 */

sail_error_t sail_alloc_io_read_mem(const void *buffer, long buffer_length, struct sail_io **io) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IO_PTR(io);

    /* FIXME */
    return SAIL_NOT_IMPLEMENTED;

    SAIL_TRY_OR_CLEANUP(assign_io_callbacks(*io),
                        /* cleanup */ sail_destroy_io(*io));

    return 0;
}

sail_error_t sail_alloc_io_write_mem(void *buffer, long buffer_length, struct sail_io **io) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IO_PTR(io);

    /* FIXME */
    return SAIL_NOT_IMPLEMENTED;

    SAIL_TRY_OR_CLEANUP(assign_io_callbacks(*io),
                        /* cleanup */ sail_destroy_io(*io));

    return 0;
}
