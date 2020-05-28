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

sail_error_t sail_alloc_iccp(struct sail_iccp **iccp) {

    *iccp = (struct sail_iccp *)malloc(sizeof(struct sail_iccp));

    if (*iccp == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*iccp)->name        = NULL;
    (*iccp)->data        = NULL;
    (*iccp)->data_length = 0;

    return 0;
}

void sail_destroy_iccp(struct sail_iccp *iccp) {

    if (iccp == NULL) {
        return;
    }

    free(iccp->name);
    free(iccp->data);
    free(iccp);
}

sail_error_t sail_copy_iccp(const struct sail_iccp *source_iccp, struct sail_iccp **target_iccp) {

    SAIL_CHECK_ICCP_PTR(source_iccp);
    SAIL_CHECK_ICCP_PTR(target_iccp);

    SAIL_TRY(sail_alloc_iccp(target_iccp));

    SAIL_TRY_OR_CLEANUP(sail_strdup(source_iccp->name, &(*target_iccp)->name),
                        /* cleanup */ sail_destroy_iccp(*target_iccp));

    (*target_iccp)->data = malloc(source_iccp->data_length);

    if ((*target_iccp)->data == NULL) {
        sail_destroy_iccp(*target_iccp);
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy((*target_iccp)->data, source_iccp->data, source_iccp->data_length);

    (*target_iccp)->data_length = source_iccp->data_length;

    return 0;
}
