/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#ifndef SAIL_VARIANT_NODE_H
#define SAIL_VARIANT_NODE_H

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

struct sail_variant;

/*
 * Represents a variant node.
 */
struct sail_variant_node {

    /*
     * Variant value.
     */
    struct sail_variant *variant;

    /*
     * Pointer to the next node or NULL.
     */
    struct sail_variant_node *next;
};

/*
 * Allocates a new variant node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_variant_node(struct sail_variant_node **node);

/*
 * Allocates a new variant node and the nested value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_variant_node_and_value(struct sail_variant_node **node);

/*
 * Destroys the specified variant node.
 */
SAIL_EXPORT void sail_destroy_variant_node(struct sail_variant_node *node);

/*
 * Makes a deep copy of the specified variant node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_variant_node(const struct sail_variant_node *source,
                                                 struct sail_variant_node **target);

/*
 * Destroys the specified variant node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_variant_node_chain(struct sail_variant_node *node);

/*
 * Makes a deep copy of the specified variant node chain. If the source chain is NULL, it assigns NULL
 * to the target chain and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_variant_node_chain(const struct sail_variant_node *source,
                                                       struct sail_variant_node **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
