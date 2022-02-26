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

#include <ctime>

#include "sail-c++.h"

#include "munit.h"

template<typename T>
static MunitResult test_type(const T &value) {

    sail::variant variant;
    variant.with_value(value);

    munit_assert(variant.is_valid());
    munit_assert(variant.has_value<T>());
    munit_assert(variant.value<T>() == value);

    return MUNIT_OK;
}

static MunitResult test_move(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    constexpr short reference_value = -500;

    sail::variant variant;
    variant.with_value<short>(reference_value);

    sail::variant variant2 = std::move(variant);

    munit_assert(variant2.is_valid());
    munit_assert(variant2.has_value<short>());
    munit_assert(variant2.value<short>() == reference_value);

    return MUNIT_OK;
}

static MunitResult test_with_value(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    test_type<char>('a');
    test_type<unsigned char>('a');

    test_type<short>(-5);
    test_type<unsigned short>(5566);

    test_type<int>(-500);
    test_type<unsigned int>(0xFFFF5);

    test_type<long>(-500);
    test_type<unsigned long>(0xFFFF5);

    test_type<float>(-5.0f);
    test_type<double>(120.0);

    test_type<std::string>("abc");
    const sail::arbitrary_data arbitrary_data(/* size */ 500, /* value */ 121);
    test_type<sail::arbitrary_data>(arbitrary_data);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/move",       test_move,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/with-value", test_with_value, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/variant",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
