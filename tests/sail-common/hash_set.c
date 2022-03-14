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

    struct sail_hash_set *hash_set;
    munit_assert(sail_alloc_hash_set(&hash_set) == SAIL_OK);

    /* Key 1. */
    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);
    munit_assert(sail_hash_set_has_key(hash_set, "ktop"));
    munit_assert(sail_hash_set_size(hash_set) == 1);

    /* Key 2. */
    munit_assert(sail_put_hash_set(hash_set, "range") == SAIL_OK);
    munit_assert(sail_hash_set_has_key(hash_set, "range"));
    munit_assert(sail_hash_set_size(hash_set) == 2);

    /* Cleanup. */
    sail_destroy_hash_set(hash_set);

    return MUNIT_OK;
}

static MunitResult test_copy(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_set *hash_set1;
    munit_assert(sail_alloc_hash_set(&hash_set1) == SAIL_OK);

    /* Key 1. */
    munit_assert(sail_put_hash_set(hash_set1, "ktop") == SAIL_OK);

    /* Key 2. */
    munit_assert(sail_put_hash_set(hash_set1, "range") == SAIL_OK);

    /* Copy. */
    struct sail_hash_set *hash_set2;
    munit_assert(sail_copy_hash_set(hash_set1, &hash_set2) == SAIL_OK);

    munit_assert(sail_test_compare_hash_sets(hash_set1, hash_set2) == SAIL_OK);

    /* Cleanup. */
    sail_destroy_hash_set(hash_set2);
    sail_destroy_hash_set(hash_set1);

    return MUNIT_OK;
}

static MunitResult test_overwrite(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_set *hash_set;
    munit_assert(sail_alloc_hash_set(&hash_set) == SAIL_OK);

    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);
    munit_assert(sail_hash_set_size(hash_set) == 1);

    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);
    munit_assert(sail_hash_set_size(hash_set) == 1);

    /* Cleanup. */
    sail_destroy_hash_set(hash_set);

    return MUNIT_OK;
}

static MunitResult test_erase(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_set *hash_set;
    munit_assert(sail_alloc_hash_set(&hash_set) == SAIL_OK);

    /* Key 1. */
    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);

    /* Key 2. */
    munit_assert(sail_put_hash_set(hash_set, "range") == SAIL_OK);

    /* Erase. */
    sail_erase_hash_set(hash_set, "ktop");
    munit_assert(!sail_hash_set_has_key(hash_set, "ktop"));
    munit_assert(sail_hash_set_size(hash_set) == 1);

    sail_erase_hash_set(hash_set, "range");
    munit_assert(!sail_hash_set_has_key(hash_set, "range"));
    munit_assert(sail_hash_set_size(hash_set) == 0);

    /* Cleanup. */
    sail_destroy_hash_set(hash_set);

    return MUNIT_OK;
}

static MunitResult test_clear(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_hash_set *hash_set;
    munit_assert(sail_alloc_hash_set(&hash_set) == SAIL_OK);

    /* Clear an empty hash set. */
    sail_clear_hash_set(hash_set);

    /* Keys. */
    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);
    munit_assert(sail_hash_set_size(hash_set) == 1);

    /* Clear. */
    sail_clear_hash_set(hash_set);
    munit_assert(sail_hash_set_size(hash_set) == 0);

    /* Keys. */
    munit_assert(sail_put_hash_set(hash_set, "ktop") == SAIL_OK);
    munit_assert(sail_put_hash_set(hash_set, "range") == SAIL_OK);
    munit_assert(sail_hash_set_size(hash_set) == 2);

    /* Clear. */
    sail_clear_hash_set(hash_set);
    munit_assert(sail_hash_set_size(hash_set) == 0);

    /* Cleanup. */
    sail_destroy_hash_set(hash_set);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/put",       test_put,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",      test_copy,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/overwrite", test_overwrite, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/erase",     test_erase,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/clear",     test_clear,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/hash-set",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
