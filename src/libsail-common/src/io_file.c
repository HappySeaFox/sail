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
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "sail-common.h"

/*
 * Private functions.
 */

static sail_error_t sail_io_file_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(read_objects_count);

    FILE *fptr = (FILE *)stream;

    *read_objects_count = fread(buf, object_size, objects_count, fptr);

    return 0;
}

static sail_error_t sail_io_file_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fseek(fptr, offset, whence) != 0) {
        sail_print_errno("Failed to seek: %s");
        return SAIL_IO_SEEK_ERROR;
    }

    return 0;
}

static sail_error_t sail_io_file_tell(void *stream, long *offset) {

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

static sail_error_t sail_io_file_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(written_objects_count);

    FILE *fptr = (FILE *)stream;

    *written_objects_count = fwrite(buf, object_size, objects_count, fptr);

    return 0;
}

static sail_error_t sail_io_file_flush(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fflush(fptr) != 0) {
        sail_print_errno("Failed to flush file buffers: %s");
        return SAIL_IO_FLUSH_ERROR;
    }

    return 0;
}

static sail_error_t sail_io_file_close(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fclose(fptr) != 0) {
        sail_print_errno("Failed to close the file: %s");
        return SAIL_IO_CLOSE_ERROR;
    }

    return 0;
}

static sail_error_t sail_io_file_eof(void *stream, bool *result) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_RESULT_PTR(result);

    FILE *fptr = (FILE *)stream;

    *result = feof(fptr);

    return 0;
}

static sail_error_t sail_alloc_io_file(const char *path, const char *mode, struct sail_io **io) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IO_PTR(io);

    /* Try to open the file first */
    FILE *fptr;

#ifdef SAIL_WIN32
    fptr = _fsopen(path, mode, _SH_DENYWR);
#else
    /* Fallback to a regular fopen() */
    fptr = fopen(path, mode);
#endif

    if (fptr == NULL) {
        sail_print_errno("Failed to open the specified file: %s");
        return SAIL_FILE_OPEN_ERROR;
    }

    SAIL_TRY_OR_CLEANUP(sail_alloc_io(io),
                        /* cleanup */ fclose(fptr));

    (*io)->stream = fptr;

    (*io)->read  = sail_io_file_read;
    (*io)->seek  = sail_io_file_seek;
    (*io)->tell  = sail_io_file_tell;
    (*io)->write = sail_io_file_write;
    (*io)->flush = sail_io_file_flush;
    (*io)->close = sail_io_file_close;
    (*io)->eof   = sail_io_file_eof;

    return 0;
}

/*
 * Public functions.
 */

sail_error_t sail_alloc_io_read_file(const char *path, struct sail_io **io) {

    SAIL_TRY(sail_alloc_io_file(path, "rb", io));

    return 0;
}

sail_error_t sail_alloc_io_write_file(const char *path, struct sail_io **io) {

    SAIL_TRY(sail_alloc_io_file(path, "wb", io));

    return 0;
}
