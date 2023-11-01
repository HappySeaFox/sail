/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#include <sail-c++/sail-c++.h>

#include "munit.h"

static MunitResult test_reverse_uint16(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_uint16(sail::reverse_bytes((std::uint16_t)0),    ==, 0);
    munit_assert_uint16(sail::reverse_bytes((std::uint16_t)100),  ==, 25600);
    munit_assert_uint16(sail::reverse_bytes((std::uint16_t)1000), ==, 59395);

    return MUNIT_OK;
}

static MunitResult test_reverse_uint32(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_uint32(sail::reverse_bytes((std::uint32_t)0),      ==, 0);
    munit_assert_uint32(sail::reverse_bytes((std::uint32_t)100),    ==, 1677721600);
    munit_assert_uint32(sail::reverse_bytes((std::uint32_t)1000),   ==, 3892510720);
    munit_assert_uint32(sail::reverse_bytes((std::uint32_t)100000), ==, 2693136640);

    return MUNIT_OK;
}

static MunitResult test_reverse_uint64(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_uint64(sail::reverse_bytes((std::uint64_t)0ull),           ==, 0ull);
    munit_assert_uint64(sail::reverse_bytes((std::uint64_t)100ull),         ==, 7205759403792793600ull);
    munit_assert_uint64(sail::reverse_bytes((std::uint64_t)1000ull),        ==, 16718206241729413120ull);
    munit_assert_uint64(sail::reverse_bytes((std::uint64_t)100000ull),      ==, 11566933792459325440ull);
    munit_assert_uint64(sail::reverse_bytes((std::uint64_t)10000000000ull), ==, 64188750128742400ull);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/reverse-uint16", test_reverse_uint16, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/reverse-uint32", test_reverse_uint32, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/reverse-uint64", test_reverse_uint64, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/utils",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
