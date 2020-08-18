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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_status_t io_file_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(read_objects_count);

    FILE *fptr = (FILE *)stream;

    *read_objects_count = fread(buf, object_size, objects_count, fptr);

    return SAIL_OK;
}

static sail_status_t io_file_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fseek(fptr, offset, whence) != 0) {
        sail_print_errno("Failed to seek: %s");
        return SAIL_ERROR_SEEK_IO;
    }

    return SAIL_OK;
}

static sail_status_t io_file_tell(void *stream, size_t *offset) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_PTR(offset);

    FILE *fptr = (FILE *)stream;

    long offset_local = ftell(fptr);

    if (offset_local < 0) {
        sail_print_errno("Failed to get the current I/O position: %s");
        return SAIL_ERROR_TELL_IO;
    }

    *offset = offset_local;

    return SAIL_OK;
}

static sail_status_t io_file_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_BUFFER_PTR(buf);
    SAIL_CHECK_RESULT_PTR(written_objects_count);

    FILE *fptr = (FILE *)stream;

    *written_objects_count = fwrite(buf, object_size, objects_count, fptr);

    return SAIL_OK;
}

static sail_status_t io_file_flush(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fflush(fptr) != 0) {
        sail_print_errno("Failed to flush file buffers: %s");
        return SAIL_ERROR_FLUSH_IO;
    }

    return SAIL_OK;
}

static sail_status_t io_file_close(void *stream) {

    SAIL_CHECK_STREAM_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fclose(fptr) != 0) {
        sail_print_errno("Failed to close the file: %s");
        return SAIL_ERROR_CLOSE_IO;
    }

    return SAIL_OK;
}

static sail_status_t io_file_eof(void *stream, bool *result) {

    SAIL_CHECK_STREAM_PTR(stream);
    SAIL_CHECK_RESULT_PTR(result);

    FILE *fptr = (FILE *)stream;

    *result = feof(fptr);

    return SAIL_OK;
}

static sail_status_t alloc_io_file(const char *path, const char *mode, struct sail_io **io) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_PTR(mode);
    SAIL_CHECK_IO_PTR(io);

    SAIL_LOG_DEBUG("Opening file '%s' in '%s' mode", path, mode);

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
        return SAIL_ERROR_OPEN_FILE;
    }

    SAIL_TRY_OR_CLEANUP(sail_alloc_io(io),
                        /* cleanup */ fclose(fptr));

    (*io)->stream = fptr;

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t alloc_io_read_file(const char *path, struct sail_io **io) {

    SAIL_TRY(alloc_io_file(path, "rb", io));

    (*io)->read  = io_file_read;
    (*io)->seek  = io_file_seek;
    (*io)->tell  = io_file_tell;
    (*io)->write = io_noop_write;
    (*io)->flush = io_noop_flush;
    (*io)->close = io_file_close;
    (*io)->eof   = io_file_eof;

    return SAIL_OK;
}

sail_status_t alloc_io_write_file(const char *path, struct sail_io **io) {

    SAIL_TRY(alloc_io_file(path, "w+b", io));

    (*io)->read  = io_file_read;
    (*io)->seek  = io_file_seek;
    (*io)->tell  = io_file_tell;
    (*io)->write = io_file_write;
    (*io)->flush = io_file_flush;
    (*io)->close = io_file_close;
    (*io)->eof   = io_file_eof;

    return SAIL_OK;
}
