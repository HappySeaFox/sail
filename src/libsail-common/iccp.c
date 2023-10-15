/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_status_t sail_alloc_iccp(struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(iccp);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_iccp), &ptr));
    *iccp = ptr;

    (*iccp)->data = NULL;
    (*iccp)->size = 0;

    return SAIL_OK;
}

sail_status_t sail_alloc_iccp_from_data(const void *data, size_t data_size, struct sail_iccp **iccp) {

    struct sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp_for_data(data_size, &iccp_local));

    memcpy(iccp_local->data, data, data_size);

    *iccp = iccp_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_iccp_from_shallow_data(void *data, size_t data_size, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(iccp);

    SAIL_TRY(sail_alloc_iccp(iccp));

    (*iccp)->data = data;
    (*iccp)->size = data_size;

    return SAIL_OK;
}

sail_status_t sail_alloc_iccp_for_data(size_t data_size, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(iccp);

    struct sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp(&iccp_local));

    SAIL_TRY_OR_CLEANUP(sail_malloc(data_size, &iccp_local->data),
                        /* cleanup */ sail_destroy_iccp(iccp_local));

    iccp_local->size = data_size;

    *iccp = iccp_local;

    return SAIL_OK;
}

void sail_destroy_iccp(struct sail_iccp *iccp) {

    if (iccp == NULL) {
        return;
    }

    sail_free(iccp->data);
    sail_free(iccp);
}

sail_status_t sail_copy_iccp(const struct sail_iccp *source_iccp, struct sail_iccp **target_iccp) {

    SAIL_CHECK_PTR(source_iccp);
    SAIL_CHECK_PTR(target_iccp);

    struct sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp(&iccp_local));

    SAIL_TRY_OR_CLEANUP(sail_malloc(source_iccp->size, &iccp_local->data),
                        /* cleanup */ sail_destroy_iccp(iccp_local));

    memcpy(iccp_local->data, source_iccp->data, source_iccp->size);
    iccp_local->size = source_iccp->size;

    *target_iccp = iccp_local;

    return SAIL_OK;
}
