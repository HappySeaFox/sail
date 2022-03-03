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

#include "sail-comparators.h"

#include "munit.h"

#include "test-images.h"

static MunitResult test_io_produce_same_images(const MunitParameter params[], void *user_data) {
    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

    struct sail_image *image_file;
    munit_assert(sail_load_image_from_file(path, &image_file) == SAIL_OK);
    munit_assert_not_null(image_file);

    void *data;
    size_t data_length;
    munit_assert(sail_file_contents_to_data(path, &data, &data_length) == SAIL_OK);
    munit_assert_not_null(data);
    munit_assert(data_length > 0);

    struct sail_image *image_mem;
    munit_assert(sail_load_image_from_memory(data, data_length, &image_mem) == SAIL_OK);
    munit_assert_not_null(image_mem);

    munit_assert(sail_test_compare_images(image_file, image_mem) == SAIL_OK);

    sail_free(data);
    sail_destroy_image(image_mem);
    sail_destroy_image(image_file);

    return MUNIT_OK;
}

static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/io-produce-same-images", test_io_produce_same_images, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-produce-same-images",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
