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

#include <string.h>

#include "sail-common.h"

/*
 * Private functions.
 */
struct hash_map_traverse_callback_holder {

    bool (*hash_set_traverse_callback)(const char *key);
};

struct hash_map_traverse_callback_with_user_data_holder {

    bool (*hash_set_traverse_callback)(const char *key, void *user_data);

    void *user_data;
};

static bool hash_map_traverse_callback(const char *key, const struct sail_variant *value, void *user_data) {

    (void)value;

    const struct hash_map_traverse_callback_holder *hash_map_traverse_callback_holder = user_data;

    return hash_map_traverse_callback_holder->hash_set_traverse_callback(key);
}

static bool hash_map_traverse_callback_with_user_data(const char *key, const struct sail_variant *value, void *user_data) {

    (void)value;

    const struct hash_map_traverse_callback_with_user_data_holder *hash_map_traverse_callback_with_user_data_holder = user_data;

    return hash_map_traverse_callback_with_user_data_holder->hash_set_traverse_callback(key, hash_map_traverse_callback_with_user_data_holder->user_data);
}

/*
 * Public functions.
 */
sail_status_t sail_alloc_hash_set(struct sail_hash_set **hash_set) {

    SAIL_CHECK_PTR(hash_set);

    struct sail_hash_set *hash_set_local;

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_hash_set), &ptr));
    hash_set_local = ptr;

    hash_set_local->hash_map = NULL;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&hash_set_local->hash_map),
                        /* cleanup */ sail_destroy_hash_set(hash_set_local));

    *hash_set = hash_set_local;

    return SAIL_OK;
}

void sail_destroy_hash_set(struct sail_hash_set *hash_set) {

    if (hash_set == NULL) {
        return;
    }

    sail_destroy_hash_map(hash_set->hash_map);

    sail_free(hash_set);
}

sail_status_t sail_put_hash_set(struct sail_hash_set *hash_set, const char *key) {

    SAIL_CHECK_PTR(hash_set);
    SAIL_CHECK_PTR(key);

    struct sail_variant *value;
    SAIL_TRY(sail_alloc_variant(&value));
    sail_set_variant_bool(value, true);

    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(hash_set->hash_map, key, value),
                        /* cleanup */ sail_destroy_variant(value));

    sail_destroy_variant(value);

    return SAIL_OK;
}

bool sail_hash_set_has_key(const struct sail_hash_set *hash_set, const char *key) {

    return sail_hash_map_has_key(hash_set->hash_map, key);
}

unsigned sail_hash_set_size(const struct sail_hash_set *hash_set) {

    return sail_hash_map_size(hash_set->hash_map);
}

void sail_traverse_hash_set(const struct sail_hash_set *hash_set, bool (*callback)(const char *key)){

    struct hash_map_traverse_callback_holder hash_map_traverse_callback_holder = { callback };

    sail_traverse_hash_map_with_user_data(hash_set->hash_map, hash_map_traverse_callback, &hash_map_traverse_callback_holder);
}

void sail_traverse_hash_set_with_user_data(const struct sail_hash_set *hash_set,
                                           bool (*callback)(const char *key, void *user_data),
                                           void *user_data) {

    struct hash_map_traverse_callback_with_user_data_holder hash_map_traverse_callback_with_user_data_holder = { callback, user_data };

    sail_traverse_hash_map_with_user_data(hash_set->hash_map, hash_map_traverse_callback_with_user_data, &hash_map_traverse_callback_with_user_data_holder);
}

void sail_erase_hash_set(struct sail_hash_set *hash_set, const char *key) {

    sail_erase_hash_map_key(hash_set->hash_map, key);
}

void sail_clear_hash_set(struct sail_hash_set *hash_set) {

    sail_clear_hash_map(hash_set->hash_map);
}

sail_status_t sail_copy_hash_set(const struct sail_hash_set *source_hash_set, struct sail_hash_set **target_hash_set) {

    SAIL_CHECK_PTR(source_hash_set);
    SAIL_CHECK_PTR(target_hash_set);

    struct sail_hash_set *hash_set_local;
    SAIL_TRY(sail_alloc_hash_set(&hash_set_local));

    SAIL_TRY_OR_CLEANUP(sail_copy_hash_map(source_hash_set->hash_map, &hash_set_local->hash_map),
                        /* cleanup */ sail_destroy_hash_set(hash_set_local));

    *target_hash_set = hash_set_local;

    return SAIL_OK;
}
