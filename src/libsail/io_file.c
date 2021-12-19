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

#ifdef _MSC_VER
    /* _SH_DENYWR */
    #include <share.h>
#endif

#include "sail.h"

/*
 * Private functions.
 */

static sail_status_t io_file_tolerant_read(void *stream, void *buf, size_t size_to_read, size_t *read_size) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(read_size);

    FILE *fptr = (FILE *)stream;

    *read_size = fread(buf, 1, size_to_read, fptr);

    return SAIL_OK;
}

static sail_status_t io_file_strict_read(void *stream, void *buf, size_t size_to_read) {

    size_t read_size;

    SAIL_TRY(io_file_tolerant_read(stream, buf, size_to_read, &read_size));

    if (read_size != size_to_read) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_file_tolerant_write(void *stream, const void *buf, size_t size_to_write, size_t *written_size) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(buf);
    SAIL_CHECK_PTR(written_size);

    FILE *fptr = (FILE *)stream;

    *written_size = fwrite(buf, 1, size_to_write, fptr);

    return SAIL_OK;
}

static sail_status_t io_file_strict_write(void *stream, const void *buf, size_t size_to_write) {

    size_t written_size;

    SAIL_TRY(io_file_tolerant_write(stream, buf, size_to_write, &written_size));

    if (written_size != size_to_write) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_WRITE_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_file_seek(void *stream, long offset, int whence) {

    SAIL_CHECK_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fseek(fptr, offset, whence) != 0) {
        sail_print_errno("Failed to seek: %s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_SEEK_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_file_tell(void *stream, size_t *offset) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(offset);

    FILE *fptr = (FILE *)stream;

    long offset_local = ftell(fptr);

    if (offset_local < 0) {
        sail_print_errno("Failed to get the current I/O position: %s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_TELL_IO);
    }

    *offset = offset_local;

    return SAIL_OK;
}

static sail_status_t io_file_flush(void *stream) {

    SAIL_CHECK_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fflush(fptr) != 0) {
        sail_print_errno("Failed to flush file buffers: %s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_FLUSH_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_file_close(void *stream) {

    SAIL_CHECK_PTR(stream);

    FILE *fptr = (FILE *)stream;

    if (fclose(fptr) != 0) {
        sail_print_errno("Failed to close the file: %s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CLOSE_IO);
    }

    return SAIL_OK;
}

static sail_status_t io_file_eof(void *stream, bool *result) {

    SAIL_CHECK_PTR(stream);
    SAIL_CHECK_PTR(result);

    FILE *fptr = (FILE *)stream;

    *result = feof(fptr);

    return SAIL_OK;
}

static sail_status_t alloc_io_file(const char *path, const char *mode, struct sail_io **io) {

    SAIL_CHECK_PTR(path);
    SAIL_CHECK_PTR(mode);
    SAIL_CHECK_PTR(io);

    SAIL_LOG_DEBUG("Opening file '%s' in '%s' mode", path, mode);

    /* Try to open the file first */
    FILE *fptr;

#ifdef _MSC_VER
    fptr = _fsopen(path, mode, _SH_DENYWR);
#else
    /* Fallback to a regular fopen() */
    fptr = fopen(path, mode);
#endif

    if (fptr == NULL) {
        sail_print_errno("Failed to open the specified file: %s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    SAIL_TRY_OR_CLEANUP(sail_alloc_io(io),
                        /* cleanup */ fclose(fptr));

    (*io)->id     = SAIL_FILE_IO_ID;
    (*io)->stream = fptr;

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_alloc_io_read_file(const char *path, struct sail_io **io) {

    SAIL_TRY(alloc_io_file(path, "rb", io));

    (*io)->features       = SAIL_IO_FEATURE_SEEKABLE;
    (*io)->tolerant_read  = io_file_tolerant_read;
    (*io)->strict_read    = io_file_strict_read;
    (*io)->tolerant_write = sail_io_noop_tolerant_write;
    (*io)->strict_write   = sail_io_noop_strict_write;
    (*io)->seek           = io_file_seek;
    (*io)->tell           = io_file_tell;
    (*io)->flush          = sail_io_noop_flush;
    (*io)->close          = io_file_close;
    (*io)->eof            = io_file_eof;

    return SAIL_OK;
}

sail_status_t sail_alloc_io_write_file(const char *path, struct sail_io **io) {

    SAIL_TRY(alloc_io_file(path, "w+b", io));

    (*io)->tolerant_read  = io_file_tolerant_read;
    (*io)->strict_read    = io_file_strict_read;
    (*io)->tolerant_write = io_file_tolerant_write;
    (*io)->strict_write   = io_file_strict_write;
    (*io)->seek           = io_file_seek;
    (*io)->tell           = io_file_tell;
    (*io)->flush          = io_file_flush;
    (*io)->close          = io_file_close;
    (*io)->eof            = io_file_eof;

    return SAIL_OK;
}
