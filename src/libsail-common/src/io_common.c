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

#include <stdlib.h>

#include "sail-common.h"

sail_error_t sail_alloc_io(struct sail_io **io) {

    SAIL_CHECK_IO_PTR(io);

    *io = (struct sail_io *)malloc(sizeof(struct sail_io));

    if (*io == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*io)->stream = NULL;
    (*io)->read   = NULL;
    (*io)->seek   = NULL;
    (*io)->tell   = NULL;
    (*io)->write  = NULL;
    (*io)->flush  = NULL;
    (*io)->close  = NULL;
    (*io)->eof    = NULL;

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

    free(io);
}
