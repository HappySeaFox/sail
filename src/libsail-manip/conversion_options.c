/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include "sail-manip.h"

sail_status_t sail_alloc_conversion_options(struct sail_conversion_options **options) {

    SAIL_CHECK_PTR(options);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_conversion_options), &ptr));
    *options = ptr;

    (*options)->options      = SAIL_CONVERSION_OPTION_DROP_ALPHA;
    (*options)->background48 = (sail_rgb48_t){ 0, 0, 0 };
    (*options)->background24 = (sail_rgb24_t){ 0, 0, 0 };

    return SAIL_OK;
}

void sail_destroy_conversion_options(struct sail_conversion_options *options) {

    if (options == NULL) {
        return;
    }

    sail_free(options);
}
