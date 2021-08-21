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

#include "config.h"

#include <string.h>

#include "sail.h"

/*
 * Private functions.
 */

struct sail_vector {

    void (*item_destroy)(void *item);

    size_t capacity;

    size_t size;

    void **data;
};

static sail_status_t grow_vector_to(struct sail_vector *vector, size_t capacity) {

    const size_t old_capacity = vector->capacity;

    if (SAIL_UNLIKELY(old_capacity == capacity)) {
        return SAIL_OK;
    }

    if (SAIL_UNLIKELY(capacity < old_capacity)) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    void *ptr = vector->data;
    SAIL_TRY(sail_realloc(sizeof(void *) * capacity, &ptr));

    vector->data     = ptr;
    vector->capacity = capacity;

    return SAIL_OK;
}

static sail_status_t grow_vector(struct sail_vector *vector) {

    const size_t capacity = (vector->capacity <= 2) ? (vector->capacity + 1) : (size_t)(vector->capacity * 1.5 + 0.5);

    SAIL_TRY(grow_vector_to(vector, capacity));

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_alloc_vector(size_t capacity, void (*item_destroy)(void *item), struct sail_vector **vector) {

    SAIL_CHECK_VECTOR_PTR(vector);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_vector), &ptr));
    struct sail_vector *local_vector = ptr;

    *local_vector = (struct sail_vector){
        .item_destroy = item_destroy,
        .capacity     = 0,
        .size         = 0,
        .data         = NULL,
    };

    SAIL_TRY_OR_CLEANUP(grow_vector_to(local_vector, capacity),
                        /* cleanup */ sail_destroy_vector(local_vector));

    *vector = local_vector;

    return SAIL_OK;
}

void sail_destroy_vector(struct sail_vector *vector) {

    if (vector == NULL) {
        return;
    }

    sail_clear_vector(vector);
    sail_free(vector);
}

void* sail_get_vector_item(const struct sail_vector *vector, size_t index) {

    return vector->data[index];
}

sail_status_t sail_push_vector(struct sail_vector *vector, void *item) {

    if (vector->size == vector->capacity) {
        SAIL_TRY(grow_vector(vector));
    }

    vector->data[vector->size++] = item;

    return SAIL_OK;
}

void* sail_pop_vector(struct sail_vector *vector) {

    return vector->data[--(vector->size)];
}

void sail_clear_vector(struct sail_vector *vector) {

    if (vector->item_destroy != NULL) {
        sail_foreach_vector(vector, vector->item_destroy);
    }

    vector->size     = 0;
    vector->capacity = 0;

    sail_free(vector->data);
    vector->data = NULL;
}

size_t sail_vector_size(struct sail_vector *vector) {

    return vector->size;
}

void sail_foreach_vector(const struct sail_vector *vector, void (*item_visit)(void *item)) {

    if (vector == NULL || vector->size == 0U || item_visit == NULL) {
        return;
    }

    for (size_t i = 0; i < vector->size; i++) {
        item_visit(vector->data[i]);
    }
}
