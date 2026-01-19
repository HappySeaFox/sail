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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sail-common/sail-common.h>

#include "sail-comparators.h"

#include "munit.h"

static MunitResult test_put(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    const double reference_value1 = 11.5;
    const int reference_value2    = 101;

    /* Value 1. */
    struct sail_variant* value1;
    munit_assert(sail_alloc_variant(&value1) == SAIL_OK);
    sail_set_variant_double(value1, reference_value1);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value1) == SAIL_OK);
    sail_destroy_variant(value1);
    munit_assert(sail_hash_map_has_key(hash_map, "ktop"));

    const struct sail_variant* value_in_map1 = sail_hash_map_value(hash_map, "ktop");
    munit_assert_not_null(value_in_map1);
    munit_assert_double(sail_variant_to_double(value_in_map1), ==, reference_value1);

    munit_assert_size(sail_hash_map_size(hash_map), ==, 1);

    /* Value 2. */
    struct sail_variant* value2;
    munit_assert(sail_alloc_variant(&value2) == SAIL_OK);
    sail_set_variant_int(value2, reference_value2);

    munit_assert(sail_put_hash_map(hash_map, "range", value2) == SAIL_OK);
    sail_destroy_variant(value2);
    munit_assert(sail_hash_map_has_key(hash_map, "range"));

    const struct sail_variant* value_in_map2 = sail_hash_map_value(hash_map, "range");
    munit_assert_not_null(value_in_map2);
    munit_assert_int(sail_variant_to_int(value_in_map2), ==, reference_value2);

    munit_assert_size(sail_hash_map_size(hash_map), ==, 2);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_put_erase_many(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    srand((unsigned)time(NULL));

    /* Construct a large array of keys to force collisions in the hash map. */
    enum
    {
        ARRAY_SIZE = 2500
    };

    char* keys[ARRAY_SIZE] = {NULL};

    for (size_t i = 0; i < ARRAY_SIZE; i++)
    {
        const size_t length = 5 + 1;

        void* ptr;
        munit_assert(sail_malloc(length, &ptr) == SAIL_OK);
        keys[i] = ptr;

        for (size_t l = 0; l < length - 1; l++)
        {
            keys[i][l] = (char)(1 + rand() % 255);
        }

        keys[i][length - 1] = '\0';
    }

    /* Value. */
    const double reference_value = 24.5;

    struct sail_variant* value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value);

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    for (size_t i = 0, prev_size = 0; i < ARRAY_SIZE; i++)
    {
        munit_assert(sail_put_hash_map(hash_map, keys[i], value) == SAIL_OK);
        munit_assert(sail_hash_map_has_key(hash_map, keys[i]));
        munit_assert_size(sail_hash_map_size(hash_map), ==, ++prev_size);
    }

    for (size_t i = 0, prev_size = ARRAY_SIZE; i < ARRAY_SIZE; i++)
    {
        sail_erase_hash_map_key(hash_map, keys[i]);
        munit_assert(!sail_hash_map_has_key(hash_map, keys[i]));
        munit_assert_size(sail_hash_map_size(hash_map), ==, --prev_size);
    }

    /* Cleanup. */
    for (size_t i = 0; i < ARRAY_SIZE; i++)
    {
        sail_free(keys[i]);
    }

    sail_destroy_variant(value);
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_copy(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map1;
    munit_assert(sail_alloc_hash_map(&hash_map1) == SAIL_OK);

    const double reference_value1 = 11.5;
    const int reference_value2    = 101;

    /* Value 1. */
    struct sail_variant* value1;
    munit_assert(sail_alloc_variant(&value1) == SAIL_OK);
    sail_set_variant_double(value1, reference_value1);

    munit_assert(sail_put_hash_map(hash_map1, "ktop", value1) == SAIL_OK);
    sail_destroy_variant(value1);

    /* Value 2. */
    struct sail_variant* value2;
    munit_assert(sail_alloc_variant(&value2) == SAIL_OK);
    sail_set_variant_int(value2, reference_value2);

    munit_assert(sail_put_hash_map(hash_map1, "range", value2) == SAIL_OK);
    sail_destroy_variant(value2);

    struct sail_hash_map* hash_map2;
    munit_assert(sail_copy_hash_map(hash_map1, &hash_map2) == SAIL_OK);

    munit_assert(sail_test_compare_hash_maps(hash_map1, hash_map2) == SAIL_OK);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map2);
    sail_destroy_hash_map(hash_map1);

    return MUNIT_OK;
}

static MunitResult test_overwrite(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    const double reference_value1 = 11.5;
    const double reference_value2 = 125.5;

    /* Value. */
    struct sail_variant* value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value1);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);

    /* Overwrite. */
    sail_set_variant_double(value, reference_value2);
    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    munit_assert_size(sail_hash_map_size(hash_map), ==, 1);

    const struct sail_variant* value_in_map = sail_hash_map_value(hash_map, "ktop");
    munit_assert_double(sail_variant_to_double(value_in_map), ==, reference_value2);

    /* Overwrite #2. */
    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    munit_assert_size(sail_hash_map_size(hash_map), ==, 1);

    value_in_map = sail_hash_map_value(hash_map, "ktop");
    munit_assert_double(sail_variant_to_double(value_in_map), ==, reference_value2);

    /* Cleanup. */
    sail_destroy_variant(value);
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static struct sail_hash_map* generate_specific_hash_map_for_erasing(int value)
{
    struct sail_hash_map* hash_map;
    sail_alloc_hash_map(&hash_map);

    /* Value 1. */
    struct sail_variant* variant;
    sail_alloc_variant(&variant);
    sail_set_variant_int(variant, value);

    sail_put_hash_map(hash_map, "z", variant);
    sail_put_hash_map(hash_map, "i1", variant);
    sail_put_hash_map(hash_map, "h2", variant);

    sail_destroy_variant(variant);

    return hash_map;
}

static MunitResult test_erase(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /*
     * The current hashing algorithm puts "z", "i1", and "h2" keys in the same bucket.
     * Let's test this specific use-case.
     */

    struct sail_hash_map* hash_map;
    int reference_value;
    const struct sail_variant* value_in_map;

    /* Erase non-existing. */
    reference_value = 444;
    hash_map        = generate_specific_hash_map_for_erasing(reference_value);

    sail_erase_hash_map_key(hash_map, "oops");
    munit_assert(sail_hash_map_has_key(hash_map, "z"));
    munit_assert(sail_hash_map_has_key(hash_map, "i1"));
    munit_assert(sail_hash_map_has_key(hash_map, "h2"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 3);

    value_in_map = sail_hash_map_value(hash_map, "z");
    munit_assert_not_null(value_in_map);
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);
    value_in_map = sail_hash_map_value(hash_map, "i1");
    munit_assert_not_null(value_in_map);
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);
    value_in_map = sail_hash_map_value(hash_map, "h2");
    munit_assert_not_null(value_in_map);
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);

    sail_destroy_hash_map(hash_map);

    /* Erase "z". */
    reference_value = 555;
    hash_map        = generate_specific_hash_map_for_erasing(reference_value);

    sail_erase_hash_map_key(hash_map, "z");
    munit_assert(!sail_hash_map_has_key(hash_map, "z"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 2);

    value_in_map = sail_hash_map_value(hash_map, "i1");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);
    value_in_map = sail_hash_map_value(hash_map, "h2");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);

    sail_destroy_hash_map(hash_map);

    /* Erase "i1". */
    reference_value = 666;
    hash_map        = generate_specific_hash_map_for_erasing(reference_value);

    sail_erase_hash_map_key(hash_map, "i1");
    munit_assert(!sail_hash_map_has_key(hash_map, "i1"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 2);

    value_in_map = sail_hash_map_value(hash_map, "z");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);
    value_in_map = sail_hash_map_value(hash_map, "h2");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);

    sail_destroy_hash_map(hash_map);

    /* Erase "h2". */
    reference_value = 777;
    hash_map        = generate_specific_hash_map_for_erasing(reference_value);

    sail_erase_hash_map_key(hash_map, "h2");
    munit_assert(!sail_hash_map_has_key(hash_map, "h2"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 2);

    value_in_map = sail_hash_map_value(hash_map, "z");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);
    value_in_map = sail_hash_map_value(hash_map, "i1");
    munit_assert_int(sail_variant_to_int(value_in_map), ==, reference_value);

    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_clear(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    /* Clear an empty hash map. */
    sail_clear_hash_map(hash_map);

    const double reference_value = 11.5;

    /* Value. */
    struct sail_variant* value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    sail_destroy_variant(value);

    /* Clear. */
    sail_clear_hash_map(hash_map);
    munit_assert_size(sail_hash_map_size(hash_map), ==, 0);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

#define TEST_PUT_HASH_MAP_TYPE(TYPE, VALUE, SETTER, GETTER, KEY)                 \
    do                                                                           \
    {                                                                            \
        TYPE test_value = VALUE;                                                 \
        munit_assert(SETTER(hash_map, KEY, test_value) == SAIL_OK);              \
        munit_assert(sail_hash_map_has_key(hash_map, KEY));                      \
        const struct sail_variant* variant = sail_hash_map_value(hash_map, KEY); \
        munit_assert_not_null(variant);                                          \
        munit_assert(GETTER(variant) == test_value);                             \
    } while (0)

static MunitResult test_put_type_functions(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    TEST_PUT_HASH_MAP_TYPE(bool, true, sail_put_hash_map_bool, sail_variant_to_bool, "test-bool");
    TEST_PUT_HASH_MAP_TYPE(char, 'a', sail_put_hash_map_char, sail_variant_to_char, "test-char");
    TEST_PUT_HASH_MAP_TYPE(unsigned char, 'b', sail_put_hash_map_unsigned_char, sail_variant_to_unsigned_char,
                           "test-unsigned-char");
    TEST_PUT_HASH_MAP_TYPE(short, 2110, sail_put_hash_map_short, sail_variant_to_short, "test-short");
    TEST_PUT_HASH_MAP_TYPE(unsigned short, 2110, sail_put_hash_map_unsigned_short, sail_variant_to_unsigned_short,
                           "test-unsigned-short");
    TEST_PUT_HASH_MAP_TYPE(int, 0xFFFF5, sail_put_hash_map_int, sail_variant_to_int, "test-int");
    TEST_PUT_HASH_MAP_TYPE(unsigned int, 0xFFFF5, sail_put_hash_map_unsigned_int, sail_variant_to_unsigned_int,
                           "test-unsigned-int");
    TEST_PUT_HASH_MAP_TYPE(long, 0xFFFF6, sail_put_hash_map_long, sail_variant_to_long, "test-long");
    TEST_PUT_HASH_MAP_TYPE(unsigned long, 0xFFFF6, sail_put_hash_map_unsigned_long, sail_variant_to_unsigned_long,
                           "test-unsigned-long");
    TEST_PUT_HASH_MAP_TYPE(long long, 0xFFFF7LL, sail_put_hash_map_long_long, sail_variant_to_long_long,
                           "test-long-long");
    TEST_PUT_HASH_MAP_TYPE(unsigned long long, 0xFFFF7ULL, sail_put_hash_map_unsigned_long_long,
                           sail_variant_to_unsigned_long_long, "test-unsigned-long-long");

    /* Test float. */
    {
        float test_value = 160.f;
        munit_assert(sail_put_hash_map_float(hash_map, "test-float", test_value) == SAIL_OK);
        munit_assert(sail_hash_map_has_key(hash_map, "test-float"));
        const struct sail_variant* variant = sail_hash_map_value(hash_map, "test-float");
        munit_assert_not_null(variant);
        munit_assert_float(sail_variant_to_float(variant), ==, test_value);
    }

    /* Test double. */
    {
        double test_value = 29555.0;
        munit_assert(sail_put_hash_map_double(hash_map, "test-double", test_value) == SAIL_OK);
        munit_assert(sail_hash_map_has_key(hash_map, "test-double"));
        const struct sail_variant* variant = sail_hash_map_value(hash_map, "test-double");
        munit_assert_not_null(variant);
        munit_assert_double(sail_variant_to_double(variant), ==, test_value);
    }

    /* Test string. */
    munit_assert(sail_put_hash_map_string(hash_map, "test-string", "abc") == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "test-string"));
    const struct sail_variant* string_variant = sail_hash_map_value(hash_map, "test-string");
    munit_assert_not_null(string_variant);
    munit_assert_string_equal(sail_variant_to_string(string_variant), "abc");

    /* Test data. */
    const char test_data[] = "xyz";
    munit_assert(sail_put_hash_map_data(hash_map, "test-data", test_data, sizeof(test_data)) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "test-data"));
    const struct sail_variant* data_variant = sail_hash_map_value(hash_map, "test-data");
    munit_assert_not_null(data_variant);
    munit_assert_memory_equal(sizeof(test_data), sail_variant_to_data(data_variant), test_data);

    munit_assert_size(sail_hash_map_size(hash_map), ==, 15);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_put_type_functions_null_hash_map(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* All functions should handle NULL hash_map gracefully. */
    munit_assert(sail_put_hash_map_bool(NULL, "key", true) == SAIL_OK);
    munit_assert(sail_put_hash_map_char(NULL, "key", 'a') == SAIL_OK);
    munit_assert(sail_put_hash_map_unsigned_char(NULL, "key", 'b') == SAIL_OK);
    munit_assert(sail_put_hash_map_short(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_unsigned_short(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_int(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_unsigned_int(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_long(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_unsigned_long(NULL, "key", 1) == SAIL_OK);
    munit_assert(sail_put_hash_map_long_long(NULL, "key", 1LL) == SAIL_OK);
    munit_assert(sail_put_hash_map_unsigned_long_long(NULL, "key", 1ULL) == SAIL_OK);
    munit_assert(sail_put_hash_map_float(NULL, "key", 1.0f) == SAIL_OK);
    munit_assert(sail_put_hash_map_double(NULL, "key", 1.0) == SAIL_OK);
    munit_assert(sail_put_hash_map_string(NULL, "key", "value") == SAIL_OK);
    munit_assert(sail_put_hash_map_data(NULL, "key", "value", 6) == SAIL_OK);

    return MUNIT_OK;
}

static MunitResult test_put_hash_map_string_empty(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    /* Empty string should be ignored. */
    munit_assert(sail_put_hash_map_string(hash_map, "key", "") == SAIL_OK);
    munit_assert(!sail_hash_map_has_key(hash_map, "key"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 0);

    /* NULL string should be ignored. */
    munit_assert(sail_put_hash_map_string(hash_map, "key", NULL) == SAIL_OK);
    munit_assert(!sail_hash_map_has_key(hash_map, "key"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 0);

    /* Valid string should be added. */
    munit_assert(sail_put_hash_map_string(hash_map, "key", "value") == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "key"));
    munit_assert_size(sail_hash_map_size(hash_map), ==, 1);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_put_value(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_hash_map* hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    bool bool_value = true;
    munit_assert(sail_put_hash_map_value(hash_map, "bool", bool_value) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "bool"));
    const struct sail_variant* variant = sail_hash_map_value(hash_map, "bool");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_bool(variant) == true);

    munit_assert(sail_put_hash_map_value(hash_map, "char", 'a') == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "char"));
    variant = sail_hash_map_value(hash_map, "char");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_char(variant) == 'a');

    unsigned char uc = 200;
    munit_assert(sail_put_hash_map_value(hash_map, "unsigned-char", uc) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "unsigned-char"));
    variant = sail_hash_map_value(hash_map, "unsigned-char");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_unsigned_char(variant) == uc);

    munit_assert(sail_put_hash_map_value(hash_map, "short", (short)1234) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "short"));
    variant = sail_hash_map_value(hash_map, "short");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_short(variant) == 1234);

    unsigned short us = 5678;
    munit_assert(sail_put_hash_map_value(hash_map, "unsigned-short", us) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "unsigned-short"));
    variant = sail_hash_map_value(hash_map, "unsigned-short");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_unsigned_short(variant) == us);

    munit_assert(sail_put_hash_map_value(hash_map, "int", 42) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "int"));
    variant = sail_hash_map_value(hash_map, "int");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_int(variant) == 42);

    unsigned int ui = 0xFFFFFF9;
    munit_assert(sail_put_hash_map_value(hash_map, "unsigned-int", ui) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "unsigned-int"));
    variant = sail_hash_map_value(hash_map, "unsigned-int");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_unsigned_int(variant) == ui);

    munit_assert(sail_put_hash_map_value(hash_map, "long", 0xFFFF9L) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "long"));
    variant = sail_hash_map_value(hash_map, "long");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_long(variant) == 0xFFFF9L);

    unsigned long ul = 0xFFFFFF9UL;
    munit_assert(sail_put_hash_map_value(hash_map, "unsigned-long", ul) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "unsigned-long"));
    variant = sail_hash_map_value(hash_map, "unsigned-long");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_unsigned_long(variant) == ul);

    munit_assert(sail_put_hash_map_value(hash_map, "long-long", 0xFFFFALL) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "long-long"));
    variant = sail_hash_map_value(hash_map, "long-long");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_long_long(variant) == 0xFFFFALL);

    unsigned long long ull = 0xFFFFFFAULL;
    munit_assert(sail_put_hash_map_value(hash_map, "unsigned-long-long", ull) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "unsigned-long-long"));
    variant = sail_hash_map_value(hash_map, "unsigned-long-long");
    munit_assert_not_null(variant);
    munit_assert(sail_variant_to_unsigned_long_long(variant) == ull);

    munit_assert(sail_put_hash_map_value(hash_map, "float", 3.14f) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "float"));
    variant = sail_hash_map_value(hash_map, "float");
    munit_assert_not_null(variant);
    munit_assert_float(sail_variant_to_float(variant), ==, 3.14f);

    munit_assert(sail_put_hash_map_value(hash_map, "double", 2.71828) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "double"));
    variant = sail_hash_map_value(hash_map, "double");
    munit_assert_not_null(variant);
    munit_assert_double(sail_variant_to_double(variant), ==, 2.71828);

    munit_assert(sail_put_hash_map_value(hash_map, "string", "hello") == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "string"));
    variant = sail_hash_map_value(hash_map, "string");
    munit_assert_not_null(variant);
    munit_assert_string_equal(sail_variant_to_string(variant), "hello");

    char* char_ptr = "world";
    munit_assert(sail_put_hash_map_value(hash_map, "char-ptr", char_ptr) == SAIL_OK);
    munit_assert(sail_hash_map_has_key(hash_map, "char-ptr"));
    variant = sail_hash_map_value(hash_map, "char-ptr");
    munit_assert_not_null(variant);
    munit_assert_string_equal(sail_variant_to_string(variant), "world");

    munit_assert_size(sail_hash_map_size(hash_map), ==, 15);

    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/put",                       test_put,                              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-erase-many",            test_put_erase_many,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",                      test_copy,                             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/overwrite",                 test_overwrite,                        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/erase",                     test_erase,                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/clear",                     test_clear,                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-type-functions",        test_put_type_functions,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-type-functions-null",   test_put_type_functions_null_hash_map, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-hash-map-string-empty", test_put_hash_map_string_empty,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-value",                 test_put_value,                        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/hash-map", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
