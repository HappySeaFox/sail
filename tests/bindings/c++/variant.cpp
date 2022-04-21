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
#include <functional>

#include "sail-c++.h"

#include "munit.h"

template<typename T>
static MunitResult test_type(const T &value) {

    const sail::variant variant(value);

    munit_assert(variant.is_valid());
    munit_assert(variant.has_value<T>());
    munit_assert(variant.value<T>() == value);

    return MUNIT_OK;
}

template<typename T>
static MunitResult test_equal(const T &value) {

    sail::variant variant1;
    variant1.set_value(value);

    sail::variant variant2;
    variant2.set_value(value);

    munit_assert(std::equal_to<sail::variant>()(variant1, variant2));
    munit_assert(std::equal_to<sail::variant>()(variant2, variant1));

    return MUNIT_OK;
}

template<typename T1, typename T2>
static MunitResult test_not_equal(const T1 &value1, const T2 &value2) {

    sail::variant variant1;
    variant1.set_value(value1);

    sail::variant variant2;
    variant2.set_value(value2);

    munit_assert(std::not_equal_to<sail::variant>()(variant1, variant2));
    munit_assert(std::not_equal_to<sail::variant>()(variant2, variant1));

    return MUNIT_OK;
}

static MunitResult test_move(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    constexpr short reference_value = -500;

    sail::variant variant;
    variant.set_value<short>(reference_value);

    sail::variant variant2 = std::move(variant);

    munit_assert(variant2.is_valid());
    munit_assert(variant2.has_value<short>());
    munit_assert(variant2.value<short>() == reference_value);

    return MUNIT_OK;
}

static MunitResult test_set_value(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    test_type<bool>(true);

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

static MunitResult test_compare(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    // ==
    test_equal<bool>(true);

    test_equal<char>('a');
    test_equal<unsigned char>('a');

    test_equal<short>(-5);
    test_equal<unsigned short>(5566);

    test_equal<int>(-500);
    test_equal<unsigned int>(0xFFFF5);

    test_equal<long>(-500);
    test_equal<unsigned long>(0xFFFF5);

    test_equal<float>(-5.0f);
    test_equal<double>(120.0);

    test_equal<std::string>("abc");
    const sail::arbitrary_data arbitrary_data(/* size */ 500, /* value */ 121);
    test_equal<sail::arbitrary_data>(arbitrary_data);

    // !=
    test_not_equal<bool, bool>(true, false);

    test_not_equal<char, char>('a', 'b');
    test_not_equal<char, unsigned char>('a', 'a');

    test_not_equal<unsigned char, unsigned char>('a', 'b');
    test_not_equal<unsigned char, char>('a', 'a');

    test_not_equal<short, short>(5, 10);
    test_not_equal<short, unsigned short>(5, 5);

    test_not_equal<unsigned short, unsigned short>(5, 10);
    test_not_equal<unsigned short, short>(5, 5);

    test_not_equal<int, int>(500, 501);
    test_not_equal<int, unsigned int>(500, 500);

    test_not_equal<unsigned int, unsigned int>(500, 501);
    test_not_equal<unsigned int, int>(500, 500);

    test_not_equal<long, long>(500, 501);
    test_not_equal<long, unsigned long>(500, 500);

    test_not_equal<unsigned long, unsigned long>(500, 501);
    test_not_equal<unsigned long, long>(500, 500);

    test_not_equal<float, float>(-5.0f, -10.0f);
    test_not_equal<float, int>(-5.0f, 1);

    test_not_equal<double, double>(-5.0, -10.0);
    test_not_equal<double, int>(-5.0, 1);

    test_not_equal<std::string, std::string>("abc", "def");
    test_not_equal<std::string, int>("abc", 6);

    const sail::arbitrary_data arbitrary_data1(/* size */ 500, /* value */ 66);
    const sail::arbitrary_data arbitrary_data2(/* size */ 400, /* value */ 66);
    const sail::arbitrary_data arbitrary_data3(/* size */ 500, /* value */ 90);
    test_not_equal<sail::arbitrary_data, sail::arbitrary_data>(arbitrary_data1, arbitrary_data2);
    test_not_equal<sail::arbitrary_data, sail::arbitrary_data>(arbitrary_data1, arbitrary_data3);
    test_not_equal<sail::arbitrary_data, int>(arbitrary_data1, 777);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/move",      test_move,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/set-value", test_set_value, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/compare",   test_compare,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

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
