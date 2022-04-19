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

sail_status_t sail_alloc_source_image(struct sail_source_image **source_image) {

    SAIL_CHECK_PTR(source_image);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_source_image), &ptr));
    *source_image = ptr;

    (*source_image)->pixel_format       = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*source_image)->chroma_subsampling = SAIL_CHROMA_SUBSAMPLING_UNKNOWN;
    (*source_image)->orientation        = SAIL_ORIENTATION_NORMAL;
    (*source_image)->compression        = SAIL_COMPRESSION_UNKNOWN;
    (*source_image)->interlaced         = false;
    (*source_image)->special_properties = NULL;

    return SAIL_OK;
}

void sail_destroy_source_image(struct sail_source_image *source_image) {

    if (source_image == NULL) {
        return;
    }

    sail_destroy_hash_map(source_image->special_properties);
    sail_free(source_image);
}

sail_status_t sail_copy_source_image(const struct sail_source_image *source, struct sail_source_image **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_source_image *target_local;
    SAIL_TRY(sail_alloc_source_image(&target_local));

    target_local->pixel_format       = source->pixel_format;
    target_local->chroma_subsampling = source->chroma_subsampling;
    target_local->orientation        = source->orientation;
    target_local->compression        = source->compression;
    target_local->interlaced         = source->interlaced;

    if (source->special_properties != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_hash_map(source->special_properties, &target_local->special_properties),
                            /* cleanup */ sail_destroy_source_image(target_local));
    }

    *target = target_local;

    return SAIL_OK;
}
