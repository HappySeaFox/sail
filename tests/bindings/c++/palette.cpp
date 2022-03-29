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

#include <utility> /* move */

#include "sail-c++.h"

#include "munit.h"

static sail::arbitrary_data construct_data() {

    sail::arbitrary_data data(8092);

    for (std::size_t i = 0; i < data.size(); i++) {
        data[i] = 50;
    }

    return data;
}

static MunitResult test_palette_create(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    {
        sail::arbitrary_data data = construct_data();

        const unsigned color_count = static_cast<unsigned>(data.size() / 2);

        sail::palette palette(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, data.data(), color_count);
        munit_assert(palette.pixel_format() == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE);
        munit_assert(palette.data()         == data);
        munit_assert(palette.color_count()  == color_count);
        munit_assert(palette.is_valid());
    }

    {
        sail::arbitrary_data data = construct_data();

        sail::palette palette(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, data);
        munit_assert(palette.pixel_format() == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE);
        munit_assert(palette.data()         == data);
        munit_assert(palette.color_count()  == (data.size() / 2));
        munit_assert(palette.is_valid());
    }

    {
        sail::palette palette;

        munit_assert(palette.color_count() == 0);
        munit_assert(palette.data().empty());
        munit_assert(palette.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN);
        munit_assert(!palette.is_valid());
    }

    return MUNIT_OK;
}

static MunitResult test_palette_copy(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    {
        sail::arbitrary_data data = construct_data();

        const unsigned color_count = static_cast<unsigned>(data.size() / 2);

        sail::palette palette(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, data.data(), color_count);
        munit_assert(palette.is_valid());

        sail::palette palette_copy = palette;
        munit_assert(palette_copy.color_count()  == palette.color_count());
        munit_assert(palette_copy.data()         == palette.data());
        munit_assert(palette_copy.pixel_format() == palette.pixel_format());
        munit_assert(palette_copy.is_valid());
    }

    {
        sail::palette palette;
        munit_assert(!palette.is_valid());

        sail::palette palette_copy = palette;
        munit_assert(palette_copy.color_count() == 0);
        munit_assert(palette_copy.data().empty());
        munit_assert(palette_copy.pixel_format() == palette.pixel_format());
        munit_assert(!palette_copy.is_valid());
    }

    return MUNIT_OK;
}

static MunitResult test_palette_move(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    {
        sail::arbitrary_data data = construct_data();

        const unsigned color_count = static_cast<unsigned>(data.size() / 2);

        sail::palette palette(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, data.data(), color_count);
        munit_assert(palette.is_valid());

        sail::palette palette_copy = std::move(palette);
        munit_assert(palette_copy.color_count()  == color_count);
        munit_assert(palette_copy.data()         == data);
        munit_assert(palette_copy.pixel_format() == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE);
        munit_assert(palette_copy.is_valid());
    }

    {
        sail::palette palette = sail::palette{};
        munit_assert(palette.color_count() == 0);
        munit_assert(palette.data().empty());
        munit_assert(palette.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN);
        munit_assert(!palette.is_valid());
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/create", test_palette_create, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",   test_palette_copy,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/move",   test_palette_move,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/palette",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
