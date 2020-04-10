#ifndef SAIL_STRING_NODE_H
#define SAIL_STRING_NODE_H

struct sail_string_node {

    char *value;
    struct sail_string_node *next;
};

typedef struct sail_string_node sail_string_node_t;

#endif
