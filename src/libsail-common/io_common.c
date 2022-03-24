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

#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_status_t sail_alloc_io(struct sail_io **io) {

    SAIL_CHECK_PTR(io);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_io), &ptr));
    *io = ptr;

    (*io)->id             = 0;
    (*io)->features       = 0;
    (*io)->stream         = NULL;
    (*io)->tolerant_read  = NULL;
    (*io)->strict_read    = NULL;
    (*io)->tolerant_write = NULL;
    (*io)->strict_write   = NULL;
    (*io)->seek           = NULL;
    (*io)->tell           = NULL;
    (*io)->flush          = NULL;
    (*io)->close          = NULL;
    (*io)->eof            = NULL;

    return SAIL_OK;
}

void sail_destroy_io(struct sail_io *io) {

    if (io == NULL) {
        return;
    }

    if (io->close != NULL && io->stream != NULL) {
        SAIL_TRY_OR_EXECUTE(io->close(io->stream),
                            /* on error */ sail_print_errno("Failed to close the I/O stream: %s"));
    }

    sail_free(io);
}

sail_status_t sail_check_io_valid(const struct sail_io *io)
{
    SAIL_CHECK_PTR(io);

    if (io->id == 0U                   ||
            io->tolerant_read  == NULL ||
            io->strict_read    == NULL ||
            io->tolerant_write == NULL ||
            io->strict_write   == NULL ||
            io->seek           == NULL ||
            io->tell           == NULL ||
            io->flush          == NULL ||
            io->close          == NULL ||
            io->eof            == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IO);
    }

    return SAIL_OK;
}

sail_status_t sail_io_size(struct sail_io *io, size_t *size) {

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(size);

    /* Save the current position. */
    size_t saved_position;
    SAIL_TRY(io->tell(io->stream, &saved_position));

    size_t size_local;
    SAIL_TRY(io->seek(io->stream, 0, SEEK_END));
    SAIL_TRY(io->tell(io->stream, &size_local));
    SAIL_TRY(io->seek(io->stream, (long)saved_position, SEEK_SET));

    *size = size_local - saved_position;

    return SAIL_OK;
}

sail_status_t sail_io_contents_into_data(struct sail_io *io, void *data) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(data);

    /* Save the current position. */
    size_t saved_position;
    SAIL_TRY(io->tell(io->stream, &saved_position));

    unsigned char buffer[4096];
    unsigned char *data_ptr = data;
    size_t actually_read;

    sail_status_t status;

    /* Read stream. */
    while ((status = io->tolerant_read(io->stream, buffer, sizeof(buffer), &actually_read)) == SAIL_OK) {
        memcpy(data_ptr, buffer, actually_read);
        data_ptr += actually_read;
    }

    SAIL_TRY(io->seek(io->stream, (long)saved_position, SEEK_SET));

    if (status != SAIL_ERROR_EOF) {
        SAIL_LOG_ERROR("Failed to read from the I/O stream, error #%d", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_IO);
    }

    return SAIL_OK;
}

sail_status_t sail_alloc_data_from_io_contents(struct sail_io *io, void **data, size_t *data_size) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(data_size);

    /* Save the current position. */
    size_t saved_position;
    SAIL_TRY(io->tell(io->stream, &saved_position));

    size_t data_size_local;
    SAIL_TRY(sail_io_size(io, &data_size_local));

    /* Read stream. */
    void *data_local;
    SAIL_TRY(sail_malloc(data_size_local, &data_local));

    SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, data_local, data_size_local),
                        /* cleanup */ sail_free(data_local));

    /* Seek back. */
    SAIL_TRY(io->seek(io->stream, (long)saved_position, SEEK_SET));

    *data = data_local;
    *data_size = data_size_local;

    return SAIL_OK;
}

sail_status_t sail_read_string_from_io(struct sail_io *io, char *str, size_t str_size) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(str);

    if (str_size < 2) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    unsigned i = 0;

    do {
        SAIL_TRY(io->strict_read(io->stream, str + i++, 1));
    } while(i < str_size - 1 && str[i - 1] != '\n');

    /* Buffer is full and no trailing \n was seen. */
    if (str[i - 1] != '\n') {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_IO);
    }

    str[i] = '\0';

    return SAIL_OK;
}
