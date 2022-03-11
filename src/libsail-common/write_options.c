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

sail_status_t sail_alloc_write_options(struct sail_write_options **write_options) {

    SAIL_CHECK_PTR(write_options);

    struct sail_write_options *write_options_local;

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_write_options), &ptr));
    write_options_local = ptr;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&write_options_local->codec_options),
                        /* cleanup */ sail_destroy_write_options(write_options_local));

    write_options_local->compression       = SAIL_COMPRESSION_UNSUPPORTED;
    write_options_local->compression_level = 0;

    *write_options = write_options_local;

    return SAIL_OK;
}

void sail_destroy_write_options(struct sail_write_options *write_options) {

    if (write_options == NULL) {
        return;
    }

    sail_destroy_hash_map(write_options->codec_options);
    sail_free(write_options);
}

sail_status_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options) {

    SAIL_CHECK_PTR(write_features);
    SAIL_CHECK_PTR(write_options);

    sail_clear_hash_map(write_options->codec_options);

    if (write_features->features & SAIL_CODEC_FEATURE_META_DATA) {
        sail_put_meta_data_codec_option(write_options->codec_options, true);
    }

    if (write_features->features & SAIL_CODEC_FEATURE_ICCP) {
        sail_put_iccp_codec_option(write_options->codec_options, true);
    }

    write_options->compression = write_features->default_compression;
    write_options->compression_level = write_features->compression_level_default;

    return SAIL_OK;
}

sail_status_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options) {

    struct sail_write_options *write_options_local;
    SAIL_TRY(sail_alloc_write_options(&write_options_local));
    SAIL_TRY_OR_CLEANUP(sail_write_options_from_features(write_features, write_options_local),
                        /* cleanup */ sail_destroy_write_options(write_options_local));

    *write_options = write_options_local;

    return SAIL_OK;
}

sail_status_t sail_copy_write_options(const struct sail_write_options *source, struct sail_write_options **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_write_options), &ptr));
    struct sail_write_options *target_local = ptr;

    SAIL_TRY_OR_CLEANUP(sail_copy_hash_map(source->codec_options, &target_local->codec_options),
                        /* cleanup */ sail_destroy_write_options(target_local));

    target_local->compression       = source->compression;
    target_local->compression_level = source->compression_level;

    *target = target_local;

    return SAIL_OK;
}
