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

int sail_alloc_read_options(struct sail_read_options **read_options) {

    *read_options = (struct sail_read_options *)malloc(sizeof(struct sail_read_options));

    if (*read_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*read_options)->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*read_options)->io_options   = 0;

    return 0;
}

void sail_destroy_read_options(struct sail_read_options *read_options) {

    if (read_options == NULL) {
        return;
    }

    free(read_options);
}

sail_error_t sail_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options *read_options) {

    SAIL_CHECK_READ_FEATURES_PTR(read_features);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    read_options->pixel_format = read_features->preferred_output_pixel_format;
    read_options->io_options = 0;

    if (read_features->features & SAIL_PLUGIN_FEATURE_META_INFO) {
        read_options->io_options |= SAIL_IO_OPTION_META_INFO;
    }

    return 0;
}

sail_error_t sail_alloc_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options **read_options) {

    SAIL_TRY(sail_alloc_read_options(read_options));
    SAIL_TRY_OR_CLEANUP(sail_read_options_from_features(read_features, *read_options),
                        /* cleanup */ sail_destroy_read_options(*read_options));

    return 0;
}
