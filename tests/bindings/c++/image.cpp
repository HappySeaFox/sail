/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2024 Dmitry Baryshev

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

static MunitResult test_image_create(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    {
        sail::image image;
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN);
        munit_assert_null(image.pixels());
        munit_assert_false(image.is_valid());
    }

    {
        sail::image image(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image.width(), ==, 16);
        munit_assert_uint(image.height(), ==, 16);
        munit_assert_not_null(image.pixels());
        munit_assert_true(image.is_valid());
    }

    {
        sail::image image(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16, 50);
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image.width(), ==, 16);
        munit_assert_uint(image.height(), ==, 16);
        munit_assert_uint(image.bytes_per_line(), ==, 50);
        munit_assert_not_null(image.pixels());
        munit_assert_true(image.is_valid());
    }

    {
        void *pixels = "abcdef";
        sail::image image(pixels, SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image.width(), ==, 16);
        munit_assert_uint(image.height(), ==, 16);
        munit_assert_ptr_equal(image.pixels(), pixels);
        munit_assert_true(image.is_valid());
    }

    {
        void *pixels = "abcdef";
        sail::image image(pixels, SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16, 50);
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image.width(), ==, 16);
        munit_assert_uint(image.height(), ==, 16);
        munit_assert_uint(image.bytes_per_line(), ==, 50);
        munit_assert_ptr_equal(image.pixels(), pixels);
        munit_assert_true(image.is_valid());
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/create", test_image_create, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/image",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
