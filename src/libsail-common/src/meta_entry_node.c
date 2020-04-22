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

int sail_alloc_meta_entry_node(struct sail_meta_entry_node **meta_entry_node) {

    *meta_entry_node = (struct sail_meta_entry_node *)malloc(sizeof(struct sail_meta_entry_node));

    if (*meta_entry_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*meta_entry_node)->next = NULL;
    (*meta_entry_node)->key = NULL;
    (*meta_entry_node)->value = NULL;

    return 0;
}

void sail_destroy_meta_entry_node(struct sail_meta_entry_node *meta_entry_node) {

    if (meta_entry_node == NULL) {
        return;
    }

    free(meta_entry_node->key);
    free(meta_entry_node->value);
    free(meta_entry_node);
}

void sail_destroy_meta_entry_node_chain(struct sail_meta_entry_node *meta_entry_node) {

    while (meta_entry_node != NULL) {
        struct sail_meta_entry_node *meta_entry_node_next = meta_entry_node->next;

        sail_destroy_meta_entry_node(meta_entry_node);

        meta_entry_node = meta_entry_node_next;
    }
}
