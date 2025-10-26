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

#include <sail-c++/sail-c++.h>

#include "munit.h"

static MunitResult test_expanding_buffer_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 1024;
    const std::string test_data        = "Hello, C++ expanding buffer!";

    sail::io_expanding_buffer io(initial_capacity);

    /* Write test data. */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);
    munit_assert(written_size == test_data.size());

    /* Check size. */
    munit_assert(io.size() == test_data.size());

    /* Verify data by reading back. */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);
    munit_assert(read_size == test_data.size());
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_multiple_writes(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 64;
    sail::io_expanding_buffer io(initial_capacity);

    /* Write multiple chunks. */
    const std::string chunk1 = "First chunk. ";
    const std::string chunk2 = "Second chunk. ";
    const std::string chunk3 = "Third chunk. ";

    std::size_t written_size;
    munit_assert(io.tolerant_write(chunk1.data(), chunk1.size(), &written_size) == SAIL_OK);
    munit_assert(io.tolerant_write(chunk2.data(), chunk2.size(), &written_size) == SAIL_OK);
    munit_assert(io.tolerant_write(chunk3.data(), chunk3.size(), &written_size) == SAIL_OK);

    /* Check size. */
    const std::size_t expected_size = chunk1.size() + chunk2.size() + chunk3.size();
    munit_assert(io.size() == expected_size);

    /* Verify data by reading back. */
    const std::string expected = chunk1 + chunk2 + chunk3;
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);
    std::vector<char> read_buffer(expected_size);
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), expected_size, &read_size) == SAIL_OK);
    munit_assert(read_size == expected_size);
    munit_assert(std::memcmp(read_buffer.data(), expected.data(), expected_size) == 0);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_expansion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 16;
    const std::size_t large_size       = 1024;

    sail::io_expanding_buffer io(initial_capacity);

    /* Write data larger than initial capacity. */
    std::vector<char> large_data(large_size, 'X');

    std::size_t written_size;
    munit_assert(io.tolerant_write(large_data.data(), large_size, &written_size) == SAIL_OK);
    munit_assert(written_size == large_size);

    /* Check size. */
    munit_assert(io.size() == large_size);

    /* Verify data by reading back. */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);
    std::vector<char> read_buffer(large_size);
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), large_size, &read_size) == SAIL_OK);
    munit_assert(read_size == large_size);
    munit_assert(std::memcmp(read_buffer.data(), large_data.data(), large_size) == 0);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_flush(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 1024;
    const std::string test_data        = "Flush test!";

    sail::io_expanding_buffer io(initial_capacity);

    /* Write test data. */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);

    /* Flush should succeed (even though it's a no-op for memory buffers). */
    munit_assert(io.flush() == SAIL_OK);

    /* Verify data is still there. */
    munit_assert(io.size() == test_data.size());

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 1024;
    const std::string test_data        = "Read this back!";

    sail::io_expanding_buffer io(initial_capacity);

    /* Write test data. */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);

    /* Seek to beginning. */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);

    /* Read back. */
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);
    munit_assert(read_size == test_data.size());
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 1024;
    sail::io_expanding_buffer io(initial_capacity);

    /* Write some data. */
    const std::string test_data = "0123456789";
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);

    /* Tell. */
    std::size_t offset;
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == test_data.size());

    /* Seek to beginning. */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 0);

    /* Seek to middle. */
    munit_assert(io.seek(5, SEEK_SET) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 5);

    /* Seek from current. */
    munit_assert(io.seek(2, SEEK_CUR) == SAIL_OK);
    munit_assert(io.tell(&offset) == SAIL_OK);
    munit_assert(offset == 7);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::size_t initial_capacity = 1024;
    const std::string test_data        = "EOF test";

    sail::io_expanding_buffer io(initial_capacity);

    /* Write test data. */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);

    /* Check EOF at end. */
    bool eof_result;
    munit_assert(io.eof(&eof_result) == SAIL_OK);
    munit_assert(eof_result == true);

    /* Seek to beginning. */
    munit_assert(io.seek(0, SEEK_SET) == SAIL_OK);
    munit_assert(io.eof(&eof_result) == SAIL_OK);
    munit_assert(eof_result == false);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/write",           test_expanding_buffer_write,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/multiple-writes", test_expanding_buffer_multiple_writes, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/expansion",       test_expanding_buffer_expansion,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/flush",           test_expanding_buffer_flush,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/read",            test_expanding_buffer_read,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/seek-tell",       test_expanding_buffer_seek_tell,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/eof",             test_expanding_buffer_eof,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++/io-expanding-buffer", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
