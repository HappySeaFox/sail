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

#include "sail-common.h"
#include "sail.h"

sail_error_t io_noop_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    (void)stream;
    (void)buf;
    (void)object_size;
    (void)objects_count;
    (void)read_objects_count;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_seek(void *stream, long offset, int whence) {

    (void)stream;
    (void)offset;
    (void)whence;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_tell(void *stream, long *offset) {

    (void)stream;
    (void)offset;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    (void)stream;
    (void)buf;
    (void)object_size;
    (void)objects_count;
    (void)written_objects_count;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_flush(void *stream) {

    (void)stream;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_close(void *stream) {

    (void)stream;

    return SAIL_NOT_IMPLEMENTED;
}

sail_error_t io_noop_eof(void *stream, bool *result) {

    (void)stream;
    (void)result;

    return SAIL_NOT_IMPLEMENTED;
}
