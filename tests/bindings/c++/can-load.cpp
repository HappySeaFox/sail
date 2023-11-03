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

#include <sail-c++/suppress_c4251_begin.h>

#include <sail-c++/abstract_io.h>
#include <sail-c++/arbitrary_data.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/image.h>
#include <sail-c++/image_input.h>
#include <sail-c++/io_base_private.h>
#include <sail-c++/io_file.h>
#include <sail-c++/io_memory.h>
#include <sail-c++/utils.h>

#include <sail-c++/suppress_end.h>

#include "munit.h"

#include "test-images.h"

static MunitResult test_can_load_path(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

    const sail::image image(path);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_memory(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

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

static MunitResult test_can_load_io_file(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

    sail::io_file io_file(path);
    sail::image_input input(io_file);
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitResult test_can_load_io_memory1(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

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

static MunitResult test_can_load_io_memory2(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

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

static MunitResult test_can_load_io_memory3(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

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

static MunitResult test_can_load_io_memory4(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

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

static MunitResult test_can_load_io_memory5(const MunitParameter params[], void *user_data) {

    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

    sail::arbitrary_data arbitrary_data;
    munit_assert(sail::read_file_contents(path, &arbitrary_data) == SAIL_OK);

    const sail::codec_info codec_info = sail::codec_info::from_path(path);
    munit_assert(codec_info.is_valid());

    std::unique_ptr<sail::abstract_io> io_memory { new sail::io_memory(arbitrary_data, sail::io_memory::Operation::Read) };
    sail::image_input input = std::move(sail::image_input(*io_memory).with(codec_info));
    sail::image image;

    munit_assert(input.next_frame(&image) == SAIL_OK);
    munit_assert(image.is_valid());

    return MUNIT_OK;
}

static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/can-load-path",       test_can_load_path,       NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-memory",     test_can_load_memory,     NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-file",    test_can_load_io_file,    NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory1", test_can_load_io_memory1, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory2", test_can_load_io_memory2, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory3", test_can_load_io_memory3, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory4", test_can_load_io_memory4, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/can-load-io-memory5", test_can_load_io_memory5, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
