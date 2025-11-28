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

static MunitResult test_expanding_buffer_write(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    const char* test_data         = "Hello, expanding buffer!";
    const size_t test_data_size   = strlen(test_data);

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);
    munit_assert_not_null(io);

    /* Write test data. */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);
    munit_assert(written_size == test_data_size);

    /* Check size. */
    size_t buffer_size;
    munit_assert(sail_io_expanding_buffer_size(io, &buffer_size) == SAIL_OK);
    munit_assert(buffer_size == test_data_size);

    /* Verify data by reading back. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_multiple_writes(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 64;
    struct sail_io* io            = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write multiple chunks. */
    const char* chunk1 = "First chunk. ";
    const char* chunk2 = "Second chunk. ";
    const char* chunk3 = "Third chunk. ";

    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, chunk1, strlen(chunk1), &written_size) == SAIL_OK);
    munit_assert(io->tolerant_write(io->stream, chunk2, strlen(chunk2), &written_size) == SAIL_OK);
    munit_assert(io->tolerant_write(io->stream, chunk3, strlen(chunk3), &written_size) == SAIL_OK);

    /* Check size. */
    size_t buffer_size;
    munit_assert(sail_io_expanding_buffer_size(io, &buffer_size) == SAIL_OK);
    munit_assert(buffer_size == strlen(chunk1) + strlen(chunk2) + strlen(chunk3));

    /* Verify data by reading back. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, buffer_size, &read_size) == SAIL_OK);
    munit_assert(read_size == buffer_size);

    char expected[256];
    snprintf(expected, sizeof(expected), "%s%s%s", chunk1, chunk2, chunk3);
    munit_assert(memcmp(read_buffer, expected, buffer_size) == 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_expansion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 16;
    const size_t large_size       = 1024;

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write data larger than initial capacity. */
    char* large_data = sail_malloc_std_signature(large_size);
    munit_assert_not_null(large_data);
    memset(large_data, 'X', large_size);

    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, large_data, large_size, &written_size) == SAIL_OK);
    munit_assert(written_size == large_size);

    /* Check size. */
    size_t buffer_size;
    munit_assert(sail_io_expanding_buffer_size(io, &buffer_size) == SAIL_OK);
    munit_assert(buffer_size == large_size);

    /* Verify data by reading back. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    char* read_buffer = sail_malloc_std_signature(large_size);
    munit_assert_not_null(read_buffer);
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, large_size, &read_size) == SAIL_OK);
    munit_assert(read_size == large_size);
    munit_assert(memcmp(read_buffer, large_data, large_size) == 0);
    sail_free(read_buffer);

    sail_free(large_data);
    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_flush(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    const char* test_data         = "Flush test!";
    const size_t test_data_size   = strlen(test_data);

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write test data. */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);

    /* Flush should succeed (even though it's a no-op for memory buffers). */
    munit_assert(io->flush(io->stream) == SAIL_OK);

    /* Verify data is still there. */
    size_t buffer_size;
    munit_assert(sail_io_expanding_buffer_size(io, &buffer_size) == SAIL_OK);
    munit_assert(buffer_size == test_data_size);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    const char* test_data         = "Read this back!";
    const size_t test_data_size   = strlen(test_data);

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write test data. */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);

    /* Seek to beginning. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);

    /* Read back. */
    char read_buffer[256];
    size_t read_size;
    munit_assert(io->tolerant_read(io->stream, read_buffer, test_data_size, &read_size) == SAIL_OK);
    munit_assert(read_size == test_data_size);
    munit_assert(memcmp(read_buffer, test_data, test_data_size) == 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_seek_tell(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    struct sail_io* io            = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write some data. */
    const char* test_data = "0123456789";
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, strlen(test_data), &written_size) == SAIL_OK);

    /* Tell. */
    size_t offset;
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == strlen(test_data));

    /* Seek to beginning. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 0);

    /* Seek to middle. */
    munit_assert(io->seek(io->stream, 5, SEEK_SET) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 5);

    /* Seek from current. */
    munit_assert(io->seek(io->stream, 2, SEEK_CUR) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 7);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_eof(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    const char* test_data         = "EOF test";
    const size_t test_data_size   = strlen(test_data);

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Write test data. */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);

    /* Check EOF at end. */
    bool eof;
    munit_assert(io->eof(io->stream, &eof) == SAIL_OK);
    munit_assert(eof == true);

    /* Seek to beginning. */
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    munit_assert(io->eof(io->stream, &eof) == SAIL_OK);
    munit_assert(eof == false);

    sail_destroy_io(io);

    return MUNIT_OK;
}

static MunitResult test_expanding_buffer_size(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const size_t initial_capacity = 1024;
    const char* test_data         = "Size test";
    const size_t test_data_size   = strlen(test_data);

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io_write_expanding_buffer(initial_capacity, &io) == SAIL_OK);

    /* Check size callback is set */
    munit_assert(io->size != NULL);

    /* Initially size should be 0 */
    size_t size;
    munit_assert(sail_io_size(io, &size) == SAIL_OK);
    munit_assert(size == 0);

    /* Write test data. */
    size_t written_size;
    munit_assert(io->tolerant_write(io->stream, test_data, test_data_size, &written_size) == SAIL_OK);

    /* Size should now be test_data_size */
    munit_assert(sail_io_size(io, &size) == SAIL_OK);
    munit_assert(size == test_data_size);

    /* Get size using callback directly */
    size_t size_direct;
    munit_assert(io->size(io->stream, &size_direct) == SAIL_OK);
    munit_assert(size_direct == test_data_size);

    /* Size should match sail_io_expanding_buffer_size */
    size_t buffer_size;
    munit_assert(sail_io_expanding_buffer_size(io, &buffer_size) == SAIL_OK);
    munit_assert(buffer_size == test_data_size);
    munit_assert(buffer_size == size);

    /* Size should remain the same after seeking */
    munit_assert(io->seek(io->stream, 5, SEEK_SET) == SAIL_OK);
    size_t size_after_seek;
    munit_assert(sail_io_size(io, &size_after_seek) == SAIL_OK);
    munit_assert(size_after_seek == test_data_size);

    sail_destroy_io(io);

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
    { (char *)"/size",            test_expanding_buffer_size,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-expanding-buffer", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
