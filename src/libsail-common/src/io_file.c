/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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
#include <string.h>

#include "sail-common.h"

sail_error_t sail_io_file_read(void *stream, void *buf, size_t object_size, size_t objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);

    FILE *fptr = (FILE *)stream;

    if (fread(buf, object_size, objects_count, fptr) < objects_count) {
        SAIL_LOG_ERROR("Failed to read from the file: %s", feof(fptr) ? "end of file" : "unknown error");
        return feof(fptr) ? SAIL_IO_EOF : SAIL_IO_READ_ERROR;
    }

    return 0;
}

sail_error_t sail_io_file_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fseek(fptr, offset, whence) != 0) {
        sail_print_errno("Failed to seek: %s");
        return SAIL_IO_SEEK_ERROR;
    }

    return 0;
}

sail_error_t sail_io_file_tell(void *stream, long *offset) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_PTR(offset);

    FILE *fptr = (FILE *)stream;

    *offset = ftell(fptr);

    if (*offset < 0) {
        sail_print_errno("Failed to get the current I/O position: %s");
        return SAIL_IO_TELL_ERROR;
    }

    return 0;
}

sail_error_t sail_io_file_write(void *stream, const void *buf, size_t object_size, size_t objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);

    FILE *fptr = (FILE *)stream;

    if (fwrite(buf, object_size, objects_count, fptr) < objects_count) {
        SAIL_LOG_ERROR("Failed to write to the file");
        return SAIL_IO_WRITE_ERROR;
    }

    return 0;
}

sail_error_t sail_io_file_flush(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fflush(fptr) != 0) {
        sail_print_errno("Failed to flush file buffers: %s");
        return SAIL_IO_FLUSH_ERROR;
    }

    return 0;
}

sail_error_t sail_io_file_close(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fclose(fptr) != 0) {
        sail_print_errno("Failed to close the file: %s");
        return SAIL_IO_CLOSE_ERROR;
    }

    return 0;
}
