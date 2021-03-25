/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include <string.h>

#include "sail-common.h"

#include "munit.h"

static MunitResult test_alloc_palette(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_palette *palette = NULL;
    munit_assert(sail_alloc_palette(&palette) == SAIL_OK);
    munit_assert_not_null(palette);
    munit_assert_null(palette->data);
    munit_assert(palette->color_count == 0);
    munit_assert(palette->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN);

    sail_destroy_palette(palette);

    return MUNIT_OK;
}

static MunitResult test_copy_palette(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_palette *palette = NULL;
    munit_assert(sail_alloc_palette(&palette) == SAIL_OK);

    palette->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    palette->color_count = 10;
    const unsigned data_length = palette->color_count * 3;
    munit_assert(sail_malloc(data_length, &palette->data) == SAIL_OK);
    munit_assert_not_null(palette->data);

    memset(palette->data, 15, data_length);

    struct sail_palette *palette_copy = NULL;
    munit_assert(sail_copy_palette(palette, &palette_copy) == SAIL_OK);
    munit_assert_not_null(palette_copy);

    munit_assert(palette_copy->data != palette->data);
    munit_assert(palette_copy->pixel_format == palette->pixel_format);
    munit_assert_memory_equal(data_length, palette_copy->data, palette->data);
    munit_assert(palette_copy->color_count == palette->color_count);

    sail_destroy_palette(palette_copy);
    sail_destroy_palette(palette);

    return MUNIT_OK;
}

static MunitResult test_palette_from_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const enum SailPixelFormat pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    const unsigned color_count = 100;
    const unsigned data_length = color_count * 3;

    void *data = NULL;
    munit_assert(sail_malloc(data_length, &data) == SAIL_OK);
    memset(data, 15, data_length);
    munit_assert_not_null(data);

    struct sail_palette *palette = NULL;
    munit_assert(sail_alloc_palette_from_data(pixel_format, data, color_count, &palette) == SAIL_OK);
    munit_assert_not_null(palette);

    munit_assert(palette->pixel_format == pixel_format);
    munit_assert_memory_equal(data_length, palette->data, data);
    munit_assert(palette->color_count == color_count);

    sail_destroy_palette(palette);
    sail_free(data);

    return MUNIT_OK;
}

static MunitResult test_palette_for_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const enum SailPixelFormat pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    const unsigned color_count = 100;
    const unsigned data_length = color_count * 3;

    struct sail_palette *palette = NULL;
    munit_assert(sail_alloc_palette_for_data(pixel_format, color_count, &palette) == SAIL_OK);
    munit_assert_not_null(palette);

    munit_assert(palette->pixel_format == pixel_format);
    munit_assert_not_null(palette->data);
    munit_assert(palette->color_count == color_count);

    memset(palette->data, 15, data_length);

    sail_destroy_palette(palette);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc", test_alloc_palette, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy", test_copy_palette, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-data", test_palette_from_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/for-data", test_palette_for_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/palette",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
