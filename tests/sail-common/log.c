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

#include <sail-common/sail-common.h>

#include "munit.h"

static MunitResult test_log_level_from_string_valid(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(sail_log_level_from_string("silence"), ==, SAIL_LOG_LEVEL_SILENCE);
    munit_assert_int(sail_log_level_from_string("error"), ==, SAIL_LOG_LEVEL_ERROR);
    munit_assert_int(sail_log_level_from_string("warning"), ==, SAIL_LOG_LEVEL_WARNING);
    munit_assert_int(sail_log_level_from_string("info"), ==, SAIL_LOG_LEVEL_INFO);
    munit_assert_int(sail_log_level_from_string("message"), ==, SAIL_LOG_LEVEL_MESSAGE);
    munit_assert_int(sail_log_level_from_string("debug"), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string("trace"), ==, SAIL_LOG_LEVEL_TRACE);

    return MUNIT_OK;
}

static MunitResult test_log_level_from_string_invalid(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(sail_log_level_from_string(NULL), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string(""), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string("invalid"), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string("SILENCE"), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string("Error"), ==, SAIL_LOG_LEVEL_DEBUG);
    munit_assert_int(sail_log_level_from_string("WARNING"), ==, SAIL_LOG_LEVEL_DEBUG);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/log-level-from-string-valid", test_log_level_from_string_valid, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/log-level-from-string-invalid", test_log_level_from_string_invalid, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/log", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}

