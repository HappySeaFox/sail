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

sail_status_t sail_alloc_meta_data_node(struct sail_meta_data_node **node) {

    SAIL_CHECK_PTR(node);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_meta_data_node), &ptr));
    *node = ptr;

    (*node)->meta_data = NULL;
    (*node)->next      = NULL;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_and_value(struct sail_meta_data_node **node) {

    SAIL_CHECK_PTR(node);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data(&node_local->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    *node = node_local;

    return SAIL_OK;
}

void sail_destroy_meta_data_node(struct sail_meta_data_node *node) {

    if (node == NULL) {
        return;
    }

    sail_destroy_meta_data(node->meta_data);
    sail_free(node);
}

sail_status_t sail_copy_meta_data_node(const struct sail_meta_data_node *source, struct sail_meta_data_node **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    SAIL_TRY_OR_CLEANUP(sail_copy_meta_data(source->meta_data, &node_local->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    *target = node_local;

    return SAIL_OK;
}

void sail_destroy_meta_data_node_chain(struct sail_meta_data_node *node) {

    while (node != NULL) {
        struct sail_meta_data_node *node_next = node->next;

        sail_destroy_meta_data_node(node);

        node = node_next;
    }
}

sail_status_t sail_copy_meta_data_node_chain(const struct sail_meta_data_node *source, struct sail_meta_data_node **target) {

    SAIL_CHECK_PTR(target);

    if (source == NULL) {
        *target = NULL;
        return SAIL_OK;
    }

    struct sail_meta_data_node *node_local = NULL;
    struct sail_meta_data_node *meta_data_node_current = NULL;

    while (source != NULL) {
        struct sail_meta_data_node *meta_data_node = NULL;

        SAIL_TRY_OR_CLEANUP(sail_copy_meta_data_node(source, &meta_data_node),
                            /* cleanup */ sail_destroy_meta_data_node_chain(node_local));

        if (node_local == NULL) {
            node_local = meta_data_node;
            meta_data_node_current = node_local;
        } else {
            meta_data_node_current->next = meta_data_node;
            meta_data_node_current = meta_data_node_current->next;
        }

        source = source->next;
    }

    *target = node_local;

    return SAIL_OK;
}
