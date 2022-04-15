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

sail_status_t sail_alloc_save_options(struct sail_save_options **save_options) {

    SAIL_CHECK_PTR(save_options);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_save_options), &ptr));
    *save_options = ptr;

    (*save_options)->options           = 0;
    (*save_options)->compression       = SAIL_COMPRESSION_UNKNOWN;
    (*save_options)->compression_level = 0;
    (*save_options)->tuning            = NULL;

    return SAIL_OK;
}

void sail_destroy_save_options(struct sail_save_options *save_options) {

    if (save_options == NULL) {
        return;
    }

    sail_free(save_options);
}

sail_status_t sail_alloc_save_options_from_features(const struct sail_save_features *save_features, struct sail_save_options **save_options) {

    struct sail_save_options *save_options_local;
    SAIL_TRY(sail_alloc_save_options(&save_options_local));

    save_options_local->options = 0;

    if (save_features->features & SAIL_CODEC_FEATURE_META_DATA) {
        save_options_local->options |= SAIL_OPTION_META_DATA;
    }

    if (save_features->features & SAIL_CODEC_FEATURE_ICCP) {
        save_options_local->options |= SAIL_OPTION_ICCP;
    }

    save_options_local->compression       = save_features->default_compression;
    save_options_local->compression_level = save_features->compression_level == NULL
                                                ? 0
                                                : save_features->compression_level->default_level;

    *save_options = save_options_local;

    return SAIL_OK;
}

sail_status_t sail_copy_save_options(const struct sail_save_options *source, struct sail_save_options **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_save_options *target_local;
    SAIL_TRY(sail_alloc_save_options(&target_local));

    target_local->options           = source->options;
    target_local->compression       = source->compression;
    target_local->compression_level = source->compression_level;

    if (source->tuning != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_hash_map(source->tuning, &target_local->tuning),
                            /* cleanup */ sail_destroy_save_options(target_local));
    }

    *target = target_local;

    return SAIL_OK;
}
