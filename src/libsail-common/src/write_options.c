/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "sail-common.h"

int sail_alloc_write_options(struct sail_write_options **write_options) {

    *write_options = (struct sail_write_options *)malloc(sizeof(struct sail_write_options));

    if (*write_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*write_options)->output_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*write_options)->io_options          = 0;
    (*write_options)->compression_type    = 0;
    (*write_options)->compression         = 0;

    return 0;
}

void sail_destroy_write_options(struct sail_write_options *write_options) {

    if (write_options == NULL) {
        return;
    }

    free(write_options);
}

sail_error_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    write_options->output_pixel_format = write_features->preferred_output_pixel_format;
    write_options->io_options = 0;

    if (write_features->features & SAIL_PLUGIN_FEATURE_META_INFO) {
        write_options->io_options |= SAIL_IO_OPTION_META_INFO;
    }

    /* Compression levels are not supported. */
    if (write_features->compression_min == write_features->compression_max) {
        write_options->compression_type = write_features->preferred_compression_type;
    } else {
        write_options->compression = write_features->compression_default;
    }

    return 0;
}

sail_error_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options) {

    SAIL_TRY(sail_alloc_write_options(write_options));
    SAIL_TRY_OR_CLEANUP(sail_write_options_from_features(write_features, *write_options),
                        /* cleanup */ sail_destroy_write_options(*write_options));

    return 0;
}
