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

    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_write_options)));
    *write_options = ptr;

    (*write_options)->output_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*write_options)->io_options          = 0;
    (*write_options)->compression_type    = 0;
    (*write_options)->compression         = 0;

    return SAIL_OK;
}

void sail_destroy_write_options(struct sail_write_options *write_options) {

    if (write_options == NULL) {
        return;
    }

    sail_free(write_options);
}

sail_status_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    write_options->output_pixel_format = SAIL_PIXEL_FORMAT_AUTO;
    write_options->io_options = 0;

    if (write_features->features & SAIL_PLUGIN_FEATURE_META_INFO) {
        write_options->io_options |= SAIL_IO_OPTION_META_INFO;
    }

    if (write_features->features & SAIL_PLUGIN_FEATURE_INTERLACED) {
        write_options->io_options |= SAIL_IO_OPTION_INTERLACED;
    }

    if (write_features->features & SAIL_PLUGIN_FEATURE_ICCP) {
        write_options->io_options |= SAIL_IO_OPTION_ICCP;
    }

    /* Compression levels are not supported. */
    if (write_features->compression_min == write_features->compression_max) {
        write_options->compression_type = write_features->default_compression_type;
    } else {
        write_options->compression = write_features->compression_default;
    }

    return SAIL_OK;
}

sail_status_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options) {

    SAIL_TRY(sail_alloc_write_options(write_options));
    SAIL_TRY_OR_CLEANUP(sail_write_options_from_features(write_features, *write_options),
                        /* cleanup */ sail_destroy_write_options(*write_options));

    return SAIL_OK;
}

sail_status_t sail_copy_write_options(const struct sail_write_options *write_options_source, struct sail_write_options **write_options_target) {

    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options_source);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options_target);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_write_options)));
    *write_options_target = ptr;

    memcpy(*write_options_target, write_options_source, sizeof(struct sail_write_options));

    return SAIL_OK;
}
