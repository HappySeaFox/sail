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

static sail_error_t sail_alloc_io(struct sail_io **io) {

    *io = (struct sail_io *)malloc(sizeof(struct sail_io));

    if (*io == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*io)->stream = NULL;
    (*io)->pimpl  = NULL;
    (*io)->read   = NULL;
    (*io)->seek   = NULL;
    (*io)->tell   = NULL;
    (*io)->write  = NULL;
    (*io)->close  = NULL;

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
    (*io)->close = sail_io_file_close;

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

void sail_destroy_io(struct sail_io *io) {

    if (io == NULL) {
        return;
    }

    if (io->close != NULL && io->stream != NULL) {
        if (io->close(io->stream) != 0) {
            sail_print_errno("Failed to close the I/O stream: %s");
        }
    }

    free(io->pimpl);
    free(io);
}
