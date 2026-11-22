/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <sail-c++/suppress_begin.h>
#include <sail-c++/suppress_c4251.h>

#include <sail-c++/abstract_io.h>
#include <sail-c++/arbitrary_data.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/image.h>
#include <sail-c++/image_input.h>
#include <sail-c++/io_base_private.h>
#include <sail-c++/io_file.h>
#include <sail-c++/io_memory.h>
#include <sail-c++/load_options.h>
#include <sail-c++/utils.h>

#include <sail-c++/suppress_end.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

static MunitResult test_can_load_path(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const sail::image image(path);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_memory(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    sail::image_input input(arbitrary_data);
    input.with(codec_info);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_file(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::io_file io_file(path);
    sail::image_input input(io_file);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory1(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    sail::io_memory io_memory(arbitrary_data.data(), arbitrary_data.size());
    sail::image_input input(io_memory);
    input.with(codec_info);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory2(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    sail::io_memory io_memory(arbitrary_data);
    sail::image_input input(io_memory);
    input.with(codec_info);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory3(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    sail::io_memory io_memory(arbitrary_data.data(), arbitrary_data.size(), sail::io_memory::Operation::Read);
    sail::image_input input(io_memory);
    input.with(codec_info);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory4(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    sail::io_memory io_memory(arbitrary_data, sail::io_memory::Operation::Read);
    sail::image_input input = std::move(sail::image_input(io_memory).with(codec_info));
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory5(const MunitParameter params[], void* user_data)
{

    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    std::unique_ptr<sail::abstract_io> io_memory{new sail::io_memory(arbitrary_data, sail::io_memory::Operation::Read)};
    sail::image_input input = std::move(sail::image_input(*io_memory).with(codec_info));
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_probe_then_load(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::image_input input(path);

    const auto probe_result           = input.probe();
    const sail::image probed          = std::get<0>(probe_result);
    const sail::codec_info codec_info = std::get<1>(probe_result);

    /* Some acceptance images cannot be probed (for example unknown codecs). */
    if (!codec_info.is_valid())
    {
        return MUNIT_SKIP;
    }

    munit_assert(probed.width() > 0);
    munit_assert(probed.height() > 0);
    munit_assert_false(probed.is_valid());

    const unsigned probed_width  = probed.width();
    const unsigned probed_height = probed.height();

    sail::image image;
    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());
    munit_assert(image.width() == probed_width);
    munit_assert(image.height() == probed_height);

    return MUNIT_OK;
}

static MunitResult test_can_probe_memory_then_load(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::arbitrary_data contents;
    munit_assert(sail::read_file_contents(path, &contents) == SAIL_OK);

    sail::image_input input(contents);

    const auto probe_result           = input.probe();
    const sail::image probed          = std::get<0>(probe_result);
    const sail::codec_info codec_info = std::get<1>(probe_result);

    /* Memory probe uses magic numbers, so path only formats such as VIDEO are skipped. */
    if (!codec_info.is_valid())
    {
        return MUNIT_SKIP;
    }

    munit_assert(probed.width() > 0);
    munit_assert(probed.height() > 0);
    munit_assert_false(probed.is_valid());

    const unsigned probed_width  = probed.width();
    const unsigned probed_height = probed.height();

    sail::image image;
    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());
    munit_assert(image.width() == probed_width);
    munit_assert(image.height() == probed_height);

    return MUNIT_OK;
}

/* Probes the image with the specified dimension limits. */
static bool probe_with_dimension_limits(const char* path,
                                        unsigned max_width,
                                        unsigned max_height,
                                        std::uint64_t max_pixels = 0)
{
    sail::load_options load_options;
    load_options.set_max_dimensions(max_width, max_height);
    load_options.set_max_pixels(max_pixels);

    sail::image_input input(path);
    input.with(load_options);

    return std::get<1>(input.probe()).is_valid();
}

/* Loads the first frame of the image with the specified dimension limits. */
static sail_status_t load_with_dimension_limits(const char* path,
                                                unsigned max_width,
                                                unsigned max_height,
                                                std::uint64_t max_pixels = 0)
{
    sail::load_options load_options;
    load_options.set_max_dimensions(max_width, max_height);
    load_options.set_max_pixels(max_pixels);

    sail::image_input input(path);
    input.with(load_options);

    sail::image image;
    return input.next_frame(&image);
}

static MunitResult test_dimension_limits(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    sail::image_input input(path);
    const auto probe_result = input.probe();

    /* Some acceptance images cannot be probed (for example unknown codecs). */
    if (!std::get<1>(probe_result).is_valid())
    {
        return MUNIT_SKIP;
    }

    const unsigned width  = std::get<0>(probe_result).width();
    const unsigned height = std::get<0>(probe_result).height();

    const std::uint64_t pixels = static_cast<std::uint64_t>(width) * height;

    munit_assert_true(probe_with_dimension_limits(path, width, height, pixels));
    munit_assert_int(load_with_dimension_limits(path, width, height, pixels), ==, SAIL_OK);

    /* 0 means no limit, so 1 pixel wide or tall images cannot exceed a limit. */
    if (width > 1)
    {
        munit_assert_false(probe_with_dimension_limits(path, width - 1, 0));
        munit_assert_int(load_with_dimension_limits(path, width - 1, 0), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }
    if (height > 1)
    {
        munit_assert_false(probe_with_dimension_limits(path, 0, height - 1));
        munit_assert_int(load_with_dimension_limits(path, 0, height - 1), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }
    if (pixels > 1)
    {
        munit_assert_false(probe_with_dimension_limits(path, 0, 0, pixels - 1));
        munit_assert_int(load_with_dimension_limits(path, 0, 0, pixels - 1), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }

    return MUNIT_OK;
}

static MunitParameterEnum test_params[] = {
    {(char*)"path", (char**)SAIL_TEST_IMAGES},
    {NULL, NULL},
};

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/can-load-path",              test_can_load_path,              NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-memory",            test_can_load_memory,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-file",           test_can_load_io_file,           NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory1",        test_can_load_io_memory1,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory2",        test_can_load_io_memory2,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory3",        test_can_load_io_memory3,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory4",        test_can_load_io_memory4,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory5",        test_can_load_io_memory5,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-probe-then-load",        test_can_probe_then_load,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-probe-memory-then-load", test_can_probe_memory_then_load, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/dimension-limits",           test_dimension_limits,           NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
