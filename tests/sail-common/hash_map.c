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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sail-common.h"

#include "sail-comparators.h"

#include "munit.h"

static MunitResult test_put(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_map *hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    const double reference_value1 = 11.5;
    const int reference_value2 = 101;

    /* Value 1. */
    struct sail_variant *value1;
    munit_assert(sail_alloc_variant(&value1) == SAIL_OK);
    sail_set_variant_double(value1, reference_value1);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value1) == SAIL_OK);
    sail_destroy_variant(value1);
    munit_assert(sail_hash_map_has_key(hash_map, "ktop"));

    const struct sail_variant *value_in_map1 = sail_hash_map_value(hash_map, "ktop");
    munit_assert_not_null(value_in_map1);
    munit_assert_double(sail_variant_to_double(value_in_map1), ==, reference_value1);

    munit_assert(sail_hash_map_size(hash_map) == 1);

    /* Value 2. */
    struct sail_variant *value2;
    munit_assert(sail_alloc_variant(&value2) == SAIL_OK);
    sail_set_variant_int(value2, reference_value2);

    munit_assert(sail_put_hash_map(hash_map, "range", value2) == SAIL_OK);
    sail_destroy_variant(value2);
    munit_assert(sail_hash_map_has_key(hash_map, "range"));

    const struct sail_variant *value_in_map2 = sail_hash_map_value(hash_map, "range");
    munit_assert_not_null(value_in_map2);
    munit_assert_int(sail_variant_to_int(value_in_map2), ==, reference_value2);

    munit_assert(sail_hash_map_size(hash_map) == 2);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_put_erase_many(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    srand((unsigned)time(NULL));

    /* Construct a large array of keys to force collisions in the hash map. */
    enum {
        ARRAY_SIZE = 2500
    };

    char* keys[ARRAY_SIZE] = { NULL };

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        const size_t length = 5 + 1;

        void *ptr;
        munit_assert(sail_malloc(length, &ptr) == SAIL_OK);
        keys[i] = ptr;

        for (size_t l = 0; l < length - 1; l++) {
            keys[i][l] = (char)(1 + rand() % 255);
        }

        keys[i][length - 1] = '\0';
    }

    /* Value. */
    const double reference_value = 24.5;

    struct sail_variant *value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value);

    struct sail_hash_map *hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    for (size_t i = 0, prev_size = 0; i < ARRAY_SIZE; i++) {
        munit_assert(sail_put_hash_map(hash_map, keys[i], value) == SAIL_OK);
        munit_assert(sail_hash_map_has_key(hash_map, keys[i]));
        munit_assert(sail_hash_map_size(hash_map) == ++prev_size);
    }

    for (size_t i = 0, prev_size = ARRAY_SIZE; i < ARRAY_SIZE; i++) {
        sail_erase_hash_map_key(hash_map, keys[i]);
        munit_assert(!sail_hash_map_has_key(hash_map, keys[i]));
        munit_assert(sail_hash_map_size(hash_map) == --prev_size);
    }

    /* Cleanup. */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        sail_free(keys[i]);
    }

    sail_destroy_variant(value);
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_copy(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_map *hash_map1;
    munit_assert(sail_alloc_hash_map(&hash_map1) == SAIL_OK);

    const double reference_value1 = 11.5;
    const int reference_value2 = 101;

    /* Value 1. */
    struct sail_variant *value1;
    munit_assert(sail_alloc_variant(&value1) == SAIL_OK);
    sail_set_variant_double(value1, reference_value1);

    munit_assert(sail_put_hash_map(hash_map1, "ktop", value1) == SAIL_OK);
    sail_destroy_variant(value1);

    /* Value 2. */
    struct sail_variant *value2;
    munit_assert(sail_alloc_variant(&value2) == SAIL_OK);
    sail_set_variant_int(value2, reference_value2);

    munit_assert(sail_put_hash_map(hash_map1, "range", value2) == SAIL_OK);
    sail_destroy_variant(value2);

    struct sail_hash_map *hash_map2;
    munit_assert(sail_copy_hash_map(hash_map1, &hash_map2) == SAIL_OK);

    munit_assert(sail_test_compare_hash_maps(hash_map1, hash_map2) == SAIL_OK);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map2);
    sail_destroy_hash_map(hash_map1);

    return MUNIT_OK;
}

static MunitResult test_overwrite(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_map *hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    const double reference_value1 = 11.5;
    const double reference_value2 = 125.5;

    /* Value. */
    struct sail_variant *value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value1);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);

    /* Overwrite. */
    sail_set_variant_double(value, reference_value2);
    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    munit_assert(sail_hash_map_size(hash_map) == 1);

    const struct sail_variant *value_in_map = sail_hash_map_value(hash_map, "ktop");
    munit_assert_double(sail_variant_to_double(value_in_map), ==, reference_value2);

    /* Overwrite #2. */
    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    munit_assert(sail_hash_map_size(hash_map) == 1);

    value_in_map = sail_hash_map_value(hash_map, "ktop");
    munit_assert_double(sail_variant_to_double(value_in_map), ==, reference_value2);

    /* Cleanup. */
    sail_destroy_variant(value);
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_erase(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_map *hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    const double reference_value1 = 11.5;
    const int reference_value2 = 101;

    /* Value 1. */
    struct sail_variant *value1;
    munit_assert(sail_alloc_variant(&value1) == SAIL_OK);
    sail_set_variant_double(value1, reference_value1);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value1) == SAIL_OK);
    sail_destroy_variant(value1);

    /* Value 2. */
    struct sail_variant *value2;
    munit_assert(sail_alloc_variant(&value2) == SAIL_OK);
    sail_set_variant_int(value2, reference_value2);

    munit_assert(sail_put_hash_map(hash_map, "range", value2) == SAIL_OK);
    sail_destroy_variant(value2);
    munit_assert(sail_hash_map_has_key(hash_map, "range"));

    /* Erase. */
    sail_erase_hash_map_key(hash_map, "ktop");
    munit_assert(!sail_hash_map_has_key(hash_map, "ktop"));
    munit_assert(sail_hash_map_size(hash_map) == 1);

    sail_erase_hash_map_key(hash_map, "range");
    munit_assert(!sail_hash_map_has_key(hash_map, "range"));
    munit_assert(sail_hash_map_size(hash_map) == 0);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitResult test_clear(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_map *hash_map;
    munit_assert(sail_alloc_hash_map(&hash_map) == SAIL_OK);

    /* Clear an empty hash map. */
    sail_clear_hash_map(hash_map);

    const double reference_value = 11.5;

    /* Value. */
    struct sail_variant *value;
    munit_assert(sail_alloc_variant(&value) == SAIL_OK);
    sail_set_variant_double(value, reference_value);

    munit_assert(sail_put_hash_map(hash_map, "ktop", value) == SAIL_OK);
    sail_destroy_variant(value);

    /* Clear. */
    sail_clear_hash_map(hash_map);
    munit_assert(sail_hash_map_size(hash_map) == 0);

    /* Cleanup. */
    sail_destroy_hash_map(hash_map);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/put",            test_put,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/put-erase-many", test_put_erase_many, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",           test_copy,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/overwrite",      test_overwrite,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/erase",          test_erase,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/clear",          test_clear,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/hash-map",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
