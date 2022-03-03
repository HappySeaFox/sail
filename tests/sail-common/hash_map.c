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
#include <string.h>

#include "sail-common.h"

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
    { (char *)"/put",   test_put,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/erase", test_erase, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/clear", test_clear, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

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
