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

#include "munit.h"

static MunitResult test_alloc_meta_data_node(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node(&meta_data_node) == SAIL_OK);
    munit_assert_not_null(meta_data_node);
    munit_assert_null(meta_data_node->key_unknown);
    munit_assert(meta_data_node->value_type == SAIL_META_DATA_TYPE_STRING);
    munit_assert_null(meta_data_node->value);
    munit_assert_null(meta_data_node->next);

    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitResult test_copy_meta_data_node(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node(&meta_data_node) == SAIL_OK);

    meta_data_node->key = SAIL_META_DATA_COMMENT;
    meta_data_node->value_type = SAIL_META_DATA_TYPE_DATA;
    meta_data_node->value_length = 1024;
    munit_assert(sail_malloc(meta_data_node->value_length, &meta_data_node->value) == SAIL_OK);
    munit_assert_not_null(meta_data_node->value);

    memset(meta_data_node->value, 15, meta_data_node->value_length);

    struct sail_meta_data_node *meta_data_node_copy = NULL;
    munit_assert(sail_copy_meta_data_node(meta_data_node, &meta_data_node_copy) == SAIL_OK);
    munit_assert_not_null(meta_data_node_copy);

    munit_assert(meta_data_node_copy->key == meta_data_node->key);
    munit_assert_null(meta_data_node_copy->key_unknown);
    munit_assert(meta_data_node_copy->value_type == meta_data_node->value_type);
    munit_assert(meta_data_node_copy->value != meta_data_node->value);
    munit_assert(meta_data_node_copy->value_length == meta_data_node->value_length);
    munit_assert(memcmp(meta_data_node_copy->value, meta_data_node->value, meta_data_node->value_length) == 0);

    sail_destroy_meta_data_node(meta_data_node_copy);
    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitResult test_meta_data_node_from_known_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const char *value = "Comment 1";

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node_from_known_string(SAIL_META_DATA_COMMENT, value, &meta_data_node) == SAIL_OK);
    munit_assert_not_null(meta_data_node);
    munit_assert(meta_data_node->key == SAIL_META_DATA_COMMENT);
    munit_assert_null(meta_data_node->key_unknown);
    munit_assert(meta_data_node->value_type == SAIL_META_DATA_TYPE_STRING);
    munit_assert_not_null(meta_data_node->value);
    munit_assert(meta_data_node->value_length == strlen(value) + 1);
    munit_assert_string_equal((char *)meta_data_node->value, value);

    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitResult test_meta_data_node_from_unknown_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const char *key = "Some Key";
    const char *value = "Comment 1";

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node_from_unknown_string(key, value, &meta_data_node) == SAIL_OK);
    munit_assert_not_null(meta_data_node);
    munit_assert(meta_data_node->key == SAIL_META_DATA_UNKNOWN);
    munit_assert_not_null(meta_data_node->key_unknown);
    munit_assert_string_equal(meta_data_node->key_unknown, key);
    munit_assert(meta_data_node->value_type == SAIL_META_DATA_TYPE_STRING);
    munit_assert_not_null(meta_data_node->value);
    munit_assert(meta_data_node->value_length == strlen(value) + 1);
    munit_assert_string_equal((char *)meta_data_node->value, value);

    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitResult test_meta_data_node_from_known_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const char *value = "Comment 1";
    const size_t value_length = strlen(value) + 1;

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node_from_known_data(SAIL_META_DATA_COMMENT, value, value_length, &meta_data_node) == SAIL_OK);
    munit_assert_not_null(meta_data_node);
    munit_assert(meta_data_node->key == SAIL_META_DATA_COMMENT);
    munit_assert_null(meta_data_node->key_unknown);
    munit_assert(meta_data_node->value_type == SAIL_META_DATA_TYPE_DATA);
    munit_assert_not_null(meta_data_node->value);
    munit_assert(meta_data_node->value_length == value_length);
    munit_assert(memcmp(meta_data_node->value, value, value_length) == 0);

    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitResult test_meta_data_node_from_unknown_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const char *key = "Some Key";
    const char *value = "Comment 1";
    const size_t value_length = strlen(value) + 1;

    struct sail_meta_data_node *meta_data_node = NULL;
    munit_assert(sail_alloc_meta_data_node_from_unknown_data(key, value, value_length, &meta_data_node) == SAIL_OK);
    munit_assert_not_null(meta_data_node);
    munit_assert(meta_data_node->key == SAIL_META_DATA_UNKNOWN);
    munit_assert_not_null(meta_data_node->key_unknown);
    munit_assert_string_equal(meta_data_node->key_unknown, key);
    munit_assert(meta_data_node->value_type == SAIL_META_DATA_TYPE_DATA);
    munit_assert_not_null(meta_data_node->value);
    munit_assert(meta_data_node->value_length == value_length);
    munit_assert(memcmp(meta_data_node->value, value, value_length) == 0);

    sail_destroy_meta_data_node(meta_data_node);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc", test_alloc_meta_data_node, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy", test_copy_meta_data_node, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-known-string", test_meta_data_node_from_known_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-unknown-string", test_meta_data_node_from_unknown_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-known-data", test_meta_data_node_from_known_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-unknown-data", test_meta_data_node_from_unknown_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/meta_data_node",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
