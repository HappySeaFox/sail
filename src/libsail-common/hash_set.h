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

#ifndef SAIL_HASH_SET_H
#define SAIL_HASH_SET_H

#include <stdbool.h>

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

struct sail_hash_set;

/*
 * Allocates a new hash set.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_hash_set(struct sail_hash_set **hash_set);

/*
 * Destroys the specified hash set. Does nothing if the hash set is NULL.
 */
SAIL_EXPORT void sail_destroy_hash_set(struct sail_hash_set *hash_set);

/*
 * Puts a new key into the hash set.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_set(struct sail_hash_set *hash_set, const char *key);

/*
 * Returns true if the hash set contains the specified key.
 */
SAIL_EXPORT bool sail_hash_set_has_key(const struct sail_hash_set *hash_set, const char *key);

/*
 * Returns the number of keys stored in the hash set.
 */
SAIL_EXPORT unsigned sail_hash_set_size(const struct sail_hash_set *hash_set);

/*
 * Traverses the hash set in random order and calls the callback function on every key.
 * If the callback returns false, the loop stops at the current element.
 */
SAIL_EXPORT void sail_traverse_hash_set(const struct sail_hash_set *hash_set, bool (*callback)(const char *key));

/*
 * Traverses the hash set in random order and calls the callback function on every key.
 * Additionally passes the specfied user data to the callback.
 * If the callback returns false, the loop stops at the current element.
 */
SAIL_EXPORT void sail_traverse_hash_set_with_user_data(const struct sail_hash_set *hash_set,
                                                       bool (*callback)(const char *key, void *user_data),
                                                       void *user_data);

/*
 * Erases the key from the hash set.
 */
SAIL_EXPORT void sail_erase_hash_set(struct sail_hash_set *hash_set, const char *key);

/*
 * Removes all the keys from the hash set.
 */
SAIL_EXPORT void sail_clear_hash_set(struct sail_hash_set *hash_set);

/*
 * Makes a deep copy of the specified hash set.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_hash_set(const struct sail_hash_set *source_hash_set, struct sail_hash_set **target_hash_set);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
