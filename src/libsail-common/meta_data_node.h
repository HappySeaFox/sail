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

#ifndef SAIL_META_DATA_NODE_H
#define SAIL_META_DATA_NODE_H

#include <stddef.h>

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
 * Represents a meta data node.
 *
 */
struct sail_meta_data_node {

    /*
     * Meta data value.
     */
    struct sail_meta_data *meta_data;

    /*
     * Pointer to the next node or NULL.
     */
    struct sail_meta_data_node *next;
};

/*
 * Allocates a new meta data node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_data_node(struct sail_meta_data_node **node);

/*
 * Allocates a new meta data node and the nested value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_data_node_and_value(struct sail_meta_data_node **node);

/*
 * Destroys the specified meta data node.
 */
SAIL_EXPORT void sail_destroy_meta_data_node(struct sail_meta_data_node *node);

/*
 * Makes a deep copy of the specified meta data node. The assigned node MUST be destroyed
 * later with sail_destroy_meta_data_node().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_meta_data_node(const struct sail_meta_data_node *source,
                                                   struct sail_meta_data_node **target);

/*
 * Destroys the specified meta data node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_meta_data_node_chain(struct sail_meta_data_node *node);

/*
 * Makes a deep copy of the specified meta data node chain. The assigned chain MUST be destroyed
 * later with sail_destroy_meta_data_node_chain(). If the source chain is NULL, it assigns NULL
 * to the target chain and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_meta_data_node_chain(const struct sail_meta_data_node *source,
                                                         struct sail_meta_data_node **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
