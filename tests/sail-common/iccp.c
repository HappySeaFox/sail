/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

static MunitResult test_alloc_iccp(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_iccp *iccp = NULL;
    munit_assert(sail_alloc_iccp(&iccp) == SAIL_OK);
    munit_assert_not_null(iccp);
    munit_assert_null(iccp->data);
    munit_assert(iccp->data_length == 0);

    sail_destroy_iccp(iccp);

    return MUNIT_OK;
}

static MunitResult test_copy_iccp(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_iccp *iccp = NULL;
    munit_assert(sail_alloc_iccp(&iccp) == SAIL_OK);

    iccp->data_length = 1024;
    munit_assert(sail_malloc(iccp->data_length, &iccp->data) == SAIL_OK);
    munit_assert_not_null(iccp->data);

    memset(iccp->data, 15, iccp->data_length);

    struct sail_iccp *iccp_copy = NULL;
    munit_assert(sail_copy_iccp(iccp, &iccp_copy) == SAIL_OK);
    munit_assert_not_null(iccp_copy);

    munit_assert(iccp_copy->data != iccp->data);
    munit_assert(iccp_copy->data_length == iccp->data_length);
    munit_assert_memory_equal(iccp->data_length, iccp_copy->data, iccp->data);

    sail_destroy_iccp(iccp_copy);
    sail_destroy_iccp(iccp);

    return MUNIT_OK;
}

static MunitResult test_iccp_from_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const unsigned data_length = 1024;
    void *data = NULL;
    munit_assert(sail_malloc(data_length, &data) == SAIL_OK);
    memset(data, 15, data_length);
    munit_assert_not_null(data);

    struct sail_iccp *iccp = NULL;
    munit_assert(sail_alloc_iccp_from_data(data, data_length, &iccp) == SAIL_OK);
    munit_assert_not_null(iccp);

    munit_assert(iccp->data_length == data_length);
    munit_assert_memory_equal(data_length, iccp->data, data);

    sail_destroy_iccp(iccp);
    sail_free(data);

    return MUNIT_OK;
}

static MunitResult test_iccp_from_shallow_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const unsigned data_length = 1024;
    void *data = NULL;
    munit_assert(sail_malloc(data_length, &data) == SAIL_OK);
    memset(data, 15, data_length);
    munit_assert_not_null(data);

    struct sail_iccp *iccp = NULL;
    munit_assert(sail_alloc_iccp_move_data(data, data_length, &iccp) == SAIL_OK);
    munit_assert_not_null(iccp);

    munit_assert(iccp->data_length == data_length);
    munit_assert(iccp->data == data);
    data = NULL;

    sail_destroy_iccp(iccp);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc", test_alloc_iccp, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy", test_copy_iccp, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-data", test_iccp_from_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-shallow-data", test_iccp_from_shallow_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/iccp",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
