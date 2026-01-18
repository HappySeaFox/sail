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

#include <string.h>

#include "sail-common.h"

/*
 * Private functions.
 */
static inline uint64_t calculate_hash(const char* key)
{
    return sail_string_hash(key) % SAIL_HASH_MAP_SIZE;
}

/*
 * Public functions.
 */
sail_status_t sail_alloc_hash_map(struct sail_hash_map** hash_map)
{
    SAIL_CHECK_PTR(hash_map);

    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_hash_map), &ptr));
    *hash_map = ptr;

    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        (*hash_map)->buckets[i] = NULL;
    }

    return SAIL_OK;
}

void sail_destroy_hash_map(struct sail_hash_map* hash_map)
{
    if (hash_map == NULL)
    {
        return;
    }

    sail_clear_hash_map(hash_map);

    sail_free(hash_map);
}

sail_status_t sail_put_hash_map(struct sail_hash_map* hash_map, const char* key, const struct sail_variant* value)
{
    SAIL_CHECK_PTR(hash_map);
    SAIL_CHECK_PTR(key);
    SAIL_CHECK_PTR(value);

    struct sail_variant_node** key_variant_node;

    for (key_variant_node = &hash_map->buckets[calculate_hash(key)]; *key_variant_node != NULL;
         key_variant_node = &(*key_variant_node)->next->next)
    {
        struct sail_variant_node* value_variant_node = (*key_variant_node)->next;

        if (strcmp(sail_variant_to_string((*key_variant_node)->variant), key) == 0)
        {
            if (!sail_equal_variants(value_variant_node->variant, value))
            {
                /* Overwrite value. */
                sail_destroy_variant(value_variant_node->variant);
                SAIL_TRY(sail_copy_variant(value, &value_variant_node->variant));
            }

            return SAIL_OK;
        }
    }

    struct sail_variant_node* local_variant_node;

    SAIL_TRY(sail_alloc_variant_node_and_value(&local_variant_node));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(local_variant_node->variant, key),
                        /* cleanup */ sail_destroy_variant_node_chain(local_variant_node));

    SAIL_TRY_OR_CLEANUP(sail_alloc_variant_node(&local_variant_node->next),
                        /* cleanup */ sail_destroy_variant_node_chain(local_variant_node));
    SAIL_TRY_OR_CLEANUP(sail_copy_variant(value, &local_variant_node->next->variant),
                        /* cleanup */ sail_destroy_variant_node_chain(local_variant_node));

    *key_variant_node = local_variant_node;

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_shallow(struct sail_hash_map* hash_map, const char* key, struct sail_variant* value)
{
    SAIL_CHECK_PTR(hash_map);
    SAIL_CHECK_PTR(key);
    SAIL_CHECK_PTR(value);

    struct sail_variant_node** key_variant_node;

    for (key_variant_node = &hash_map->buckets[calculate_hash(key)]; *key_variant_node != NULL;
         key_variant_node = &(*key_variant_node)->next->next)
    {
        struct sail_variant_node* value_variant_node = (*key_variant_node)->next;

        if (strcmp(sail_variant_to_string((*key_variant_node)->variant), key) == 0)
        {
            if (!sail_equal_variants(value_variant_node->variant, value))
            {
                /* Overwrite value. */
                sail_destroy_variant(value_variant_node->variant);
                value_variant_node->variant = value;
            }
            else
            {
                /* Same value, destroy the passed variant. */
                sail_destroy_variant(value);
            }

            return SAIL_OK;
        }
    }

    struct sail_variant_node* local_variant_node;

    SAIL_TRY(sail_alloc_variant_node_and_value(&local_variant_node));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(local_variant_node->variant, key),
                        /* cleanup */ sail_destroy_variant_node_chain(local_variant_node));

    SAIL_TRY_OR_CLEANUP(sail_alloc_variant_node(&local_variant_node->next),
                        /* cleanup */ sail_destroy_variant_node_chain(local_variant_node));
    local_variant_node->next->variant = value;

    *key_variant_node = local_variant_node;

    return SAIL_OK;
}

bool sail_hash_map_has_key(const struct sail_hash_map* hash_map, const char* key)
{
    if (key == NULL)
    {
        return false;
    }

    for (const struct sail_variant_node* key_variant_node = hash_map->buckets[calculate_hash(key)];
         key_variant_node != NULL; key_variant_node       = key_variant_node->next->next)
    {
        if (strcmp(sail_variant_to_string(key_variant_node->variant), key) == 0)
        {
            return true;
        }
    }

    return false;
}

struct sail_variant* sail_hash_map_value(const struct sail_hash_map* hash_map, const char* key)
{
    if (key == NULL)
    {
        return NULL;
    }

    for (struct sail_variant_node* key_variant_node = hash_map->buckets[calculate_hash(key)]; key_variant_node != NULL;
         key_variant_node                           = key_variant_node->next->next)
    {
        if (strcmp(sail_variant_to_string(key_variant_node->variant), key) == 0)
        {
            return key_variant_node->next->variant;
        }
    }

    return NULL;
}

unsigned sail_hash_map_size(const struct sail_hash_map* hash_map)
{
    unsigned size = 0;

    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        for (struct sail_variant_node* key_variant_node = hash_map->buckets[i]; key_variant_node != NULL;
             key_variant_node                           = key_variant_node->next->next)
        {
            size++;
        }
    }

    return size;
}

void sail_traverse_hash_map(const struct sail_hash_map* hash_map,
                            bool (*callback)(const char* key, const struct sail_variant* value))
{
    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        for (const struct sail_variant_node* key_variant_node = hash_map->buckets[i];
             key_variant_node != NULL
             && callback(sail_variant_to_string(key_variant_node->variant), key_variant_node->next->variant);
             key_variant_node = key_variant_node->next->next)
        {
        }
    }
}

void sail_traverse_hash_map_with_user_data(const struct sail_hash_map* hash_map,
                                           bool (*callback)(const char* key,
                                                            const struct sail_variant* value,
                                                            void* user_data),
                                           void* user_data)
{
    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        for (const struct sail_variant_node* key_variant_node = hash_map->buckets[i];
             key_variant_node != NULL
             && callback(sail_variant_to_string(key_variant_node->variant), key_variant_node->next->variant, user_data);
             key_variant_node = key_variant_node->next->next)
        {
        }
    }
}

void sail_erase_hash_map_key(struct sail_hash_map* hash_map, const char* key)
{
    if (key == NULL)
    {
        return;
    }

    for (struct sail_variant_node **head_variant_node = &hash_map->buckets[calculate_hash(key)],
                                  *key_variant_node = *head_variant_node, *prev_key_variant_node = NULL;
         key_variant_node != NULL;
         prev_key_variant_node = key_variant_node, key_variant_node = key_variant_node->next->next)
    {
        if (strcmp(sail_variant_to_string(key_variant_node->variant), key) == 0)
        {
            struct sail_variant_node* next_key_variant_node = key_variant_node->next->next;

            sail_destroy_variant_node(key_variant_node->next);
            sail_destroy_variant_node(key_variant_node);

            /* Erase from the head. */
            if (key_variant_node == *head_variant_node)
            {
                *head_variant_node = next_key_variant_node;
            }
            else
            {
                /* Erase from the middle/end. */
                prev_key_variant_node->next->next = next_key_variant_node;
            }

            return;
        }
    }
}

void sail_clear_hash_map(struct sail_hash_map* hash_map)
{
    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        sail_destroy_variant_node_chain(hash_map->buckets[i]);
        hash_map->buckets[i] = NULL;
    }
}

sail_status_t sail_copy_hash_map(const struct sail_hash_map* source_hash_map, struct sail_hash_map** target_hash_map)
{
    SAIL_CHECK_PTR(source_hash_map);
    SAIL_CHECK_PTR(target_hash_map);

    struct sail_hash_map* hash_map_local;
    SAIL_TRY(sail_alloc_hash_map(&hash_map_local));

    for (size_t i = 0; i < SAIL_HASH_MAP_SIZE; i++)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_variant_node_chain(source_hash_map->buckets[i], &hash_map_local->buckets[i]),
                            /* cleanup */ sail_destroy_hash_map(hash_map_local));
    }

    *target_hash_map = hash_map_local;

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_bool(struct sail_hash_map* hash_map, const char* key, bool value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_bool(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_char(struct sail_hash_map* hash_map, const char* key, char value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_char(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_unsigned_char(struct sail_hash_map* hash_map, const char* key, unsigned char value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_unsigned_char(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_short(struct sail_hash_map* hash_map, const char* key, short value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_short(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_unsigned_short(struct sail_hash_map* hash_map, const char* key, unsigned short value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_unsigned_short(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_int(struct sail_hash_map* hash_map, const char* key, int value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_int(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_unsigned_int(struct sail_hash_map* hash_map, const char* key, unsigned int value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_unsigned_int(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_long(struct sail_hash_map* hash_map, const char* key, long value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_long(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_unsigned_long(struct sail_hash_map* hash_map, const char* key, unsigned long value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_unsigned_long(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_long_long(struct sail_hash_map* hash_map, const char* key, long long value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_long_long(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_unsigned_long_long(struct sail_hash_map* hash_map, const char* key, unsigned long long value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_unsigned_long_long(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_float(struct sail_hash_map* hash_map, const char* key, float value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_float(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_double(struct sail_hash_map* hash_map, const char* key, double value)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_double(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_string(struct sail_hash_map* hash_map, const char* key, const char* value)
{
    if (hash_map == NULL || value == NULL || value[0] == '\0')
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(variant, value),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}

sail_status_t sail_put_hash_map_data(struct sail_hash_map* hash_map, const char* key, const void* value, size_t size)
{
    if (hash_map == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_data(variant, value, size),
                        /* cleanup */ sail_destroy_variant(variant));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map_shallow(hash_map, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    return SAIL_OK;
}
