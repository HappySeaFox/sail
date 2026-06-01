/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include "sail-common.h"

/*
 * Private functions.
 */

/* Adapters for linked_list_node callbacks: void pointers and matching function pointer types. */
static sail_status_t variant_node_alloc_value(void** value)
{
    SAIL_CHECK_PTR(value);

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    *value = variant;

    return SAIL_OK;
}

static void variant_node_dealloc_value(void* value)
{
    sail_destroy_variant(value);
}

static sail_status_t variant_node_copy_value(const void* source_value, void** target_value)
{
    SAIL_CHECK_PTR(target_value);

    struct sail_variant* variant;
    SAIL_TRY(sail_copy_variant(source_value, &variant));

    *target_value = variant;

    return SAIL_OK;
}

/*
 * Public functions.
 */
sail_status_t sail_alloc_variant_node(struct sail_variant_node** node)
{
    SAIL_TRY(sail_private_alloc_linked_list_node((struct linked_list_node**)node));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_node_and_value(struct sail_variant_node** node)
{
    SAIL_TRY(sail_private_alloc_linked_list_node_and_value(variant_node_alloc_value, variant_node_dealloc_value,
                                                           (struct linked_list_node**)node));

    return SAIL_OK;
}

void sail_destroy_variant_node(struct sail_variant_node* node)
{
    sail_private_destroy_linked_list_node((struct linked_list_node*)node, variant_node_dealloc_value);
}

sail_status_t sail_copy_variant_node(const struct sail_variant_node* source, struct sail_variant_node** target)
{
    SAIL_TRY(sail_private_copy_linked_list_node((const struct linked_list_node*)source,
                                                (struct linked_list_node**)target, variant_node_copy_value,
                                                variant_node_dealloc_value));

    return SAIL_OK;
}

void sail_destroy_variant_node_chain(struct sail_variant_node* node)
{
    sail_private_destroy_linked_list_node_chain((struct linked_list_node*)node, variant_node_dealloc_value);
}

sail_status_t sail_copy_variant_node_chain(const struct sail_variant_node* source, struct sail_variant_node** target)
{
    SAIL_TRY(sail_private_copy_linked_list_node_chain((const struct linked_list_node*)source,
                                                      (struct linked_list_node**)target, variant_node_copy_value,
                                                      variant_node_dealloc_value));

    return SAIL_OK;
}
