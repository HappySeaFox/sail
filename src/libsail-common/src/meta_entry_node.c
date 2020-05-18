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

sail_error_t sail_alloc_meta_entry_node(struct sail_meta_entry_node **meta_entry_node) {

    *meta_entry_node = (struct sail_meta_entry_node *)malloc(sizeof(struct sail_meta_entry_node));

    if (*meta_entry_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*meta_entry_node)->key   = NULL;
    (*meta_entry_node)->value = NULL;
    (*meta_entry_node)->next  = NULL;

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

sail_error_t sail_copy_meta_entry_node(struct sail_meta_entry_node *source_meta_entry_node, struct sail_meta_entry_node **target_meta_entry_node) {

    SAIL_CHECK_META_ENTRY_NODE_PTR(source_meta_entry_node);
    SAIL_CHECK_META_ENTRY_NODE_PTR(target_meta_entry_node);

    SAIL_TRY(sail_alloc_meta_entry_node(target_meta_entry_node));

    SAIL_TRY_OR_CLEANUP(sail_strdup(source_meta_entry_node->key, &(*target_meta_entry_node)->key),
                        /* cleanup */ sail_destroy_meta_entry_node(*target_meta_entry_node));
    SAIL_TRY_OR_CLEANUP(sail_strdup(source_meta_entry_node->value, &(*target_meta_entry_node)->value),
                        /* cleanup */ sail_destroy_meta_entry_node(*target_meta_entry_node));

    return 0;
}

void sail_destroy_meta_entry_node_chain(struct sail_meta_entry_node *meta_entry_node) {

    while (meta_entry_node != NULL) {
        struct sail_meta_entry_node *meta_entry_node_next = meta_entry_node->next;

        sail_destroy_meta_entry_node(meta_entry_node);

        meta_entry_node = meta_entry_node_next;
    }
}

sail_error_t sail_copy_meta_entry_node_chain(struct sail_meta_entry_node *source_meta_entry_node, struct sail_meta_entry_node **target_meta_entry_node) {

    SAIL_CHECK_META_ENTRY_NODE_PTR(target_meta_entry_node);

    if (source_meta_entry_node == NULL) {
        *target_meta_entry_node = NULL;
        return 0;
    }

    *target_meta_entry_node = NULL;
    struct sail_meta_entry_node *meta_entry_node_current = NULL;

    while (source_meta_entry_node != NULL) {
        struct sail_meta_entry_node *meta_entry_node = NULL;

        SAIL_TRY_OR_CLEANUP(sail_copy_meta_entry_node(source_meta_entry_node, &meta_entry_node),
                            /* cleanup */sail_destroy_meta_entry_node_chain(*target_meta_entry_node));

        if (*target_meta_entry_node == NULL) {
            *target_meta_entry_node = meta_entry_node;
            meta_entry_node_current = *target_meta_entry_node;
        } else {
            meta_entry_node_current->next = meta_entry_node;
            meta_entry_node_current = meta_entry_node_current->next;
        }

        source_meta_entry_node = source_meta_entry_node->next;
    }

    return 0;
}
