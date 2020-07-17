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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_error_t sail_alloc_iccp(struct sail_iccp **iccp) {

    SAIL_TRY(sail_malloc(iccp, sizeof(struct sail_iccp)));

    (*iccp)->data        = NULL;
    (*iccp)->data_length = 0;

    return 0;
}

sail_error_t sail_alloc_iccp_with_data(struct sail_iccp **iccp, const void *data, unsigned data_length) {

    SAIL_CHECK_ICCP_PTR(iccp);

    SAIL_TRY(sail_alloc_iccp(iccp));

    SAIL_TRY_OR_CLEANUP(sail_malloc(&(*iccp)->data, data_length),
                        /* cleanup */ sail_destroy_iccp(*iccp));

    memcpy((*iccp)->data, data, data_length);
    (*iccp)->data_length = data_length;

    return 0;
}

void sail_destroy_iccp(struct sail_iccp *iccp) {

    if (iccp == NULL) {
        return;
    }

    sail_free(iccp->data);
    sail_free(iccp);
}

sail_error_t sail_copy_iccp(const struct sail_iccp *source_iccp, struct sail_iccp **target_iccp) {

    SAIL_CHECK_ICCP_PTR(source_iccp);
    SAIL_CHECK_ICCP_PTR(target_iccp);

    SAIL_TRY(sail_alloc_iccp(target_iccp));

    SAIL_TRY_OR_CLEANUP(sail_malloc(&(*target_iccp)->data, source_iccp->data_length),
                        /* cleanup */ sail_destroy_iccp(*target_iccp));

    memcpy((*target_iccp)->data, source_iccp->data, source_iccp->data_length);

    (*target_iccp)->data_length = source_iccp->data_length;

    return 0;
}
