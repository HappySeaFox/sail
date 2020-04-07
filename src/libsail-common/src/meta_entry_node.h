#ifndef SAIL_META_ENTRY_NODE_H
#define SAIL_META_ENTRY_NODE_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A simple key-pair structure representing meta information like a JPEG comment.
 */
struct sail_meta_entry_node {

    struct sail_meta_entry_node *next;

    char *key;
    char *value;
};

/*
 * Allocates a new meta entry node. The assigned node MUST be destroyed later with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_meta_entry_node(struct sail_meta_entry_node **meta_entry_node);

/*
 * Destroys the specified meta entry node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_meta_entry_node(struct sail_meta_entry_node *meta_entry_node);

/*
 * Destroys the specified meta entry node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_meta_entry_node_chain(struct sail_meta_entry_node *meta_entry_node);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
