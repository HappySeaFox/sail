/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

static MunitResult test_io_compute_seek_position(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    size_t new_position;

    munit_assert_int(sail_io_compute_seek_position(100u, 50L, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, 150u);

    munit_assert_int(sail_io_compute_seek_position(100u, 0L, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, 100u);

    munit_assert_int(sail_io_compute_seek_position(100u, -30L, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, 70u);

    munit_assert_int(sail_io_compute_seek_position(100u, -100L, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, 0u);

    munit_assert_int(sail_io_compute_seek_position(10u, -11L, &new_position), ==, SAIL_ERROR_SEEK_IO);
    munit_assert_int(sail_io_compute_seek_position(0u, -1L, &new_position), ==, SAIL_ERROR_SEEK_IO);

    munit_assert_int(sail_io_compute_seek_position(SIZE_MAX, 1L, &new_position), ==, SAIL_ERROR_SEEK_IO);

    munit_assert_int(sail_io_compute_seek_position(SIZE_MAX - 5u, 5L, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, SIZE_MAX);

    munit_assert_int(sail_io_compute_seek_position(SIZE_MAX - 5u, 6L, &new_position), ==, SAIL_ERROR_SEEK_IO);

    const size_t long_min_magnitude = (size_t)LONG_MAX + 1u;

    munit_assert_int(sail_io_compute_seek_position(long_min_magnitude, LONG_MIN, &new_position), ==, SAIL_OK);
    munit_assert_size(new_position, ==, 0u);

    munit_assert_int(sail_io_compute_seek_position((size_t)LONG_MAX, LONG_MIN, &new_position), ==, SAIL_ERROR_SEEK_IO);
    munit_assert_int(sail_io_compute_seek_position(0u, LONG_MIN, &new_position), ==, SAIL_ERROR_SEEK_IO);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/compute-seek-position", test_io_compute_seek_position, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-common", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
