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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_error_t sail_alloc_icc(struct sail_icc **icc) {

    *icc = (struct sail_icc *)malloc(sizeof(struct sail_icc));

    if (*icc == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*icc)->name        = NULL;
    (*icc)->data        = NULL;
    (*icc)->data_length = 0;

    return 0;
}

void sail_destroy_icc(struct sail_icc *icc) {

    if (icc == NULL) {
        return;
    }

    free(icc->name);
    free(icc->data);
    free(icc);
}

sail_error_t sail_copy_icc(const struct sail_icc *source_icc, struct sail_icc **target_icc) {

    SAIL_CHECK_ICC_PTR(source_icc);
    SAIL_CHECK_ICC_PTR(target_icc);

    SAIL_TRY(sail_alloc_icc(target_icc));

    SAIL_TRY_OR_CLEANUP(sail_strdup(source_icc->name, &(*target_icc)->name),
                        /* cleanup */ sail_destroy_icc(*target_icc));

    (*target_icc)->data = malloc(source_icc->data_length);

    if ((*target_icc)->data == NULL) {
        sail_destroy_icc(*target_icc);
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy((*target_icc)->data, source_icc->data, source_icc->data_length);

    (*target_icc)->data_length = source_icc->data_length;

    return 0;
}
