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

    SAIL_CHECK_META_DATA_NODE_PTR(node);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_meta_data_node), &ptr));
    *node = ptr;

    (*node)->key          = SAIL_META_DATA_UNKNOWN;
    (*node)->key_unknown  = NULL;
    (*node)->value_type   = SAIL_META_DATA_TYPE_STRING;
    (*node)->value        = NULL;
    (*node)->value_length = 0;
    (*node)->next         = NULL;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_known_string(enum SailMetaData key, const char *value, struct sail_meta_data_node **node) {

    SAIL_CHECK_STRING_PTR(value);

    SAIL_TRY(sail_alloc_meta_data_node_from_known_substring(key, value, strlen(value), node));

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_unknown_string(const char *key_unknown, const char *value, struct sail_meta_data_node **node) {

    SAIL_CHECK_STRING_PTR(value);

    SAIL_TRY(sail_alloc_meta_data_node_from_unknown_substring(key_unknown, value, strlen(value), node));

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_known_substring(enum SailMetaData key, const char *value, size_t size, struct sail_meta_data_node **node) {

    if (key == SAIL_META_DATA_UNKNOWN) {
        SAIL_LOG_ERROR("%s() accepts only known meta data keys", __func__);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_STRING_PTR(value);
    SAIL_CHECK_META_DATA_NODE_PTR(node);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    node_local->key          = key;
    node_local->value_type   = SAIL_META_DATA_TYPE_STRING;
    node_local->value_length = size + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(node_local->value_length, &node_local->value),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    memcpy(node_local->value, value, node_local->value_length - 1);
    *((char *)node_local->value + node_local->value_length - 1) = '\0';

    *node = node_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_unknown_substring(const char *key_unknown, const char *value, size_t size, struct sail_meta_data_node **node) {

    SAIL_CHECK_STRING_PTR(key_unknown);
    SAIL_CHECK_STRING_PTR(value);
    SAIL_CHECK_META_DATA_NODE_PTR(node);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    SAIL_TRY_OR_CLEANUP(sail_strdup(key_unknown, &node_local->key_unknown),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    node_local->key          = SAIL_META_DATA_UNKNOWN;
    node_local->value_type   = SAIL_META_DATA_TYPE_STRING;
    node_local->value_length = size + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(node_local->value_length, &node_local->value),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    memcpy(node_local->value, value, node_local->value_length - 1);
    *((char *)node_local->value + node_local->value_length - 1) = '\0';

    *node = node_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_known_data(enum SailMetaData key, const void *value, size_t value_length, struct sail_meta_data_node **node) {

    if (key == SAIL_META_DATA_UNKNOWN) {
        SAIL_LOG_ERROR("%s() accepts only known meta data keys", __func__);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_DATA_PTR(value);
    SAIL_CHECK_META_DATA_NODE_PTR(node);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    node_local->key          = key;
    node_local->value_type   = SAIL_META_DATA_TYPE_DATA;
    node_local->value_length = value_length;

    SAIL_TRY_OR_CLEANUP(sail_memdup(value, value_length, &node_local->value),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    *node = node_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_node_from_unknown_data(const char *key_unknown, const void *value, size_t value_length, struct sail_meta_data_node **node) {

    SAIL_CHECK_STRING_PTR(key_unknown);
    SAIL_CHECK_DATA_PTR(value);
    SAIL_CHECK_META_DATA_NODE_PTR(node);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    SAIL_TRY_OR_CLEANUP(sail_strdup(key_unknown, &node_local->key_unknown),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    node_local->key          = SAIL_META_DATA_UNKNOWN;
    node_local->value_type   = SAIL_META_DATA_TYPE_DATA;
    node_local->value_length = value_length;

    SAIL_TRY_OR_CLEANUP(sail_memdup(value, value_length, &node_local->value),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    *node = node_local;

    return SAIL_OK;
}

void sail_destroy_meta_data_node(struct sail_meta_data_node *node) {

    if (node == NULL) {
        return;
    }

    sail_free(node->key_unknown);
    sail_free(node->value);
    sail_free(node);
}

sail_status_t sail_copy_meta_data_node(const struct sail_meta_data_node *source, struct sail_meta_data_node **target) {

    SAIL_CHECK_META_DATA_NODE_PTR(source);
    SAIL_CHECK_META_DATA_NODE_PTR(target);

    struct sail_meta_data_node *node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&node_local));

    node_local->key = source->key;

    if (source->key_unknown != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_strdup(source->key_unknown, &node_local->key_unknown),
                            /* cleanup */ sail_destroy_meta_data_node(node_local));
    }

    node_local->value_type = source->value_type;

    SAIL_TRY_OR_CLEANUP(sail_memdup(source->value, source->value_length, &node_local->value),
                        /* cleanup */ sail_destroy_meta_data_node(node_local));

    node_local->value_length = source->value_length;

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

    SAIL_CHECK_META_DATA_NODE_PTR(target);

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
