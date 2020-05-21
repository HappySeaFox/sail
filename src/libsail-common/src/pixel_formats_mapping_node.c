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

sail_error_t sail_alloc_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node **node) {

    *node = (struct sail_pixel_formats_mapping_node *)malloc(sizeof(struct sail_pixel_formats_mapping_node));

    if (*node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*node)->input_pixel_format          = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*node)->output_pixel_formats        = NULL;
    (*node)->output_pixel_formats_length = 0;
    (*node)->next                        = NULL;

    return 0;
}

void sail_destroy_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node *node) {

    if (node == NULL) {
        return;
    }

    free(node->output_pixel_formats);
    free(node);
}

void sail_destroy_pixel_formats_mapping_node_chain(struct sail_pixel_formats_mapping_node *node) {

    while (node != NULL) {
        struct sail_pixel_formats_mapping_node *node_next = node->next;

        sail_destroy_pixel_formats_mapping_node(node);

        node = node_next;
    }
}
