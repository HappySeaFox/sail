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

#ifndef SAIL_LINKED_LIST_NODE_H
#define SAIL_LINKED_LIST_NODE_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Represents a linked list node. It can be used together with the linked list API
 * to build type-specific linked lists which is achieved in C++ by using templates.
 *
 * Type-specific linked list nodes must have exactly two members: a pointer to a value
 * followed by a pointer to the next node. This way pointers to type-specific linked list
 * nodes can be casted to pointers to linked_list_node and the linked list API can be used.
 */
struct linked_list_node {

    /*
     * Node value.
     */
    void *value;

    /*
     * Pointer to the next node or NULL.
     */
    struct linked_list_node *next;
};

typedef sail_status_t (*linked_list_value_allocator_t)(void **value);
typedef sail_status_t (*linked_list_value_copier_t)(const void *source_value, void **target_value);
typedef void (*linked_list_value_deallocator_t)(void *value);

/*
 * Allocates a new linked list node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_private_alloc_linked_list_node(struct linked_list_node **node);

/*
 * Allocates a new linked list node and the nested value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_private_alloc_linked_list_node_and_value(linked_list_value_allocator_t value_allocator,
                                                                        linked_list_value_deallocator_t value_deallocator,
                                                                        struct linked_list_node **node);

/*
 * Destroys the specified linked list node.
 */
SAIL_EXPORT void sail_private_destroy_linked_list_node(struct linked_list_node *node,
                                                       linked_list_value_deallocator_t value_deallocator);

/*
 * Makes a deep copy of the specified linked list node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_private_copy_linked_list_node(const struct linked_list_node *source,
                                                             struct linked_list_node **target,
                                                             linked_list_value_copier_t value_copier,
                                                             linked_list_value_deallocator_t value_deallocator);

/*
 * Destroys the specified linked list node.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_private_destroy_linked_list_node_chain(struct linked_list_node *node,
                                                             linked_list_value_deallocator_t value_deallocator);

/*
 * Makes a deep copy of the specified linked list node chain. If the source chain is NULL, it assigns NULL
 * to the target chain and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_private_copy_linked_list_node_chain(const struct linked_list_node *source,
                                                                   struct linked_list_node **target,
                                                                   linked_list_value_copier_t value_copier,
                                                                   linked_list_value_deallocator_t value_deallocator);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
