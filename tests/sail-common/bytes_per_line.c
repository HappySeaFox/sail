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

static MunitResult test_indexed(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 1-bit indexed. */
    munit_assert(sail_bytes_per_line(7, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 1);

    munit_assert(sail_bytes_per_line(8, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 1);

    munit_assert(sail_bytes_per_line(9, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 2);

    /* 2-bit indexed. */
    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 3);

    munit_assert(sail_bytes_per_line(12, SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 3);

    munit_assert(sail_bytes_per_line(13, SAIL_PIXEL_FORMAT_BPP2_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 4);

    /* 4-bit indexed. */
    munit_assert(sail_bytes_per_line(9, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 6);

    /* 8-bit indexed. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 10);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &result) == SAIL_OK);
    munit_assert(result == 11);

    return MUNIT_OK;
}

static MunitResult test_grayscale(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 1-bit grayscale. */
    munit_assert(sail_bytes_per_line(7, SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 1);

    munit_assert(sail_bytes_per_line(8, SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 1);

    munit_assert(sail_bytes_per_line(9, SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 2);

    /* 2-bit grayscale. */
    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 3);

    munit_assert(sail_bytes_per_line(12, SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 3);

    munit_assert(sail_bytes_per_line(13, SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 4);

    /* 4-bit grayscale. */
    munit_assert(sail_bytes_per_line(9, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 6);

    /* 8-bit grayscale. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 10);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 11);

    /* 16-bit grayscale. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, &result) == SAIL_OK);
    munit_assert(result == 22);

    return MUNIT_OK;
}

static MunitResult test_grayscale_alpha(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 4-bit grayscale-alpha. */
    munit_assert(sail_bytes_per_line(9, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 5);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 6);

    /* 8-bit grayscale-alpha. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 10);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 11);

    /* 16-bit grayscale-alpha. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 22);

    /* 32-bit grayscale-alpha. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 40);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA, &result) == SAIL_OK);
    munit_assert(result == 44);

    return MUNIT_OK;
}

static MunitResult test_rgb_555_565(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* RGB-555. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_RGB555, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_RGB555, &result) == SAIL_OK);
    munit_assert(result == 22);

    /* BGR-555. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_BGR555, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_BGR555, &result) == SAIL_OK);
    munit_assert(result == 22);

    /* RGB-565. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_RGB565, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_RGB565, &result) == SAIL_OK);
    munit_assert(result == 22);

    /* BGR-565. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP16_BGR565, &result) == SAIL_OK);
    munit_assert(result == 20);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP16_BGR565, &result) == SAIL_OK);
    munit_assert(result == 22);

    return MUNIT_OK;
}

static MunitResult test_rgb(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 24-bit RGB. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP24_RGB, &result) == SAIL_OK);
    munit_assert(result == 30);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP24_RGB, &result) == SAIL_OK);
    munit_assert(result == 33);

    /* 48-bit RGB. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP48_RGB, &result) == SAIL_OK);
    munit_assert(result == 60);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP48_RGB, &result) == SAIL_OK);
    munit_assert(result == 66);

    return MUNIT_OK;
}

static MunitResult test_rgba(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 32-bit RGBA. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP32_RGBA, &result) == SAIL_OK);
    munit_assert(result == 40);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP32_RGBA, &result) == SAIL_OK);
    munit_assert(result == 44);

    /* 64-bit RGBA. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP64_RGBA, &result) == SAIL_OK);
    munit_assert(result == 80);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP64_RGBA, &result) == SAIL_OK);
    munit_assert(result == 88);

    return MUNIT_OK;
}

static MunitResult test_cmyk(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 32-bit CMYK. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP32_CMYK, &result) == SAIL_OK);
    munit_assert(result == 40);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP32_CMYK, &result) == SAIL_OK);
    munit_assert(result == 44);

    /* 64-bit CMYK. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP64_CMYK, &result) == SAIL_OK);
    munit_assert(result == 80);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP64_CMYK, &result) == SAIL_OK);
    munit_assert(result == 88);

    return MUNIT_OK;
}

static MunitResult test_ycbcr(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 24-bit YCbCr. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP24_YCBCR, &result) == SAIL_OK);
    munit_assert(result == 30);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP24_YCBCR, &result) == SAIL_OK);
    munit_assert(result == 33);

    return MUNIT_OK;
}

static MunitResult test_ycck(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 32-bit YCbCr. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP32_YCCK, &result) == SAIL_OK);
    munit_assert(result == 40);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP32_YCCK, &result) == SAIL_OK);
    munit_assert(result == 44);

    return MUNIT_OK;
}

static MunitResult test_cie_lab(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    unsigned result;

    /* 24-bit CIE-LAB. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP24_CIE_LAB, &result) == SAIL_OK);
    munit_assert(result == 30);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP24_CIE_LAB, &result) == SAIL_OK);
    munit_assert(result == 33);

    /* 40-bit CIE-LAB. */
    munit_assert(sail_bytes_per_line(10, SAIL_PIXEL_FORMAT_BPP40_CIE_LAB, &result) == SAIL_OK);
    munit_assert(result == 50);

    munit_assert(sail_bytes_per_line(11, SAIL_PIXEL_FORMAT_BPP40_CIE_LAB, &result) == SAIL_OK);
    munit_assert(result == 55);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/indexed",         test_indexed,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/grayscale",       test_grayscale,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/grayscale-alpha", test_grayscale_alpha, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgb-555-565",     test_rgb_555_565,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgb",             test_rgb,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgba",            test_rgba,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/cmyk",            test_cmyk,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ycbcr",           test_ycbcr,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ycck",            test_ycck,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/cie-lab",         test_cie_lab,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bytes-per-line",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
