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
#include <fstream>
#include <string>

#include <sail-c++/sail-c++.h>

#include "munit.h"

static MunitResult test_io_file_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_file = sail::temp_file_path("sail_io_file_cpp_test_read");
    const std::string test_data = "Test data for reading";

    /* Create test file */
    std::ofstream f(test_file, std::ios::binary);
    munit_assert(f.is_open());
    f.write(test_data.data(), test_data.size());
    f.close();

    /* Open for reading */
    sail::io_file io(test_file);

    /* Read data */
    std::vector<char> read_buffer(test_data.size());
    std::size_t read_size;
    munit_assert(io.tolerant_read(read_buffer.data(), test_data.size(), &read_size) == SAIL_OK);
    munit_assert(read_size == test_data.size());
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);

    std::remove(test_file.c_str());

    return MUNIT_OK;
}

static MunitResult test_io_file_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_file = sail::temp_file_path("sail_io_file_cpp_test_write");
    const std::string test_data = "Test data for writing";

    /* Open for writing */
    sail::io_file io(test_file, sail::io_file::Operation::ReadWrite);

    /* Write data */
    std::size_t written_size;
    munit_assert(io.tolerant_write(test_data.data(), test_data.size(), &written_size) == SAIL_OK);
    munit_assert(written_size == test_data.size());

    /* Flush */
    munit_assert(io.flush() == SAIL_OK);

    /* Will be closed by the destructor as well, test double closing */
    io.close();

    /* Verify file contents */
    std::ifstream f(test_file, std::ios::binary);
    munit_assert(f.is_open());
    std::vector<char> read_buffer(test_data.size());
    f.read(read_buffer.data(), test_data.size());
    munit_assert(f.gcount() == static_cast<std::streamsize>(test_data.size()));
    munit_assert(std::memcmp(read_buffer.data(), test_data.data(), test_data.size()) == 0);
    f.close();

    std::remove(test_file.c_str());

    return MUNIT_OK;
}

static MunitResult test_io_file_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_file = sail::temp_file_path("sail_io_file_cpp_test_seek");
    const std::string test_data = "0123456789ABCDEF";

    /* Create test file */
    std::ofstream f(test_file, std::ios::binary);
    f.write(test_data.data(), test_data.size());
    f.close();

    /* Open for reading */
    sail::io_file io(test_file);

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

    std::remove(test_file.c_str());

    return MUNIT_OK;
}

static MunitResult test_io_file_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const std::string test_file = sail::temp_file_path("sail_io_file_cpp_test_eof");
    const std::string test_data = "EOF";

    /* Create test file */
    std::ofstream f(test_file, std::ios::binary);
    f.write(test_data.data(), test_data.size());
    f.close();

    /* Open for reading */
    sail::io_file io(test_file);

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

    std::remove(test_file.c_str());

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/read",      test_io_file_read,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/write",     test_io_file_write,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/seek-tell", test_io_file_seek_tell, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/eof",       test_io_file_eof,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-file", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
