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

sail_status_t sail_alloc_load_options(struct sail_load_options **load_options) {

    SAIL_CHECK_PTR(load_options);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_load_options), &ptr));
    *load_options = ptr;

    (*load_options)->options = 0;
    (*load_options)->tuning  = NULL;

    return SAIL_OK;
}

void sail_destroy_load_options(struct sail_load_options *load_options) {

    if (load_options == NULL) {
        return;
    }

    sail_destroy_hash_map(load_options->tuning);
    sail_free(load_options);
}

sail_status_t sail_alloc_load_options_from_features(const struct sail_load_features *load_features, struct sail_load_options **load_options) {

    SAIL_CHECK_PTR(load_options);

    struct sail_load_options *load_options_local;
    SAIL_TRY(sail_alloc_load_options(&load_options_local));

    load_options_local->options = 0;

    if (load_features->features & SAIL_CODEC_FEATURE_META_DATA) {
        load_options_local->options |= SAIL_OPTION_META_DATA;
    }

    if (load_features->features & SAIL_CODEC_FEATURE_ICCP) {
        load_options_local->options |= SAIL_OPTION_ICCP;
    }

    *load_options = load_options_local;

    return SAIL_OK;
}

sail_status_t sail_copy_load_options(const struct sail_load_options *source, struct sail_load_options **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_load_options *target_local;
    SAIL_TRY(sail_alloc_load_options(&target_local));

    target_local->options = source->options;

    if (source->tuning != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_hash_map(source->tuning, &target_local->tuning),
                            /* cleanup */ sail_destroy_load_options(target_local));
    }

    *target = target_local;

    return SAIL_OK;
}
