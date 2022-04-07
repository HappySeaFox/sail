/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include <stdio.h>

#include "sail.h"
#include "sail-manip.h"

#include "munit.h"

static MunitResult test_best_conversion_grayscale(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP24_RGB };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP2_INDEXED };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_UNKNOWN);
    }

    return MUNIT_OK;
}

static MunitResult test_best_conversion_indexed(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP8_INDEXED, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP24_RGB };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP1_INDEXED, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP1_INDEXED, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
    }

    return MUNIT_OK;
}

static MunitResult test_best_conversion_rgb(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP24_RGB, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP32_RGBA };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP24_RGB, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);
    }

    {
        const enum SailPixelFormat pixel_formats[] = { SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE };
        const size_t pixel_formats_length = sizeof(pixel_formats) / sizeof(pixel_formats[0]);

        munit_assert_int(sail_closest_pixel_format(SAIL_PIXEL_FORMAT_BPP24_RGB, pixel_formats, pixel_formats_length), ==, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/grayscale", test_best_conversion_grayscale, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed", test_best_conversion_indexed, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgb", test_best_conversion_rgb, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/best-conversion",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
