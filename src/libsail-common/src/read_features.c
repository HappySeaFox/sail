/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

int sail_alloc_read_features(struct sail_read_features **read_features) {

    *read_features = (struct sail_read_features *)malloc(sizeof(struct sail_read_features));

    if (*read_features == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*read_features)->input_pixel_formats           = NULL;
    (*read_features)->input_pixel_formats_length    = 0;
    (*read_features)->output_pixel_formats          = NULL;
    (*read_features)->output_pixel_formats_length   = 0;
    (*read_features)->preferred_output_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*read_features)->features                      = 0;

    return 0;
}

void sail_destroy_read_features(struct sail_read_features *read_features) {

    if (read_features == NULL) {
        return;
    }

    free(read_features->input_pixel_formats);
    free(read_features->output_pixel_formats);
    free(read_features);
}
