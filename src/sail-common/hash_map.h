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

#pragma once

#include <stdbool.h>
#include <stddef.h> /* size_t */

#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_hash_map;
struct sail_variant;

/*
 * Allocates a new hash map.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_hash_map(struct sail_hash_map** hash_map);

/*
 * Destroys the specified hash map. Does nothing if the hash map is NULL.
 */
SAIL_EXPORT void sail_destroy_hash_map(struct sail_hash_map* hash_map);

/*
 * Puts a new key-value pair into the hash map. The value gets deep copied.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map(struct sail_hash_map* hash_map,
                                            const char* key,
                                            const struct sail_variant* value);

/*
 * Puts a new key-value pair into the hash map. Takes ownership of the variant.
 * The variant will be destroyed when the hash map is destroyed or when the key is erased.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_shallow(struct sail_hash_map* hash_map,
                                                     const char* key,
                                                     struct sail_variant* value);

/*
 * Returns true if the hash map contains the specified key.
 */
SAIL_EXPORT bool sail_hash_map_has_key(const struct sail_hash_map* hash_map, const char* key);

/*
 * Returns the key associated value or NULL.
 */
SAIL_EXPORT struct sail_variant* sail_hash_map_value(const struct sail_hash_map* hash_map, const char* key);

/*
 * Returns the number of keys stored in the hash map.
 */
SAIL_EXPORT unsigned sail_hash_map_size(const struct sail_hash_map* hash_map);

/*
 * Traverses the hash map in random order and calls the callback function on every key-value pair.
 * If the callback returns false, the loop stops at the current element.
 */
SAIL_EXPORT void sail_traverse_hash_map(const struct sail_hash_map* hash_map,
                                        bool (*callback)(const char* key, const struct sail_variant* value));

/*
 * Traverses the hash map in random order and calls the callback function on every key-value pair.
 * Additionally passes the specified user data to the callback.
 * If the callback returns false, the loop stops at the current element.
 */
SAIL_EXPORT void sail_traverse_hash_map_with_user_data(const struct sail_hash_map* hash_map,
                                                       bool (*callback)(const char* key,
                                                                        const struct sail_variant* value,
                                                                        void* user_data),
                                                       void* user_data);

/*
 * Erases the key-value pair from the hash map.
 */
SAIL_EXPORT void sail_erase_hash_map_key(struct sail_hash_map* hash_map, const char* key);

/*
 * Removes all the key-value pairs from the hash map.
 */
SAIL_EXPORT void sail_clear_hash_map(struct sail_hash_map* hash_map);

/*
 * Makes a deep copy of the specified hash map.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_hash_map(const struct sail_hash_map* source_hash_map,
                                             struct sail_hash_map** target_hash_map);

/*
 * Creates a variant with the specified boolean value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_bool(struct sail_hash_map* hash_map, const char* key, bool value);

/*
 * Creates a variant with the specified char value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_char(struct sail_hash_map* hash_map, const char* key, char value);

/*
 * Creates a variant with the specified unsigned char value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_unsigned_char(struct sail_hash_map* hash_map,
                                                          const char* key,
                                                          unsigned char value);

/*
 * Creates a variant with the specified short value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_short(struct sail_hash_map* hash_map, const char* key, short value);

/*
 * Creates a variant with the specified unsigned short value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_unsigned_short(struct sail_hash_map* hash_map,
                                                           const char* key,
                                                           unsigned short value);

/*
 * Creates a variant with the specified int value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_int(struct sail_hash_map* hash_map, const char* key, int value);

/*
 * Creates a variant with the specified unsigned int value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_unsigned_int(struct sail_hash_map* hash_map,
                                                         const char* key,
                                                         unsigned int value);

/*
 * Creates a variant with the specified long value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_long(struct sail_hash_map* hash_map, const char* key, long value);

/*
 * Creates a variant with the specified unsigned long value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_unsigned_long(struct sail_hash_map* hash_map,
                                                          const char* key,
                                                          unsigned long value);

/*
 * Creates a variant with the specified long long value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_long_long(struct sail_hash_map* hash_map, const char* key, long long value);

/*
 * Creates a variant with the specified unsigned long long value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_unsigned_long_long(struct sail_hash_map* hash_map,
                                                               const char* key,
                                                               unsigned long long value);

/*
 * Creates a variant with the specified float value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_float(struct sail_hash_map* hash_map, const char* key, float value);

/*
 * Creates a variant with the specified double value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_double(struct sail_hash_map* hash_map, const char* key, double value);

/*
 * Creates a variant with the specified string value and puts it into the hash map.
 * If hash_map is NULL, or value is NULL or empty, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_string(struct sail_hash_map* hash_map, const char* key, const char* value);

/*
 * Creates a variant with the specified data value and puts it into the hash map.
 * If hash_map is NULL, does nothing and returns SAIL_OK.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_put_hash_map_data(struct sail_hash_map* hash_map,
                                                 const char* key,
                                                 const void* value,
                                                 size_t size);

/*
 * Puts a key-value pair into the hash map using _Generic macro to automatically select
 * the appropriate function based on the value type.
 *
 * Example:
 *   sail_put_hash_map_value(hash_map, "count", 42);      / int
 *   sail_put_hash_map_value(hash_map, "pi", 3.14);       // double
 *   sail_put_hash_map_value(hash_map, "name", "hello");  // const char*
 *
 * Returns SAIL_OK on success.
 */
#define sail_put_hash_map_value(hash_map, key, value) \
    _Generic((value), \
        bool: sail_put_hash_map_bool, \
        char: sail_put_hash_map_char, \
        unsigned char: sail_put_hash_map_unsigned_char, \
        short: sail_put_hash_map_short, \
        unsigned short: sail_put_hash_map_unsigned_short, \
        int: sail_put_hash_map_int, \
        unsigned int: sail_put_hash_map_unsigned_int, \
        long: sail_put_hash_map_long, \
        unsigned long: sail_put_hash_map_unsigned_long, \
        long long: sail_put_hash_map_long_long, \
        unsigned long long: sail_put_hash_map_unsigned_long_long, \
        float: sail_put_hash_map_float, \
        double: sail_put_hash_map_double, \
        const char*: sail_put_hash_map_string, \
        char*: sail_put_hash_map_string \
    )(hash_map, key, value)

/* extern "C" */
#ifdef __cplusplus
}
#endif
