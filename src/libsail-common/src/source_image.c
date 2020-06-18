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

sail_error_t sail_alloc_source_image(struct sail_source_image **source_image) {

    SAIL_CHECK_IMAGE_PTR(source_image);

    *source_image = (struct sail_source_image *)malloc(sizeof(struct sail_source_image));

    if (*source_image == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*source_image)->pixel_format     = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*source_image)->properties       = 0;
    (*source_image)->compression_type = SAIL_COMPRESSION_UNSUPPORTED;

    return 0;
}

void sail_destroy_source_image(struct sail_source_image *source_image) {

    if (source_image == NULL) {
        return;
    }

    free(source_image);
}

sail_error_t sail_copy_source_image(const struct sail_source_image *source, struct sail_source_image **target) {

    SAIL_CHECK_IMAGE_PTR(source);
    SAIL_CHECK_IMAGE_PTR(target);

    SAIL_TRY(sail_alloc_source_image(target));

    (*target)->pixel_format     = source->pixel_format;
    (*target)->properties       = source->properties;
    (*target)->compression_type = source->compression_type;

    return 0;
}
