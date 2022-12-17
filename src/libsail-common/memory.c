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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"

sail_status_t sail_malloc(size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = malloc(size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

sail_status_t sail_realloc(size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = realloc(*ptr, size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

sail_status_t sail_calloc(size_t nmemb, size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = calloc(nmemb, size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

void sail_free(void *ptr) {

    free(ptr);
}
