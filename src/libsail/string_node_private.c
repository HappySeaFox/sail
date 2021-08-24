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

#include "sail.h"

sail_status_t alloc_string_node(struct sail_string_node **string_node) {

    SAIL_CHECK_STRING_NODE_PTR(string_node);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_string_node), &ptr));
    *string_node = ptr;

    (*string_node)->value = NULL;
    (*string_node)->next  = NULL;

    return SAIL_OK;
}

void destroy_string_node(struct sail_string_node *string_node) {

    if (string_node == NULL) {
        return;
    }

    sail_free(string_node->value);
    sail_free(string_node);
}

void destroy_string_node_chain(struct sail_string_node *string_node) {

    while (string_node != NULL) {
        struct sail_string_node *string_node_next = string_node->next;

        destroy_string_node(string_node);

        string_node = string_node_next;
    }
}

sail_status_t split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node) {

    struct sail_string_node **last_string_node = target_string_node;

    while (*(value += strspn(value, ";")) != '\0') {
        size_t length = strcspn(value, ";");

        struct sail_string_node *string_node;

        SAIL_TRY(alloc_string_node(&string_node));

        SAIL_TRY_OR_CLEANUP(sail_strdup_length(value, length, &string_node->value),
                            /* cleanup */ destroy_string_node(string_node));

        *last_string_node = string_node;
        last_string_node = &string_node->next;

        value += length;
    }

    return SAIL_OK;
}
