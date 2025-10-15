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

#include <utility> /* move */

#include <sail-c++/sail-c++.h>

#include "munit.h"

static MunitResult test_image_create(const MunitParameter params[], void* user_data)
{
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
        // Peek a random pointer
        void* pixels = &user_data;
        sail::image image(pixels, SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert(image.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image.width(), ==, 16);
        munit_assert_uint(image.height(), ==, 16);
        munit_assert_ptr_equal(image.pixels(), pixels);
        munit_assert_true(image.is_valid());
    }

    {
        // Peek a random pointer
        void* pixels = &user_data;
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

static MunitResult test_image_copy(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    {
        sail::image image(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert_true(image.is_valid());

        sail::image image_copy = image;
        munit_assert(image_copy.pixel_format() == image.pixel_format());
        munit_assert_uint(image_copy.width(), ==, image.width());
        munit_assert_uint(image_copy.height(), ==, image.height());
        munit_assert_uint(image_copy.bytes_per_line(), ==, image.bytes_per_line());
        munit_assert_ptr_not_equal(image_copy.pixels(), image.pixels());
        munit_assert_true(image_copy.is_valid());
    }

    {
        char pixels[16 * 16 * 3];
        sail::image image(pixels, SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert_true(image.is_valid());

        sail::image image_copy = image;
        munit_assert(image_copy.pixel_format() == image.pixel_format());
        munit_assert_uint(image_copy.width(), ==, image.width());
        munit_assert_uint(image_copy.height(), ==, image.height());
        munit_assert_uint(image_copy.bytes_per_line(), ==, image.bytes_per_line());
        munit_assert_ptr_not_equal(image_copy.pixels(), image.pixels());
        munit_assert_true(image_copy.is_valid());
    }

    return MUNIT_OK;
}

static MunitResult test_image_move(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    {
        sail::image image(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert_true(image.is_valid());

        sail::image image_copy = std::move(image);
        munit_assert(image_copy.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image_copy.width(), ==, 16);
        munit_assert_uint(image_copy.height(), ==, 16);
        munit_assert_not_null(image_copy.pixels());
        munit_assert_true(image_copy.is_valid());
    }

    {
        char pixels[16 * 16 * 3];
        sail::image image(pixels, SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert_true(image.is_valid());

        sail::image image_copy = std::move(image);

        munit_assert(image_copy.pixel_format() == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(image_copy.width(), ==, 16);
        munit_assert_uint(image_copy.height(), ==, 16);
        munit_assert_not_null(image_copy.pixels());
        munit_assert_true(image_copy.is_valid());
    }

    return MUNIT_OK;
}

static MunitResult test_image_special_properties(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    {
        sail::image image(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, 16);
        munit_assert_true(image.is_valid());
        munit_assert_true(image.source_image().special_properties().empty());

        // Add special properties
        image.source_image().special_properties()["test-key-1"] = sail::variant(42u);
        image.source_image().special_properties()["test-key-2"] = sail::variant(std::string("test-value"));

        munit_assert_uint(image.source_image().special_properties().size(), ==, 2ull);
        munit_assert_uint(image.source_image().special_properties()["test-key-1"].value<unsigned>(), ==, 42u);
        munit_assert_string_equal(image.source_image().special_properties()["test-key-2"].value<std::string>().c_str(),
                                  "test-value");

        // Copy and verify special_properties are copied
        sail::image image_copy = image;
        munit_assert_uint(image_copy.source_image().special_properties().size(), ==, 2ull);
        munit_assert_uint(image_copy.source_image().special_properties()["test-key-1"].value<unsigned>(), ==, 42u);
        munit_assert_string_equal(
            image_copy.source_image().special_properties()["test-key-2"].value<std::string>().c_str(), "test-value");

        // Modify copy and verify original is not affected
        image_copy.source_image().special_properties()["test-key-3"] = sail::variant(100u);
        munit_assert_uint(image_copy.source_image().special_properties().size(), ==, 3ull);
        munit_assert_uint(image.source_image().special_properties().size(), ==, 2ull);
    }

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/create",             test_image_create,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",               test_image_copy,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/move",               test_image_move,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/special-properties", test_image_special_properties, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/image", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
