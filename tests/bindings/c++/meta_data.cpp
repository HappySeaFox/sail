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

#include <utility> /* move */

#include "sail-c++.h"

#include "munit.h"

static MunitResult test_meta_data_create(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    {
        const sail::variant value(std::string("Mike"));
        const sail::meta_data meta_data(SAIL_META_DATA_AUTHOR, value);

        munit_assert(meta_data.key_unknown().empty());
        munit_assert(meta_data.key()   == SAIL_META_DATA_AUTHOR);
        munit_assert(meta_data.value() == value);
    }

    {
        const sail::meta_data meta_data(SAIL_META_DATA_AUTHOR, sail::variant(std::string("Mike")));
        munit_assert(meta_data.key_unknown().empty());
        munit_assert(meta_data.key()   == SAIL_META_DATA_AUTHOR);
        munit_assert(meta_data.value() == sail::variant(std::string("Mike")));
    }

    {
        const sail::meta_data meta_data("Unknown Key", sail::variant(std::string("Mike")));
        munit_assert(meta_data.key_unknown() == "Unknown Key");
        munit_assert(meta_data.key()   == SAIL_META_DATA_UNKNOWN);
        munit_assert(meta_data.value() == sail::variant(std::string("Mike")));
    }

    return MUNIT_OK;
}

static MunitResult test_meta_data_copy(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const sail::variant value(std::string("Mike"));
    const sail::meta_data meta_data(SAIL_META_DATA_AUTHOR, value);
    const sail::meta_data meta_data2 = meta_data;

    munit_assert(meta_data2.key_unknown().empty());
    munit_assert(meta_data2.key()   == SAIL_META_DATA_AUTHOR);
    munit_assert(meta_data2.value() == value);

    return MUNIT_OK;
}

static MunitResult test_meta_data_move(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const sail::variant value(std::string("Mike"));
    sail::meta_data meta_data(SAIL_META_DATA_AUTHOR, value);
    const sail::meta_data meta_data2 = std::move(meta_data);

    munit_assert(meta_data2.key_unknown().empty());
    munit_assert(meta_data2.key()   == SAIL_META_DATA_AUTHOR);
    munit_assert(meta_data2.value() == value);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/create", test_meta_data_create, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",   test_meta_data_copy,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/move",   test_meta_data_move,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/meta-data",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
