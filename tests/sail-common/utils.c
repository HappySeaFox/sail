/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

static MunitResult test_reverse_uint16(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_uint16(sail_reverse_uint16(0), ==, 0);
    munit_assert_uint16(sail_reverse_uint16(100), ==, 25600);
    munit_assert_uint16(sail_reverse_uint16(1000), ==, 59395);

    return MUNIT_OK;
}

static MunitResult test_reverse_uint32(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_uint32(sail_reverse_uint32(0), ==, 0);
    munit_assert_uint32(sail_reverse_uint32(100), ==, 1677721600);
    munit_assert_uint32(sail_reverse_uint32(1000), ==, 3892510720);
    munit_assert_uint32(sail_reverse_uint32(100000), ==, 2693136640);

    return MUNIT_OK;
}

static MunitResult test_reverse_uint64(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_uint64(sail_reverse_uint64(0ull), ==, 0ull);
    munit_assert_uint64(sail_reverse_uint64(100ull), ==, 7205759403792793600ull);
    munit_assert_uint64(sail_reverse_uint64(1000ull), ==, 16718206241729413120ull);
    munit_assert_uint64(sail_reverse_uint64(100000ull), ==, 11566933792459325440ull);
    munit_assert_uint64(sail_reverse_uint64(10000000000ull), ==, 64188750128742400ull);

    return MUNIT_OK;
}

static MunitResult test_size_mul(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    size_t result;

    munit_assert_int(sail_size_mul(100u, 200u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 20000u);

    munit_assert_int(sail_size_mul(200u, 100u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 20000u);

    munit_assert_int(sail_size_mul(1u, 1u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 1u);

    munit_assert_int(sail_size_mul(0u, 100u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 0u);

    munit_assert_int(sail_size_mul(100u, 0u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 0u);

    munit_assert_int(sail_size_mul(0u, 0u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 0u);

    munit_assert_int(sail_size_mul(100u, 200u, NULL), ==, SAIL_ERROR_NULL_PTR);

#if SIZE_MAX > UINT32_MAX
    /* 64-bit size_t: product of two max unsigned values still fits. */
    munit_assert_int(sail_size_mul(UINT32_MAX, UINT32_MAX, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, (size_t)UINT32_MAX * (size_t)UINT32_MAX);
#else
    /* 32-bit size_t: 65536 * 65536 exceeds SIZE_MAX. */
    munit_assert_int(sail_size_mul(65536u, 65536u, &result), ==, SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
    munit_assert_int(sail_size_mul(100000u, 50000u, &result), ==, SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);

    munit_assert_int(sail_size_mul(65536u, 65535u, &result), ==, SAIL_OK);
    munit_assert_size(result, ==, 4294901760u);
#endif

    return MUNIT_OK;
}

static MunitResult test_pixels_buffer_size(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    size_t pixels_size;

    munit_assert_int(sail_pixels_buffer_size(100u, 300u, &pixels_size), ==, SAIL_OK);
    munit_assert_size(pixels_size, ==, 30000u);

    munit_assert_int(sail_pixels_buffer_size(0u, 300u, &pixels_size), ==, SAIL_OK);
    munit_assert_size(pixels_size, ==, 0u);

    munit_assert_int(sail_pixels_buffer_size(100u, 0u, &pixels_size), ==, SAIL_ERROR_INVALID_BYTES_PER_LINE);

    munit_assert_int(sail_pixels_buffer_size(100u, 300u, NULL), ==, SAIL_ERROR_NULL_PTR);

#if SIZE_MAX > UINT32_MAX
    munit_assert_int(sail_pixels_buffer_size(UINT32_MAX, UINT32_MAX, &pixels_size), ==, SAIL_OK);
    munit_assert_size(pixels_size, ==, (size_t)UINT32_MAX * (size_t)UINT32_MAX);
#else
    munit_assert_int(sail_pixels_buffer_size(65536u, 65536u, &pixels_size), ==, SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
    munit_assert_int(sail_pixels_buffer_size(100000u, 50000u, &pixels_size), ==, SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);

    munit_assert_int(sail_pixels_buffer_size(65536u, 65535u, &pixels_size), ==, SAIL_OK);
    munit_assert_size(pixels_size, ==, 4294901760u);
#endif

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/reverse-uint16",       test_reverse_uint16,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/reverse-uint32",       test_reverse_uint32,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/reverse-uint64",       test_reverse_uint64,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/size-mul",             test_size_mul,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pixels-buffer-size",   test_pixels_buffer_size,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/utils", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
