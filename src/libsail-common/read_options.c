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

sail_status_t sail_alloc_read_options(struct sail_read_options **read_options) {

    SAIL_CHECK_PTR(read_options);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_read_options), &ptr));
    *read_options = ptr;

    (*read_options)->io_options = 0;

    return SAIL_OK;
}

void sail_destroy_read_options(struct sail_read_options *read_options) {

    if (read_options == NULL) {
        return;
    }

    sail_free(read_options);
}

sail_status_t sail_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options *read_options) {

    SAIL_CHECK_PTR(read_features);
    SAIL_CHECK_PTR(read_options);

    read_options->io_options = 0;

    if (read_features->features & SAIL_CODEC_FEATURE_META_DATA) {
        read_options->io_options |= SAIL_IO_OPTION_META_DATA;
    }

    if (read_features->features & SAIL_CODEC_FEATURE_INTERLACED) {
        read_options->io_options |= SAIL_IO_OPTION_INTERLACED;
    }

    if (read_features->features & SAIL_CODEC_FEATURE_ICCP) {
        read_options->io_options |= SAIL_IO_OPTION_ICCP;
    }

    return SAIL_OK;
}

sail_status_t sail_alloc_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options **read_options) {

    SAIL_CHECK_PTR(read_options);

    struct sail_read_options *read_options_local;
    SAIL_TRY(sail_alloc_read_options(&read_options_local));
    SAIL_TRY_OR_CLEANUP(sail_read_options_from_features(read_features, read_options_local),
                        /* cleanup */ sail_destroy_read_options(read_options_local));

    *read_options = read_options_local;

    return SAIL_OK;
}

sail_status_t sail_copy_read_options(const struct sail_read_options *source, struct sail_read_options **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_read_options), &ptr));
    *target = ptr;

    memcpy(*target, source, sizeof(struct sail_read_options));

    return SAIL_OK;
}
