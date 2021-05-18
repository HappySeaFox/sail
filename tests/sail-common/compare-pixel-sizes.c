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

#include "sail-common.h"

#include "munit.h"

static MunitResult test_less(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    bool result;

    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,    SAIL_PIXEL_FORMAT_BPP32_RGBA,   &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,    SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA,   SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);

    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) != SAIL_OK);
    munit_assert(sail_less_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);

    return MUNIT_OK;
}

static MunitResult test_less_equal(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    bool result;

    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP32_RGBA,   &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP2_INDEXED,  SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA,    SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);

    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) != SAIL_OK);
    munit_assert(sail_less_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);

    return MUNIT_OK;
}

static MunitResult test_equal(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    bool result;

    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,  SAIL_PIXEL_FORMAT_BPP24_RGB,  &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,  SAIL_PIXEL_FORMAT_BPP24_BGR,  &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP32_RGBA, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP32_BGRA, &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP32_RGBA,   &result) == SAIL_OK); munit_assert(!result);

    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) != SAIL_OK);
    munit_assert(sail_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);

    return MUNIT_OK;
}

static MunitResult test_greater_equal(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    bool result;

    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP2_INDEXED,  SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA,    SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED,  SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED,  SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,     SAIL_PIXEL_FORMAT_BPP32_RGBA,   &result) == SAIL_OK); munit_assert(!result);

    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) != SAIL_OK);
    munit_assert(sail_greater_equal_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);

    return MUNIT_OK;
}

static MunitResult test_greater(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    bool result;

    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP2_INDEXED, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,    SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP32_RGBA,   SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(result);

    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,    SAIL_PIXEL_FORMAT_BPP24_RGB,    &result) == SAIL_OK); munit_assert(!result);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP24_RGB,    SAIL_PIXEL_FORMAT_BPP32_RGBA,   &result) == SAIL_OK); munit_assert(!result);

    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) != SAIL_OK);
    munit_assert(sail_greater_bits_per_pixel(SAIL_PIXEL_FORMAT_UNKNOWN,      SAIL_PIXEL_FORMAT_UNKNOWN,      &result) != SAIL_OK);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/less",          test_less,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/less-equal",    test_less_equal,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/equal",         test_equal,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/greater-equal", test_greater_equal, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/greater",       test_greater,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/compare-pixel-sizes",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
