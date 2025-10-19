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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sail/sail.h>

#include "munit.h"

static MunitResult test_io_file_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    char* test_file = NULL;
    munit_assert(sail_temp_file_path("sail_io_file_test_read", &test_file) == SAIL_OK);

    const char* test_data       = "Test data for reading";
    const size_t test_data_size = strlen(test_data);

    /* Create test file */
    FILE* f = fopen(test_file, "wb");
    munit_assert_not_null(f);
    munit_assert(fwrite(test_data, 1, test_data_size, f) == test_data_size);
    fclose(f);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_file(test_file, &io) == SAIL_OK);
    munit_assert_not_null(io);

    /* Read data */
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);

    sail_destroy_io(io);
    remove(test_file);
    sail_free(test_file);

    return MUNIT_OK;
}

static MunitResult test_io_file_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    char* test_file = NULL;
    munit_assert(sail_temp_file_path("sail_io_file_test_write", &test_file) == SAIL_OK);

    const char* test_data       = "Test data for writing";
    const size_t test_data_size = strlen(test_data);

    /* Open for writing */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_write_file(test_file, &io) == SAIL_OK);
    munit_assert_not_null(io);

    /* Write data */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);
    munit_assert(written_size == test_data_size);

    /* Flush */
    munit_assert(io->flush(io->stream) == SAIL_OK);

    sail_destroy_io(io);

    /* Verify file contents */
    FILE* f = fopen(test_file, "rb");
    munit_assert_not_null(f);
    char read_buffer[256];
    size_t read_size = fread(read_buffer, 1, test_data_size, f);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);
    fclose(f);

    remove(test_file);
    sail_free(test_file);

    return MUNIT_OK;
}

static MunitResult test_io_file_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    char* test_file = NULL;
    munit_assert(sail_temp_file_path("sail_io_file_test_seek", &test_file) == SAIL_OK);

    const char* test_data       = "0123456789ABCDEF";
    const size_t test_data_size = strlen(test_data);

    /* Create test file */
    FILE* f = fopen(test_file, "wb");
    munit_assert_not_null(f);
    fwrite(test_data, 1, test_data_size, f);
    fclose(f);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_file(test_file, &io) == SAIL_OK);

    /* Tell at start */
    size_t offset;
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 0);

    /* Seek to middle */
    munit_assert(io->seek(io->stream, 5, SEEK_SET) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 5);

    /* Read from middle */
    char read_buffer[5];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, 5, &read_size) == SAIL_OK);
    munit_assert(memcmp(read_buffer, "56789", 5) == 0);

    /* Seek from current */
    munit_assert(io->seek(io->stream, -3, SEEK_CUR) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 7);

    sail_destroy_io(io);
    remove(test_file);
    sail_free(test_file);

    return MUNIT_OK;
}

static MunitResult test_io_file_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    char* test_file = NULL;
    munit_assert(sail_temp_file_path("sail_io_file_test_eof", &test_file) == SAIL_OK);

    const char* test_data = "EOF";

    /* Create test file */
    FILE* f = fopen(test_file, "wb");
    munit_assert_not_null(f);
    fwrite(test_data, 1, strlen(test_data), f);
    fclose(f);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_file(test_file, &io) == SAIL_OK);

    /* Not at EOF initially */
    bool eof_result;
    munit_assert(io->eof(io->stream, &eof_result) == SAIL_OK);
    munit_assert(eof_result == false);

    /* Read all data */
    char read_buffer[10];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, strlen(test_data), &read_size) == SAIL_OK);

    /* At EOF now */
    munit_assert(io->eof(io->stream, &eof_result) == SAIL_OK);
    munit_assert(eof_result == true);

    sail_destroy_io(io);
    remove(test_file);
    sail_free(test_file);

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
