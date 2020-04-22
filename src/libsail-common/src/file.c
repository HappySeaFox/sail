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

static int sail_alloc_file_private(struct sail_file **file) {

    *file = (struct sail_file *)malloc(sizeof(struct sail_file));

    if (*file == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*file)->fptr = NULL;
    (*file)->pimpl = NULL;

    return 0;
}

int sail_alloc_file(const char *path, const char *mode, struct sail_file **file) {

    SAIL_CHECK_PATH_PTR(path);

    /* Try to open the file first */
    FILE *fptr;

#ifdef SAIL_WIN32
    fptr = _fsopen(path, mode, _SH_DENYWR);
#else
    /* Fallback to a regular fopen() */
    fptr = fopen(path, mode);
#endif

    if (fptr == NULL) {
#ifdef SAIL_WIN32
        char buffer[80];
        strerror_s(buffer, sizeof(buffer), errno);
        SAIL_LOG_ERROR("Failed to open '%s': %s", path, buffer);
#else
        SAIL_LOG_ERROR("Failed to open '%s': %s", path, strerror(errno));
#endif
        return SAIL_FILE_OPEN_ERROR;
    }

    SAIL_TRY(sail_alloc_file_private(file));

    (*file)->fptr = fptr;

    return 0;
}

sail_error_t sail_alloc_file_for_reading(const char *path, struct sail_file **file) {

    SAIL_TRY(sail_alloc_file(path, "rb", file));

    return 0;
}

sail_error_t sail_alloc_file_for_writing(const char *path, struct sail_file **file) {

    SAIL_TRY(sail_alloc_file(path, "wb", file));

    return 0;
}

void sail_destroy_file(struct sail_file *file) {

    if (file == NULL) {
        return;
    }

    if (file->fptr != NULL) {
        fclose(file->fptr);
    }

    free(file->pimpl);
    free(file);
}
