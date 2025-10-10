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

#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <zlib.h>

#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

/* Gzip compression wrapper state */
struct gzip_io_state
{
    gzFile gz_file;
    const char* path;
};

/* Error simulation wrapper state */
struct error_sim_state
{
    struct sail_io* underlying_io;
    size_t fail_after_bytes;
    size_t bytes_read;
    bool should_fail;
};

/* Gzip I/O callbacks */
static sail_status_t gzip_io_tolerant_read(void* stream, void* buf, size_t size_to_read, size_t* read_size)
{
    struct gzip_io_state* state = (struct gzip_io_state*)stream;

    int bytes_read = gzread(state->gz_file, buf, (unsigned)size_to_read);

    if (bytes_read < 0)
    {
        return SAIL_ERROR_READ_IO;
    }

    *read_size = (size_t)bytes_read;
    return SAIL_OK;
}

static sail_status_t gzip_io_strict_read(void* stream, void* buf, size_t size_to_read)
{
    size_t read_size;
    SAIL_TRY(gzip_io_tolerant_read(stream, buf, size_to_read, &read_size));

    if (read_size != size_to_read)
    {
        return SAIL_ERROR_READ_IO;
    }

    return SAIL_OK;
}

static sail_status_t gzip_io_seek(void* stream, long offset, int whence)
{
    struct gzip_io_state* state = (struct gzip_io_state*)stream;

    z_off_t result = gzseek(state->gz_file, offset, whence);

    return (result < 0) ? SAIL_ERROR_SEEK_IO : SAIL_OK;
}

static sail_status_t gzip_io_tell(void* stream, size_t* offset)
{
    struct gzip_io_state* state = (struct gzip_io_state*)stream;

    z_off_t pos = gztell(state->gz_file);

    if (pos < 0)
    {
        return SAIL_ERROR_TELL_IO;
    }

    *offset = (size_t)pos;
    return SAIL_OK;
}

static sail_status_t gzip_io_close(void* stream)
{
    struct gzip_io_state* state = (struct gzip_io_state*)stream;

    if (state->gz_file != NULL)
    {
        gzclose(state->gz_file);
        state->gz_file = NULL;
    }

    return SAIL_OK;
}

static sail_status_t gzip_io_eof(void* stream, bool* result)
{
    struct gzip_io_state* state = (struct gzip_io_state*)stream;

    *result = (gzeof(state->gz_file) != 0);

    return SAIL_OK;
}

/* Error simulation callbacks */
static sail_status_t error_sim_tolerant_read(void* stream, void* buf, size_t size_to_read, size_t* read_size)
{
    struct error_sim_state* state = (struct error_sim_state*)stream;

    if (state->should_fail && state->bytes_read >= state->fail_after_bytes)
    {
        return SAIL_ERROR_READ_IO;
    }

    sail_status_t status =
        state->underlying_io->tolerant_read(state->underlying_io->stream, buf, size_to_read, read_size);

    if (status == SAIL_OK)
    {
        state->bytes_read += *read_size;
    }

    return status;
}

static sail_status_t error_sim_strict_read(void* stream, void* buf, size_t size_to_read)
{
    size_t read_size;
    SAIL_TRY(error_sim_tolerant_read(stream, buf, size_to_read, &read_size));

    if (read_size != size_to_read)
    {
        return SAIL_ERROR_READ_IO;
    }

    return SAIL_OK;
}

static sail_status_t error_sim_seek(void* stream, long offset, int whence)
{
    struct error_sim_state* state = (struct error_sim_state*)stream;
    return state->underlying_io->seek(state->underlying_io->stream, offset, whence);
}

static sail_status_t error_sim_tell(void* stream, size_t* offset)
{
    struct error_sim_state* state = (struct error_sim_state*)stream;
    return state->underlying_io->tell(state->underlying_io->stream, offset);
}

static sail_status_t error_sim_close(void* stream)
{
    struct error_sim_state* state = (struct error_sim_state*)stream;
    return state->underlying_io->close(state->underlying_io->stream);
}

static sail_status_t error_sim_eof(void* stream, bool* result)
{
    struct error_sim_state* state = (struct error_sim_state*)stream;
    return state->underlying_io->eof(state->underlying_io->stream, result);
}

/* Helper function to compress file using zlib */
static bool compress_file_gzip(const char* input_path, const char* output_path)
{
    FILE* input = fopen(input_path, "rb");
    if (!input)
        return false;

    gzFile output = gzopen(output_path, "wb");
    if (!output)
    {
        fclose(input);
        return false;
    }

    unsigned char buffer[8192];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0)
    {
        if (gzwrite(output, buffer, (unsigned)bytes_read) != (int)bytes_read)
        {
            gzclose(output);
            fclose(input);
            return false;
        }
    }

    gzclose(output);
    fclose(input);
    return true;
}

/* Test loading from gzip-compressed image */
static MunitResult test_custom_io_gzip_wrapper(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* input_path = SAIL_TEST_IMAGES[0];
    char gz_path[512]      = "/tmp/sail_test_gzip_XXXXXX.png.gz";

    int fd = mkstemps(gz_path, 7);
    if (fd < 0)
        return MUNIT_SKIP;
    close(fd);

    if (!compress_file_gzip(input_path, gz_path))
    {
        remove(gz_path);
        return MUNIT_SKIP;
    }

    struct gzip_io_state* gz_state = NULL;
    sail_malloc(sizeof(struct gzip_io_state), (void**)&gz_state);
    gz_state->path    = gz_path;
    gz_state->gz_file = gzopen(gz_path, "rb");

    if (gz_state->gz_file == NULL)
    {
        sail_free(gz_state);
        remove(gz_path);
        return MUNIT_SKIP;
    }

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io(&io) == SAIL_OK);

    io->stream        = gz_state;
    io->tolerant_read = gzip_io_tolerant_read;
    io->strict_read   = gzip_io_strict_read;
    io->seek          = gzip_io_seek;
    io->tell          = gzip_io_tell;
    io->close         = gzip_io_close;
    io->eof           = gzip_io_eof;
    io->features      = SAIL_IO_FEATURE_SEEKABLE;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(input_path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (status == SAIL_OK)
        {
            munit_assert(image->width > 0);
            munit_assert(image->height > 0);
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
    }

    sail_destroy_io(io);
    sail_free(gz_state);
    remove(gz_path);

    return MUNIT_OK;
}

/* Test I/O error during read */
static MunitResult test_custom_io_error_during_read(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES[0];

    struct sail_io* underlying_io = NULL;
    munit_assert(sail_alloc_io_read_file(path, &underlying_io) == SAIL_OK);

    struct error_sim_state* err_state = NULL;
    sail_malloc(sizeof(struct error_sim_state), (void**)&err_state);
    err_state->underlying_io    = underlying_io;
    err_state->fail_after_bytes = 100;
    err_state->bytes_read       = 0;
    err_state->should_fail      = true;

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io(&io) == SAIL_OK);

    io->stream        = err_state;
    io->tolerant_read = error_sim_tolerant_read;
    io->strict_read   = error_sim_strict_read;
    io->seek          = error_sim_seek;
    io->tell          = error_sim_tell;
    io->close         = error_sim_close;
    io->eof           = error_sim_eof;
    io->features      = underlying_io->features;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (image != NULL)
        {
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
        munit_assert(status != SAIL_OK);
    }

    io->close = NULL;
    sail_destroy_io(io);
    sail_free(err_state);
    sail_destroy_io(underlying_io);

    return MUNIT_OK;
}

/* Test I/O error before any reads */
static MunitResult test_custom_io_error_immediate(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES[0];

    struct sail_io* underlying_io = NULL;
    munit_assert(sail_alloc_io_read_file(path, &underlying_io) == SAIL_OK);

    struct error_sim_state* err_state = NULL;
    sail_malloc(sizeof(struct error_sim_state), (void**)&err_state);
    err_state->underlying_io    = underlying_io;
    err_state->fail_after_bytes = 0;
    err_state->bytes_read       = 0;
    err_state->should_fail      = true;

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io(&io) == SAIL_OK);

    io->stream        = err_state;
    io->tolerant_read = error_sim_tolerant_read;
    io->strict_read   = error_sim_strict_read;
    io->seek          = error_sim_seek;
    io->tell          = error_sim_tell;
    io->close         = error_sim_close;
    io->eof           = error_sim_eof;
    io->features      = underlying_io->features;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_io(io, codec_info, &state);

    munit_assert(status != SAIL_OK);

    io->close = NULL;
    sail_destroy_io(io);
    sail_free(err_state);
    sail_destroy_io(underlying_io);

    return MUNIT_OK;
}

/* Test partial read simulation */
static MunitResult test_custom_io_partial_reads(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES[0];

    struct sail_io* underlying_io = NULL;
    munit_assert(sail_alloc_io_read_file(path, &underlying_io) == SAIL_OK);

    struct error_sim_state* err_state = NULL;
    sail_malloc(sizeof(struct error_sim_state), (void**)&err_state);
    err_state->underlying_io    = underlying_io;
    err_state->fail_after_bytes = SIZE_MAX;
    err_state->bytes_read       = 0;
    err_state->should_fail      = false;

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io(&io) == SAIL_OK);

    io->stream        = err_state;
    io->tolerant_read = error_sim_tolerant_read;
    io->strict_read   = error_sim_strict_read;
    io->seek          = error_sim_seek;
    io->tell          = error_sim_tell;
    io->close         = error_sim_close;
    io->eof           = error_sim_eof;
    io->features      = underlying_io->features;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (status == SAIL_OK)
        {
            munit_assert(image->width > 0);
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
    }

    io->close = NULL;
    sail_destroy_io(io);
    sail_free(err_state);
    sail_destroy_io(underlying_io);

    return MUNIT_OK;
}

/* Test gzip compression round-trip */
static MunitResult test_custom_io_gzip_roundtrip(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* input_path = SAIL_TEST_IMAGES[0];

    struct sail_image* original_image = NULL;
    munit_assert(sail_load_from_file(input_path, &original_image) == SAIL_OK);

    char gz_path[512] = "/tmp/sail_test_roundtrip_XXXXXX.png.gz";

    int fd = mkstemps(gz_path, 7);
    if (fd < 0)
    {
        sail_destroy_image(original_image);
        return MUNIT_SKIP;
    }
    close(fd);

    if (!compress_file_gzip(input_path, gz_path))
    {
        sail_destroy_image(original_image);
        remove(gz_path);
        return MUNIT_SKIP;
    }

    struct gzip_io_state* gz_state = NULL;
    sail_malloc(sizeof(struct gzip_io_state), (void**)&gz_state);
    gz_state->path    = gz_path;
    gz_state->gz_file = gzopen(gz_path, "rb");

    if (gz_state->gz_file == NULL)
    {
        sail_destroy_image(original_image);
        sail_free(gz_state);
        remove(gz_path);
        return MUNIT_SKIP;
    }

    struct sail_io* io = NULL;
    munit_assert(sail_alloc_io(&io) == SAIL_OK);

    io->stream        = gz_state;
    io->tolerant_read = gzip_io_tolerant_read;
    io->strict_read   = gzip_io_strict_read;
    io->seek          = gzip_io_seek;
    io->tell          = gzip_io_tell;
    io->close         = gzip_io_close;
    io->eof           = gzip_io_eof;
    io->features      = SAIL_IO_FEATURE_SEEKABLE;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(input_path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* decompressed_image = NULL;
        status                                = sail_load_next_frame(state, &decompressed_image);

        if (status == SAIL_OK)
        {
            munit_assert(decompressed_image->width == original_image->width);
            munit_assert(decompressed_image->height == original_image->height);
            sail_destroy_image(decompressed_image);
        }

        sail_stop_loading(state);
    }

    sail_destroy_io(io);
    sail_free(gz_state);
    sail_destroy_image(original_image);
    remove(gz_path);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/error-during-read", test_custom_io_error_during_read, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/error-immediate",   test_custom_io_error_immediate,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/gzip-roundtrip",    test_custom_io_gzip_roundtrip,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/gzip-wrapper",      test_custom_io_gzip_wrapper,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/partial-reads",     test_custom_io_partial_reads,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/custom-io", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
