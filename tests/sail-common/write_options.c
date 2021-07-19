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

#include "sail-common.h"

#include "munit.h"

static MunitResult test_alloc_options(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_write_options *write_options = NULL;
    munit_assert(sail_alloc_write_options(&write_options) == SAIL_OK);
    munit_assert_not_null(write_options);
    munit_assert(write_options->io_options == 0);
    munit_assert(write_options->compression == SAIL_COMPRESSION_UNSUPPORTED);
    munit_assert(write_options->compression_level == 0);

    sail_destroy_write_options(write_options);

    return MUNIT_OK;
}

static MunitResult test_copy_options(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_write_options *write_options = NULL;
    munit_assert(sail_alloc_write_options(&write_options) == SAIL_OK);

    write_options->io_options        = SAIL_IO_OPTION_ICCP;
    write_options->compression       = SAIL_COMPRESSION_JPEG;
    write_options->compression_level = 55;

    struct sail_write_options *write_options_copy = NULL;
    munit_assert(sail_copy_write_options(write_options, &write_options_copy) == SAIL_OK);
    munit_assert_not_null(write_options_copy);

    munit_assert(write_options_copy->io_options == write_options->io_options);
    munit_assert(write_options_copy->compression == write_options->compression);
    munit_assert(write_options_copy->compression_level == write_options->compression_level);

    sail_destroy_write_options(write_options_copy);
    sail_destroy_write_options(write_options);

    return MUNIT_OK;
}

static MunitResult test_options_from_features(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_write_options *write_options = NULL;
    munit_assert(sail_alloc_write_options(&write_options) == SAIL_OK);
    munit_assert_not_null(write_options);

    struct sail_write_features write_features;
    write_features.default_compression = SAIL_COMPRESSION_JPEG;
    write_features.compression_level_min = 1;
    write_features.compression_level_max = 100;
    write_features.compression_level_default = 81;
    write_features.compression_level_step = 1;
    write_features.features = SAIL_CODEC_FEATURE_META_DATA | SAIL_CODEC_FEATURE_INTERLACED | SAIL_CODEC_FEATURE_ICCP;
    munit_assert(sail_write_options_from_features(&write_features, write_options) == SAIL_OK);

    munit_assert(write_options->io_options == (SAIL_IO_OPTION_META_DATA | SAIL_IO_OPTION_INTERLACED | SAIL_IO_OPTION_ICCP));
    munit_assert(write_options->compression ==  write_features.default_compression);
    munit_assert(write_options->compression_level ==  write_features.compression_level_default);

    sail_destroy_write_options(write_options);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc", test_alloc_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy", test_copy_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-features", test_options_from_features, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/write-options",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
