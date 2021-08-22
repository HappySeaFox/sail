/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_VECTOR_H
#define SAIL_VECTOR_H

#include <stddef.h> /* size_t */

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_vector;

/*
 * A simple unsafe implementation of a C++ vector with minimum functionality. The vector is designed
 * to store pointers to data, not values.
 */

/*
 * Allocates a new vector. The assigned vector MUST be destroyed later with sail_destroy_vector().
 * An optional item_destroy() callback can be specified to destroy vector items. If it's NULL,
 * the vector items are destroyed with sail_free().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_vector(size_t capacity, void (*item_destroy)(void *item), struct sail_vector **vector);

/*
 * Destroys the specified vector. Does nothing if the vector is NULL.
 * Calls the destructor specified by sail_alloc_vector() on each item. If no destructor was specified,
 * calls sail_free() on each item.
 */
SAIL_EXPORT void sail_destroy_vector(struct sail_vector *vector);

/*
 * Returns the item at the specified index. The index must be in the vector bounds.
 * The vector must not be NULL.
 */
SAIL_EXPORT void* sail_get_vector_item(const struct sail_vector *vector, size_t index);

/*
 * Pushes the item to the end of the vector. The vector must not be NULL.
 * Transfers the ownership of the item to the vector.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_push_vector(struct sail_vector *vector, void *item);

/*
 * Pops the last item from the vector. The vector must not be NULL and empty.
 * Transfers the ownership of the item to the caller.
 */
SAIL_EXPORT void* sail_pop_vector(struct sail_vector *vector);

/*
 * Clears the vector. The vector must not be NULL.
 *
 * Calls the destructor specified by sail_alloc_vector() on each item. If no destructor was specified,
 * calls sail_free() on each item.
 */
SAIL_EXPORT void sail_clear_vector(struct sail_vector *vector);

/*
 * Returns the number of items in the vector. The vector must not be NULL.
 */
SAIL_EXPORT size_t sail_vector_size(const struct sail_vector *vector);

/*
 * Iterates over the vector and calls the specified visitor on each item.
 *
 * Does nothing if:
 *  - the vector is NULL
 *  - the vector is empty
 *  - the visitor is NULL
 */
SAIL_EXPORT void sail_foreach_vector(const struct sail_vector *vector, void (*item_visit)(void *item));

#endif
