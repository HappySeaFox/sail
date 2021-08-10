/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

static MunitResult test_hex_string_to_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    void *data;
    size_t data_size;

    {
        const char *str = "61 62\n63";
        munit_assert(sail_hex_string_to_data(str, &data, &data_size) == SAIL_OK);
        munit_assert(data_size == 3);
        munit_assert_memory_equal(data_size, data, "abc");
        sail_free(data);
    }

    {
        const char *str = " 61\r\n62\n63 ";
        munit_assert(sail_hex_string_to_data(str, &data, &data_size) == SAIL_OK);
        munit_assert(data_size == 3);
        munit_assert_memory_equal(data_size, data, "abc");
        sail_free(data);
    }

    {
        const char *str = "0A410A";
        munit_assert(sail_hex_string_to_data(str, &data, &data_size) == SAIL_OK);
        munit_assert(data_size == 3);
        munit_assert_memory_equal(data_size, data, "\nA\n");
        sail_free(data);
    }


    {
        const char *str = "616263";
        munit_assert(sail_malloc(3, &data) == SAIL_OK);
        munit_assert(sail_hex_string_into_data(str, data) == SAIL_OK);
        munit_assert_memory_equal(3, data, "abc");
        sail_free(data);
    }

    {
        const char *str = "0A410A";
        munit_assert(sail_malloc(3, &data) == SAIL_OK);
        munit_assert(sail_hex_string_into_data(str, data) == SAIL_OK);
        munit_assert_memory_equal(3, data, "\nA\n");
        sail_free(data);
    }

    {
        /* Odd length. */
        const char *str = "0A4";
        data = NULL;
        munit_assert(sail_hex_string_to_data(str, &data, &data_size) == SAIL_OK);
        munit_assert_memory_equal(1, data, "\n");
        sail_free(data);
    }

    {
        /* NULL strings must fail. */
        data = NULL;
        munit_assert(sail_hex_string_to_data(NULL, &data, &data_size) != SAIL_OK);
        munit_assert_null(data);
    }

    return MUNIT_OK;
}

static MunitResult test_data_to_hex_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    char *str;
    void *ptr;

    {
        str = NULL;
        const char *data = "abc";
        munit_assert(sail_data_to_hex_string(data, strlen(data), &str) == SAIL_OK);
        munit_assert_not_null(str);
        munit_assert(strlen(str) == 3*2);
        munit_assert_string_equal(str, "616263");
        sail_free(str);
    }

    {
        str = NULL;
        const char *data = "\nA\n";
        munit_assert(sail_data_to_hex_string(data, strlen(data), &str) == SAIL_OK);
        munit_assert_not_null(str);
        munit_assert(strlen(str) == 3*2);
        munit_assert_string_equal(str, "0A410A");
        sail_free(str);
    }

    {
        const char *data = "abc";
        munit_assert(sail_malloc(3*2+1, &ptr) == SAIL_OK);
        str = ptr;
        munit_assert(sail_data_into_hex_string(data, strlen(data), str) == SAIL_OK);
        munit_assert(strlen(str) == 3*2);
        munit_assert_string_equal(str, "616263");
        sail_free(str);
    }

    {
        const char *data = "\nA\n";
        munit_assert(sail_malloc(3*2+1, &ptr) == SAIL_OK);
        str = ptr;
        munit_assert(sail_data_into_hex_string(data, strlen(data), str) == SAIL_OK);
        munit_assert(strlen(str) == 3*2);
        munit_assert_string_equal(str, "0A410A");
        sail_free(str);
    }

    {
        /* NULL data must fail. */
        str = NULL;
        munit_assert(sail_data_to_hex_string(NULL, 0, &str) != SAIL_OK);
        munit_assert_null(str);
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/hex-string-to-data", test_hex_string_to_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/data-to-hex-string", test_data_to_hex_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/hex-data",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
