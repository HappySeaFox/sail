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

#include "sail-common.h"
#include "sail.h"

sail_status_t io_noop_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    (void)stream;
    (void)buf;
    (void)object_size;
    (void)objects_count;
    (void)read_objects_count;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_seek(void *stream, long offset, int whence) {

    (void)stream;
    (void)offset;
    (void)whence;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_tell(void *stream, size_t *offset) {

    (void)stream;
    (void)offset;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    (void)stream;
    (void)buf;
    (void)object_size;
    (void)objects_count;
    (void)written_objects_count;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_flush(void *stream) {

    (void)stream;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_close(void *stream) {

    (void)stream;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}

sail_status_t io_noop_eof(void *stream, bool *result) {

    (void)stream;
    (void)result;

    return SAIL_ERROR_NOT_IMPLEMENTED;
}
