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

#ifndef SAIL_STRING_NODE_H
#define SAIL_STRING_NODE_H

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
 * Represents a string node.
 */
struct sail_string_node {

    /*
     * String value.
     */
    char *string;

    /*
     * Pointer to the next node or NULL.
     */
    struct sail_string_node *next;
};

/*
 * Allocates a new string node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_string_node(struct sail_string_node **node);

/*
 * Destroys the specified string node.
 */
SAIL_EXPORT void sail_destroy_string_node(struct sail_string_node *node);

/*
 * Makes a deep copy of the specified string node.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_string_node(const struct sail_string_node *source, struct sail_string_node **target);

/*
 * Destroys the specified string node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_string_node_chain(struct sail_string_node *node);

/*
 * Makes a deep copy of the specified string node chain. If the source chain is NULL, it assigns NULL
 * to the target chain and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_string_node_chain(const struct sail_string_node *source, struct sail_string_node **target);

/*
 * Split a ';'-separated list of strings.
 */
SAIL_EXPORT sail_status_t sail_split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
