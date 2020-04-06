#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "meta_entry_node.h"

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

    if (meta_entry_node->key != NULL) {
        free(meta_entry_node->key);
    }

    if (meta_entry_node->value != NULL) {
        free(meta_entry_node->value);
    }

    free(meta_entry_node);
}

void sail_destroy_meta_entry_node_chain(struct sail_meta_entry_node *meta_entry_node) {

    while (meta_entry_node != NULL) {
        struct sail_meta_entry_node *meta_entry_node_next = meta_entry_node->next;

        sail_destroy_meta_entry_node(meta_entry_node);

        meta_entry_node = meta_entry_node_next;
    }
}
