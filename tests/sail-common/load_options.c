/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <limits.h>
#include <stdint.h>

#include <sail-common/sail-common.h>

#include "munit.h"

static MunitResult test_alloc_options(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);
    munit_assert_not_null(load_options);
    munit_assert(load_options->options == 0);
    munit_assert_null(load_options->tuning);
    munit_assert_uint(load_options->max_width, ==, 0);
    munit_assert_uint(load_options->max_height, ==, 0);
    munit_assert_uint64(load_options->max_pixels, ==, 0);
    munit_assert_uint(load_options->max_frames, ==, 0);

    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

static MunitResult test_copy_options(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);

    load_options->options    = SAIL_OPTION_ICCP;
    load_options->max_width  = 8192;
    load_options->max_height = 4096;
    load_options->max_pixels = 10000000000ULL;
    load_options->max_frames = 100;

    struct sail_load_options* load_options_copy = NULL;
    munit_assert(sail_copy_load_options(load_options, &load_options_copy) == SAIL_OK);
    munit_assert_not_null(load_options_copy);

    munit_assert(load_options_copy->options == load_options->options);
    munit_assert_null(load_options_copy->tuning);
    munit_assert_uint(load_options_copy->max_width, ==, 8192);
    munit_assert_uint(load_options_copy->max_height, ==, 4096);
    munit_assert_uint64(load_options_copy->max_pixels, ==, 10000000000ULL);
    munit_assert_uint(load_options_copy->max_frames, ==, 100);

    sail_destroy_load_options(load_options_copy);
    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

static MunitResult test_options_from_features(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_load_options* load_options;
    struct sail_load_features load_features;
    load_features.features = SAIL_CODEC_FEATURE_META_DATA | SAIL_CODEC_FEATURE_INTERLACED | SAIL_CODEC_FEATURE_ICCP
                             | SAIL_CODEC_FEATURE_SOURCE_IMAGE;
    munit_assert(sail_alloc_load_options_from_features(&load_features, &load_options) == SAIL_OK);

    munit_assert(load_options->options == (SAIL_OPTION_META_DATA | SAIL_OPTION_ICCP | SAIL_OPTION_SOURCE_IMAGE));
    munit_assert_null(load_options->tuning);
    munit_assert_uint(load_options->max_width, ==, 0);
    munit_assert_uint(load_options->max_height, ==, 0);
    munit_assert_uint64(load_options->max_pixels, ==, 0);
    munit_assert_uint(load_options->max_frames, ==, 0);

    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

static MunitResult test_check_image_dimensions_allowed(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* No load options means no limits. */
    munit_assert(sail_check_image_dimensions_allowed(NULL, 100000, 100000) == SAIL_OK);

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);

    /* 0 means no limit. */
    munit_assert(sail_check_image_dimensions_allowed(load_options, 100000, 100000) == SAIL_OK);

    load_options->max_width  = 8192;
    load_options->max_height = 4096;

    munit_assert(sail_check_image_dimensions_allowed(load_options, 1, 1) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 4096) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8193, 4096) == SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 4097) == SAIL_ERROR_IMAGE_TOO_LARGE);

    /* A zero dimension is not known yet, so only the other dimension gets checked. */
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 0) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8193, 0) == SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 0, 4096) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 0, 4097) == SAIL_ERROR_IMAGE_TOO_LARGE);

    /* Limit only the width. */
    load_options->max_height = 0;

    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 100000) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8193, 1) == SAIL_ERROR_IMAGE_TOO_LARGE);

    /* Limit only the height. */
    load_options->max_width  = 0;
    load_options->max_height = 4096;

    munit_assert(sail_check_image_dimensions_allowed(load_options, 100000, 4096) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 1, 4097) == SAIL_ERROR_IMAGE_TOO_LARGE);

    /* Limit only the number of pixels. */
    load_options->max_height = 0;
    load_options->max_pixels = 1000000;

    munit_assert(sail_check_image_dimensions_allowed(load_options, 1000, 1000) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 1000000, 1) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 1001, 1000) == SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 1000001, 1) == SAIL_ERROR_IMAGE_TOO_LARGE);

    /* The number of pixels is not known while any dimension is not known. */
    munit_assert(sail_check_image_dimensions_allowed(load_options, 100000, 0) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 0, 100000) == SAIL_OK);

    /* The number of pixels must not overflow. */
    munit_assert(sail_check_image_dimensions_allowed(load_options, UINT_MAX, UINT_MAX) == SAIL_ERROR_IMAGE_TOO_LARGE);

    load_options->max_pixels = (uint64_t)UINT_MAX * UINT_MAX;
    munit_assert(sail_check_image_dimensions_allowed(load_options, UINT_MAX, UINT_MAX) == SAIL_OK);

    /* All the limits together. */
    load_options->max_width  = 8192;
    load_options->max_height = 8192;
    load_options->max_pixels = 4096 * 4096;

    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 2048) == SAIL_OK);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8192, 2049) == SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert(sail_check_image_dimensions_allowed(load_options, 8193, 1) == SAIL_ERROR_IMAGE_TOO_LARGE);

    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/alloc", test_alloc_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy", test_copy_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-features", test_options_from_features, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/check-image-dimensions-allowed", test_check_image_dimensions_allowed, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/load-options", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
