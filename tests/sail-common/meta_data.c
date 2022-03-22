/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include "sail-comparators.h"

#include "munit.h"

static MunitResult test_alloc_meta_data(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_meta_data *meta_data = NULL;
    munit_assert(sail_alloc_meta_data(&meta_data) == SAIL_OK);
    munit_assert_not_null(meta_data);
    munit_assert_null(meta_data->key_unknown);
    munit_assert_null(meta_data->value);

    sail_destroy_meta_data(meta_data);

    return MUNIT_OK;
}

static MunitResult test_meta_data_from_known_key(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_meta_data *meta_data = NULL;
    munit_assert(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_COMMENT, &meta_data) == SAIL_OK);
    munit_assert_not_null(meta_data);
    munit_assert(meta_data->key == SAIL_META_DATA_COMMENT);
    munit_assert_null(meta_data->key_unknown);

    sail_destroy_meta_data(meta_data);

    return MUNIT_OK;
}

static MunitResult test_meta_data_from_unknown_key(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const char *key = "Some Key";

    struct sail_meta_data *meta_data = NULL;
    munit_assert(sail_alloc_meta_data_from_unknown_key(key, &meta_data) == SAIL_OK);
    munit_assert_not_null(meta_data);
    munit_assert_null(meta_data->value);
    munit_assert_not_null(meta_data->key_unknown);
    munit_assert_string_equal(meta_data->key_unknown, key);

    sail_destroy_meta_data(meta_data);

    return MUNIT_OK;
}

static MunitResult test_copy_known_string_meta_data(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const char *value = "Comment 1";

    struct sail_meta_data *meta_data = NULL;
    munit_assert(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_COMMENT, &meta_data) == SAIL_OK);
    munit_assert_not_null(meta_data);

    munit_assert(sail_alloc_variant(&meta_data->value) == SAIL_OK);
    munit_assert(sail_set_variant_string(meta_data->value, value) == SAIL_OK);
    munit_assert_not_null(meta_data->value);

    struct sail_meta_data *meta_data_copy = NULL;
    munit_assert(sail_copy_meta_data(meta_data, &meta_data_copy) == SAIL_OK);
    munit_assert_not_null(meta_data_copy);

    munit_assert(sail_test_compare_meta_datas(meta_data_copy, meta_data) == SAIL_OK);

    sail_destroy_meta_data(meta_data_copy);
    sail_destroy_meta_data(meta_data);

    return MUNIT_OK;
}

static MunitResult test_copy_unknown_string_meta_data(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const char *key = "Some Key";
    const char *value = "Comment 1";

    struct sail_meta_data *meta_data = NULL;
    munit_assert(sail_alloc_meta_data_from_unknown_key(key, &meta_data) == SAIL_OK);
    munit_assert_not_null(meta_data);

    munit_assert(sail_alloc_variant(&meta_data->value) == SAIL_OK);
    munit_assert(sail_set_variant_string(meta_data->value, value) == SAIL_OK);
    munit_assert_not_null(meta_data->value);

    struct sail_meta_data *meta_data_copy = NULL;
    munit_assert(sail_copy_meta_data(meta_data, &meta_data_copy) == SAIL_OK);
    munit_assert_not_null(meta_data_copy);

    munit_assert(sail_test_compare_meta_datas(meta_data_copy, meta_data) == SAIL_OK);

    sail_destroy_meta_data(meta_data_copy);
    sail_destroy_meta_data(meta_data);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc",               test_alloc_meta_data,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-known-key",      test_meta_data_from_known_key,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-unknown-key",    test_meta_data_from_unknown_key,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy-known-string",   test_copy_known_string_meta_data,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy-unknown-string", test_copy_unknown_string_meta_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/meta_data",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
