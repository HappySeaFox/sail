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

static MunitResult test_io_memory_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* test_data       = "Test data for reading";
    const size_t test_data_size = strlen(test_data);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_memory(test_data, test_data_size, &io) == SAIL_OK);
    munit_assert_not_null(io);

    /* Read data */
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_io_memory_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* test_data       = "Test data for writing";
    const size_t test_data_size = strlen(test_data);
    char buffer[256]            = {0};

    /* Open for writing */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_write_memory(buffer, sizeof(buffer), &io) == SAIL_OK);
    munit_assert_not_null(io);

    /* Write data */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);
    munit_assert(written_size == test_data_size);

    sail_destroy_io(io);

    /* Verify buffer contents */
    munit_assert(memcmp(buffer, test_data, test_data_size) == 0);

    return MUNIT_OK;
}

static MunitResult test_io_memory_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* test_data       = "0123456789ABCDEF";
    const size_t test_data_size = strlen(test_data);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_memory(test_data, test_data_size, &io) == SAIL_OK);

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

    /* Seek from end */
    munit_assert(io->seek(io->stream, -2, SEEK_END) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == test_data_size - 2);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_io_memory_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* test_data       = "EOF";
    const size_t test_data_size = strlen(test_data);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_memory(test_data, test_data_size, &io) == SAIL_OK);

    /* Not at EOF initially */
    bool eof_result;
    munit_assert(io->eof(io->stream, &eof_result) == SAIL_OK);
    munit_assert(eof_result == false);

    /* Read all data */
    char read_buffer[10];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);

    /* At EOF now */
    munit_assert(io->eof(io->stream, &eof_result) == SAIL_OK);
    munit_assert(eof_result == true);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_io_memory_read_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    char buffer[256]            = {0};
    const char* test_data       = "Read-write test";
    const size_t test_data_size = strlen(test_data);

    /* Open for read-write */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_write_memory(buffer, sizeof(buffer), &io) == SAIL_OK);

    /* Write data */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);
    munit_assert(written_size == test_data_size);

    /* Seek to beginning */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);

    /* Read back */
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_io_memory_size(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* test_data       = "Test data for size";
    const size_t test_data_size = strlen(test_data);

    /* Open for reading */
    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_read_memory(test_data, test_data_size, &io) == SAIL_OK);

    /* Check size callback is set */
    munit_assert(io->size != NULL);

    /* Get size using sail_io_size */
    size_t size;
    munit_assert(sail_io_size(io, &size) == SAIL_OK);
    munit_assert(size == test_data_size);

    /* Get size using callback directly */
    size_t size_direct;
    munit_assert(io->size(io->stream, &size_direct) == SAIL_OK);
    munit_assert(size_direct == test_data_size);

    /* Size should remain the same after seeking */
    munit_assert(io->seek(io->stream, 5, SEEK_SET) == SAIL_OK);
    size_t size_after_seek;
    munit_assert(sail_io_size(io, &size_after_seek) == SAIL_OK);
    munit_assert(size_after_seek == test_data_size);

    sail_destroy_io(io);

    /* Test read-write memory buffer */
    char buffer[256] = {0};
    const size_t buffer_size = sizeof(buffer);
    munit_assert(sail_alloc_io_read_write_memory(buffer, buffer_size, &io) == SAIL_OK);

    /* Size should be buffer size, not written data size */
    size_t buffer_size_result;
    munit_assert(sail_io_size(io, &buffer_size_result) == SAIL_OK);
    munit_assert(buffer_size_result == buffer_size);

    sail_destroy_io(io);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/read",       test_io_memory_read,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/write",      test_io_memory_write,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/seek-tell",  test_io_memory_seek_tell,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/eof",        test_io_memory_eof,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/read-write", test_io_memory_read_write, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/size",       test_io_memory_size,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-memory", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
