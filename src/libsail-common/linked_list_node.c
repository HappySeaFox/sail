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

#include "sail-common.h"

sail_status_t sail_private_alloc_linked_list_node(struct linked_list_node **node) {

    SAIL_CHECK_PTR(node);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct linked_list_node), &ptr));
    *node = ptr;

    (*node)->value = NULL;
    (*node)->next  = NULL;

    return SAIL_OK;
}

sail_status_t sail_private_alloc_linked_list_node_and_value(linked_list_value_allocator_t value_allocator,
                                                            linked_list_value_deallocator_t value_deallocator,
                                                            struct linked_list_node **node)
{
    SAIL_CHECK_PTR(node);

    struct linked_list_node *node_local;
    SAIL_TRY(sail_private_alloc_linked_list_node(&node_local));

    SAIL_TRY_OR_CLEANUP(value_allocator(&node_local->value),
                        /* cleanup */ sail_private_destroy_linked_list_node(node_local, value_deallocator));

    *node = node_local;

    return SAIL_OK;
}

void sail_private_destroy_linked_list_node(struct linked_list_node *node,
                                           linked_list_value_deallocator_t value_deallocator) {

    if (node == NULL) {
        return;
    }

    value_deallocator(node->value);
    sail_free(node);
}

sail_status_t sail_private_copy_linked_list_node(const struct linked_list_node *source,
                                                 struct linked_list_node **target,
                                                 linked_list_value_copier_t value_copier,
                                                 linked_list_value_deallocator_t value_deallocator) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct linked_list_node *node_local;
    SAIL_TRY(sail_private_alloc_linked_list_node(&node_local));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(value_copier(source->value, &ptr),
                        /* cleanup */ sail_private_destroy_linked_list_node(node_local, value_deallocator));
    node_local->value = ptr;

    *target = node_local;

    return SAIL_OK;
}

void sail_private_destroy_linked_list_node_chain(struct linked_list_node *node,
                                                 linked_list_value_deallocator_t value_deallocator) {

    while (node != NULL) {
        struct linked_list_node *node_next = node->next;

        sail_private_destroy_linked_list_node(node, value_deallocator);

        node = node_next;
    }
}

sail_status_t sail_private_copy_linked_list_node_chain(const struct linked_list_node *source,
                                                       struct linked_list_node **target,
                                                       linked_list_value_copier_t value_copier,
                                                       linked_list_value_deallocator_t value_deallocator) {

    SAIL_CHECK_PTR(target);

    if (source == NULL) {
        *target = NULL;
        return SAIL_OK;
    }

    struct linked_list_node *node_local = NULL;
    struct linked_list_node *linked_list_node_current = NULL;

    while (source != NULL) {
        struct linked_list_node *linked_list_node = NULL;

        SAIL_TRY_OR_CLEANUP(sail_private_copy_linked_list_node(source, &linked_list_node, value_copier, value_deallocator),
                            /* cleanup */ sail_private_destroy_linked_list_node_chain(node_local, value_deallocator));

        if (node_local == NULL) {
            node_local = linked_list_node;
            linked_list_node_current = node_local;
        } else {
            linked_list_node_current->next = linked_list_node;
            linked_list_node_current = linked_list_node_current->next;
        }

        source = source->next;
    }

    *target = node_local;

    return SAIL_OK;
}
