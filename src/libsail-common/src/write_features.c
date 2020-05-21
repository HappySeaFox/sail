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

int sail_alloc_write_features(struct sail_write_features **write_features) {

    *write_features = (struct sail_write_features *)malloc(sizeof(struct sail_write_features));

    if (*write_features == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*write_features)->pixel_formats_mapping_node            = NULL;
    (*write_features)->features                              = 0;
    (*write_features)->properties                            = 0;
    (*write_features)->passes                                = 0;
    (*write_features)->compression_types                     = NULL;
    (*write_features)->compression_types_length              = 0;
    (*write_features)->preferred_compression_type            = 0;
    (*write_features)->compression_min                       = 0;
    (*write_features)->compression_max                       = 0;
    (*write_features)->compression_default                   = 0;

    return 0;
}

void sail_destroy_write_features(struct sail_write_features *write_features) {

    if (write_features == NULL) {
        return;
    }

    sail_destroy_pixel_formats_mapping_node_chain(write_features->pixel_formats_mapping_node);
    free(write_features->compression_types);
    free(write_features);
}
