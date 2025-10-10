/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <sail/sail.h>

#include "munit.h"

#include "tests/images/bugs/test-images.h"

static MunitResult test_bugs_must_fail(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

static MunitResult test_bugs_must_succeed(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);

    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params_must_fail[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES_MUST_FAIL },
    { NULL, NULL },
};

static MunitParameterEnum test_params_must_succeed[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES_MUST_SUCCEED },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/must-fail",    test_bugs_must_fail,    NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params_must_fail },
    { (char *)"/must-succeed", test_bugs_must_succeed, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params_must_succeed },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bugs", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
