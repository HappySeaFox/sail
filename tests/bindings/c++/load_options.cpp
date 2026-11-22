/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <utility>

#include <sail-c++/suppress_begin.h>
#include <sail-c++/suppress_c4251.h>

#include <sail-c++/codec_info.h>
#include <sail-c++/load_features.h>
#include <sail-c++/load_options.h>

#include <sail-c++/suppress_end.h>

#include "munit.h"

static MunitResult test_load_options_construct(const MunitParameter params[], void* user_data)
{

    (void)params;
    (void)user_data;

    {
        sail::load_options load_options;

        munit_assert(load_options.options() == 0);
        munit_assert(load_options.tuning().empty());
        munit_assert_uint(load_options.max_width(), ==, 0);
        munit_assert_uint(load_options.max_height(), ==, 0);
        munit_assert_uint64(load_options.max_pixels(), ==, 0);
        munit_assert_uint(load_options.max_frames(), ==, 0);
    }

    {
        sail::load_options load_options(SAIL_OPTION_META_DATA);

        munit_assert(load_options.options() == SAIL_OPTION_META_DATA);
        munit_assert(load_options.tuning().empty());
    }

    return MUNIT_OK;
}

static MunitResult test_load_options_copy(const MunitParameter params[], void* user_data)
{

    (void)params;
    (void)user_data;

    const sail::codec_info first_codec = sail::codec_info::list().front();

    {
        sail::load_options load_options;
        munit_assert(first_codec.load_features().to_options(&load_options) == SAIL_OK);
        munit_assert(load_options.tuning().empty());

        const sail::load_options load_options2 = load_options;
        munit_assert(load_options.options() == load_options2.options());
        munit_assert(load_options.tuning() == load_options2.tuning());
    }

    {
        sail::load_options load_options;
        munit_assert(first_codec.load_features().to_options(&load_options) == SAIL_OK);
        load_options.tuning()["key"] = 10.0;
        munit_assert_double(load_options.tuning()["key"].value<double>(), ==, 10.0);

        const sail::load_options load_options2 = load_options;
        munit_assert(load_options.options() == load_options2.options());
        munit_assert(load_options.tuning() == load_options2.tuning());
    }

    {
        sail::load_options load_options;
        load_options.set_max_width(8192);
        load_options.set_max_height(4096);
        load_options.set_max_pixels(10000000000ULL);
        load_options.set_max_frames(100);

        const sail::load_options load_options2 = load_options;
        munit_assert_uint(load_options2.max_width(), ==, 8192);
        munit_assert_uint(load_options2.max_height(), ==, 4096);
        munit_assert_uint64(load_options2.max_pixels(), ==, 10000000000ULL);
        munit_assert_uint(load_options2.max_frames(), ==, 100);
    }

    return MUNIT_OK;
}

static MunitResult test_load_options_move(const MunitParameter params[], void* user_data)
{

    (void)params;
    (void)user_data;

    const sail::codec_info first_codec = sail::codec_info::list().front();

    {
        sail::load_options load_options;
        munit_assert(first_codec.load_features().to_options(&load_options) == SAIL_OK);

        sail::load_options load_options2       = load_options;
        const sail::load_options load_options3 = std::move(load_options2);
        munit_assert(load_options.options() == load_options3.options());
        munit_assert(load_options.tuning() == load_options3.tuning());
    }

    {
        sail::load_options load_options;
        munit_assert(first_codec.load_features().to_options(&load_options) == SAIL_OK);
        load_options.tuning()["key"] = 10.0;
        munit_assert_double(load_options.tuning()["key"].value<double>(), ==, 10.0);

        sail::load_options load_options2       = load_options;
        const sail::load_options load_options3 = std::move(load_options2);
        munit_assert(load_options.options() == load_options3.options());
        munit_assert(load_options.tuning() == load_options3.tuning());
    }

    {
        sail::load_options load_options;
        load_options.set_max_width(8192);
        load_options.set_max_height(4096);
        load_options.set_max_pixels(10000000000ULL);
        load_options.set_max_frames(100);

        const sail::load_options load_options2 = std::move(load_options);
        munit_assert_uint(load_options2.max_width(), ==, 8192);
        munit_assert_uint(load_options2.max_height(), ==, 4096);
        munit_assert_uint64(load_options2.max_pixels(), ==, 10000000000ULL);
        munit_assert_uint(load_options2.max_frames(), ==, 100);
    }

    return MUNIT_OK;
}

static MunitResult test_load_options_max_dimensions(const MunitParameter params[], void* user_data)
{

    (void)params;
    (void)user_data;

    sail::load_options load_options;
    munit_assert(load_options.max_dimensions() == std::make_pair(0u, 0u));

    load_options.set_max_dimensions(8192, 4096);
    munit_assert_uint(load_options.max_width(), ==, 8192);
    munit_assert_uint(load_options.max_height(), ==, 4096);
    munit_assert_uint(load_options.max_dimensions().first, ==, 8192);
    munit_assert_uint(load_options.max_dimensions().second, ==, 4096);

    load_options.set_max_width(100);
    munit_assert(load_options.max_dimensions() == std::make_pair(100u, 4096u));

    load_options.set_max_height(200);
    munit_assert(load_options.max_dimensions() == std::make_pair(100u, 200u));

    load_options.set_max_dimensions(0, 0);
    munit_assert(load_options.max_dimensions() == std::make_pair(0u, 0u));

    return MUNIT_OK;
}

static MunitResult test_load_options_max_pixels_and_frames(const MunitParameter params[], void* user_data)
{

    (void)params;
    (void)user_data;

    sail::load_options load_options;

    load_options.set_max_pixels(10000000000ULL);
    load_options.set_max_frames(100);
    munit_assert_uint64(load_options.max_pixels(), ==, 10000000000ULL);
    munit_assert_uint(load_options.max_frames(), ==, 100);

    load_options.set_max_pixels(0);
    load_options.set_max_frames(0);
    munit_assert_uint64(load_options.max_pixels(), ==, 0);
    munit_assert_uint(load_options.max_frames(), ==, 0);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/construct",             test_load_options_construct,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",                  test_load_options_copy,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/move",                  test_load_options_move,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/max-dimensions",        test_load_options_max_dimensions,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/max-pixels-and-frames", test_load_options_max_pixels_and_frames, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/load-options", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
