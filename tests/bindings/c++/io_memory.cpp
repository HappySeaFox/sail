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

#include <cstring>
#include <string>
#include <vector>

#include <sail-c++/sail-c++.h>

#include "munit.h"

static MunitResult test_io_memory_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_data = "Test data for reading";

    /* Open for reading */
    sail::io_memory io(test_data.data(), test_data.size());

    /* Read data */
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);
    munit_assert(read_size == test_data.size());
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);

    return MUNIT_OK;
}

static MunitResult test_io_memory_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_data = "Test data for writing";
    std::vector<char> buffer(256, 0);

    /* Open for writing */
    sail::io_memory io(buffer.data(), buffer.size());

    /* Write data */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);
    munit_assert(written_size == test_data.size());

    /* Verify buffer contents */
    munit_assert(std::memcmp(buffer.data(), test_data.data(), test_data.size()) == 0);

    return MUNIT_OK;
}

static MunitResult test_io_memory_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_data = "0123456789ABCDEF";

    /* Open for reading */
    sail::io_memory io(test_data.data(), test_data.size());

    /* Tell at start */
    std::size_t offset;
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 0);

    /* Seek to middle */
    munit_assert(io.seek(5, SEEK_SET) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 5);

    /* Read from middle */
    std::vector<char> read_buffer(5);
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), 5, &read_size) == SAIL_OK);
    munit_assert(std::memcmp(read_buffer.data(), "56789", 5) == 0);

    /* Seek from current */
    munit_assert(io.seek(-3, SEEK_CUR) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 7);

    /* Seek from end */
    munit_assert(io.seek(-2, SEEK_END) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == test_data.size() - 2);

    return MUNIT_OK;
}

static MunitResult test_io_memory_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_data = "EOF";

    /* Open for reading */
    sail::io_memory io(test_data.data(), test_data.size());

    /* Not at EOF initially */
    bool eof_result;
    munit_assert(io.eof(&eof_result) == SAIL_OK);
    munit_assert(eof_result == false);

    /* Read all data */
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);

    /* At EOF now */
    munit_assert(io.eof(&eof_result) == SAIL_OK);
    munit_assert(eof_result == true);

    return MUNIT_OK;
}

static MunitResult test_io_memory_read_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    std::vector<char> buffer(256, 0);
    const std::string test_data = "Read-write test";

    /* Open for read-write */
    sail::io_memory io(buffer.data(), buffer.size());

    /* Write data */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);
    munit_assert(written_size == test_data.size());

    /* Seek to beginning */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);

    /* Read back */
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);
    munit_assert(read_size == test_data.size());
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);

    return MUNIT_OK;
}

/* Test loading from empty bytes */
static MunitResult test_io_memory_image_input_null_bytes(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    auto* empty_data = "";
    sail::image_input input(empty_data, 0);

    sail::image img = input.next_frame();
    munit_assert_false(img.is_valid());

    return MUNIT_OK;
}

/* Test loading from invalid image data */
static MunitResult test_io_memory_image_input_invalid_bytes(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    std::string invalid_data = "not an image";
    sail::image_input input(invalid_data.c_str(), invalid_data.length());

    sail::image img = input.next_frame();
    munit_assert_false(img.is_valid());

    return MUNIT_OK;
}

/* Test loading truncated image data */
static MunitResult test_io_memory_image_input_truncated_data(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    // Valid PNG header but truncated
    std::string png_header = "\x89PNG\r\n\x1a\n";
    sail::image_input input(png_header.c_str(), png_header.length());

    sail::image img = input.next_frame();
    munit_assert_false(img.is_valid());

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/read",                       test_io_memory_read,                       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/write",                      test_io_memory_write,                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/seek-tell",                  test_io_memory_seek_tell,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/eof",                        test_io_memory_eof,                        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/read-write",                 test_io_memory_read_write,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/image-input-null-bytes",     test_io_memory_image_input_null_bytes,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/image-input-invalid-bytes",  test_io_memory_image_input_invalid_bytes,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/image-input-truncated-data", test_io_memory_image_input_truncated_data, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/io-memory", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
