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

static MunitResult test_malloc(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const size_t size = 10 * 1024;

    void *ptr = NULL;
    munit_assert(sail_malloc(size, &ptr) == SAIL_OK);
    munit_assert_not_null(ptr);

    memset(ptr, 0, size);
    sail_free(ptr);

    return MUNIT_OK;
}

static MunitResult test_calloc(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const size_t size = 1;
    const size_t members = 1024;

    void *ptr = NULL;
    munit_assert(sail_calloc(members, size, &ptr) == SAIL_OK);
    munit_assert_not_null(ptr);

    unsigned char *cptr = ptr;

    for (size_t i = 0; i < size * members; i++, cptr++) {
        munit_assert(*cptr == 0);
    }

    sail_free(ptr);

    return MUNIT_OK;
}

static MunitResult test_realloc(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const size_t size = 10 * 1024;
    size_t current_size = size;
    const double factor = 1.2;
    const int iterations = 10;
    int iteration = 0;

    void *ptr = NULL;

    while (iteration++ < iterations) {
        munit_assert(sail_realloc(current_size, &ptr) == SAIL_OK);
        munit_assert_not_null(ptr);

        memset(ptr, 0, current_size);

        current_size = (size_t)(current_size * factor);
    }

    sail_free(ptr);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/malloc",  test_malloc,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/calloc",  test_calloc,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/realloc", test_realloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/malloc",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
