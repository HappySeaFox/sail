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

#include "sail-common.h"

sail_status_t sail_alloc_resolution(struct sail_resolution **resolution) {

    SAIL_TRY(sail_alloc_resolution_from_data(SAIL_RESOLUTION_UNIT_UNKNOWN, 0, 0, resolution));

    return SAIL_OK;
}

sail_status_t sail_alloc_resolution_from_data(enum SailResolutionUnit unit, double x, double y, struct sail_resolution **resolution) {

    SAIL_CHECK_PTR(resolution);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_resolution), &ptr));
    *resolution = ptr;

    (*resolution)->unit = unit;
    (*resolution)->x    = x;
    (*resolution)->y    = y;

    return SAIL_OK;
}

void sail_destroy_resolution(struct sail_resolution *resolution) {

    if (resolution == NULL) {
        return;
    }

    sail_free(resolution);
}

sail_status_t sail_copy_resolution(struct sail_resolution *source, struct sail_resolution **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    SAIL_TRY(sail_alloc_resolution(target));

    (*target)->unit = source->unit;
    (*target)->x    = source->x;
    (*target)->y    = source->y;

    return SAIL_OK;
}
