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
