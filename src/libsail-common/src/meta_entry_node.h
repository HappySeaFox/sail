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

#ifndef SAIL_META_ENTRY_NODE_H
#define SAIL_META_ENTRY_NODE_H

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A simple key-pair structure representing meta information like a JPEG comment.
 *
 * For example:
 *
 * {
 *     key         = SAIL_META_INFO_UNKNOWN,
 *     key_unknown = "My Data",
 *     value       = "Data"
 * }
 *
 * {
 *     key         = SAIL_META_INFO_COMMENT,
 *     key_unknown = NULL,
 *     value       = "Holidays"
 * }
 */
struct sail_meta_entry_node {

    /*
     * If key is SAIL_META_INFO_UNKNOWN, key_unknown contains an actual string key.
     * If key is not SAIL_META_INFO_UNKNOWN, key_unknown is NULL.
     */
    enum SailMetaInfo key;
    char *key_unknown;

    /* Actual meta info value. Any string data. */
    char *value;

    struct sail_meta_entry_node *next;
};

/*
 * Allocates a new meta entry node. The assigned node MUST be destroyed later with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_entry_node(struct sail_meta_entry_node **node);

/*
 * Allocates a new meta entry node from the specified data. The assigned node MUST be destroyed later
 * with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_entry_node_from_data(enum SailMetaInfo key,
                                                                const char *key_unknown,
                                                                const char *value,
                                                                struct sail_meta_entry_node **node);

/*
 * Destroys the specified meta entry node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_meta_entry_node(struct sail_meta_entry_node *node);

/*
 * Makes a deep copy of the specified meta entry node. The assigned node MUST be destroyed
 * later with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_copy_meta_entry_node(struct sail_meta_entry_node *source,
                                                   struct sail_meta_entry_node **target);

/*
 * Destroys the specified meta entry node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_meta_entry_node_chain(struct sail_meta_entry_node *node);

/*
 * Makes a deep copy of the specified meta entry node chain. The assigned chain MUST be destroyed
 * later with sail_destroy_meta_entry_node_chain(). If the source chain is NULL, it assigns NULL
 * to the target chain and returns 0.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_copy_meta_entry_node_chain(struct sail_meta_entry_node *source,
                                                         struct sail_meta_entry_node **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
