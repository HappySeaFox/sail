/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_META_ENTRY_NODE_H
#define SAIL_META_ENTRY_NODE_H

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
 * A simple key-pair structure representing meta information like a JPEG comment.
 */
struct sail_meta_entry_node {

    char *key;
    char *value;

    struct sail_meta_entry_node *next;
};

/*
 * Allocates a new meta entry node. The assigned node MUST be destroyed later with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_meta_entry_node(struct sail_meta_entry_node **node);

/*
 * Destroys the specified meta entry node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_meta_entry_node(struct sail_meta_entry_node *node);

/*
 * Makes a deep copy of the specified meta entry node. The assigned node MUST be destroyed
 * later with sail_destroy_meta_entry_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_meta_entry_node(struct sail_meta_entry_node *source,
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
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_meta_entry_node_chain(struct sail_meta_entry_node *source,
                                                         struct sail_meta_entry_node **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
