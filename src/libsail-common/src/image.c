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

int sail_alloc_image(struct sail_image **image) {

    *image = (struct sail_image *)malloc(sizeof(struct sail_image));

    if (*image == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*image)->width                = 0;
    (*image)->height               = 0;
    (*image)->bytes_per_line       = 0;
    (*image)->pixel_format         = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->passes               = 0;
    (*image)->animated             = false;
    (*image)->delay                = 0;
    (*image)->palette_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->palette              = NULL;
    (*image)->palette_size         = 0;
    (*image)->meta_entry_node      = NULL;
    (*image)->properties           = 0;
    (*image)->source_pixel_format  = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->source_properties    = 0;

    return 0;
}

void sail_destroy_image(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    free(image->palette);

    sail_destroy_meta_entry_node_chain(image->meta_entry_node);

    free(image);
}
