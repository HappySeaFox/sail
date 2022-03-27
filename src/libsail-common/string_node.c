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

#include <string.h>

#include "sail-common.h"

sail_status_t sail_alloc_string_node(struct sail_string_node **node) {

    SAIL_TRY(sail_private_alloc_linked_list_node((struct linked_list_node **)node));

    return SAIL_OK;
}

void sail_destroy_string_node(struct sail_string_node *node) {

    sail_private_destroy_linked_list_node((struct linked_list_node *)node,
                                          (linked_list_value_deallocator_t)&sail_free);
}

sail_status_t sail_copy_string_node(const struct sail_string_node *source, struct sail_string_node **target) {

    SAIL_TRY(sail_private_copy_linked_list_node((const struct linked_list_node *)source,
                                                (struct linked_list_node **)target,
                                                (linked_list_value_copier_t)&sail_strdup,
                                                (linked_list_value_deallocator_t)&sail_free));

    return SAIL_OK;
}

void sail_destroy_string_node_chain(struct sail_string_node *node) {

    sail_private_destroy_linked_list_node_chain((struct linked_list_node *)node,
                                                (linked_list_value_deallocator_t)&sail_free);
}

sail_status_t sail_copy_string_node_chain(const struct sail_string_node *source, struct sail_string_node **target) {

    SAIL_TRY(sail_private_copy_linked_list_node_chain((const struct linked_list_node *)source,
                                                      (struct linked_list_node **)target,
                                                      (linked_list_value_copier_t)&sail_strdup,
                                                      (linked_list_value_deallocator_t)&sail_free));

    return SAIL_OK;
}

sail_status_t sail_split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node) {

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(target_string_node);

    struct sail_string_node *result_string_node = NULL;
    struct sail_string_node **last_string_node = &result_string_node;

    const char *separator = ";";

    while (*(value += strspn(value, separator)) != '\0') {
        size_t length = strcspn(value, separator);

        struct sail_string_node *string_node;
        SAIL_TRY(sail_alloc_string_node(&string_node));

        SAIL_TRY_OR_CLEANUP(sail_strdup_length(value, length, &string_node->string),
                            /* cleanup */ sail_destroy_string_node_chain(result_string_node),
                                          sail_destroy_string_node(string_node));

        *last_string_node = string_node;
        last_string_node = &string_node->next;

        value += length;
    }

    *target_string_node = result_string_node;

    return SAIL_OK;
}
