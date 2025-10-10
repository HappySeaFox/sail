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

#include <stdio.h>
#include <string.h>

#include <sail-manip/sail-manip.h>
#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

/* Test loading from custom I/O (file-based) */
static MunitResult test_technical_diver_load_from_io_file(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_file(path, &io) == SAIL_OK);
    munit_assert_not_null(io);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_io(io, codec_info, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_io(io);

    return MUNIT_OK;
}

/* Test loading from custom I/O (memory-based) */
static MunitResult test_technical_diver_load_from_io_memory(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* data;
    size_t data_size;
    munit_assert(sail_alloc_data_from_file_contents(path, &data, &data_size) == SAIL_OK);

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_memory(data, data_size, &io) == SAIL_OK);
    munit_assert_not_null(io);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_io(io, codec_info, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_io(io);
    sail_free(data);

    return MUNIT_OK;
}

/* Test loading from I/O with options */
static MunitResult test_technical_diver_load_from_io_with_options(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_file(path, &io) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_io_with_options(io, codec_info, load_options, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_load_options(load_options);
    sail_destroy_io(io);

    return MUNIT_OK;
}

/* Test saving to custom I/O (file-based) */
static MunitResult test_technical_diver_save_to_io_file(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    char temp_path[256];
    const char* ext = strrchr(path, '.');
    if (ext != NULL)
    {
        size_t base_len = ext - path;
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.io-file%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.io-file", path);
    }

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_write_file(temp_path, &io) == SAIL_OK);

    void* state = NULL;
    sail_status_t status = sail_start_saving_into_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

    sail_destroy_io(io);
    sail_destroy_image(image_to_save);
    sail_destroy_image(image);
    remove(temp_path);

    if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
        || status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        return MUNIT_SKIP;
    }

    munit_assert(status == SAIL_OK);

    return MUNIT_OK;
}

/* Test saving to custom I/O (memory-based) */
static MunitResult test_technical_diver_save_to_io_memory(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    size_t buffer_size = 1024 * 1024;
    void* buffer;
    munit_assert(sail_malloc(buffer_size, &buffer) == SAIL_OK);

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_write_memory(buffer, buffer_size, &io) == SAIL_OK);

    void* state = NULL;
    sail_status_t status = sail_start_saving_into_io(io, codec_info, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

    sail_destroy_io(io);
    sail_destroy_image(image_to_save);
    sail_destroy_image(image);
    sail_free(buffer);

    if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
        || status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        return MUNIT_SKIP;
    }

    munit_assert(status == SAIL_OK);

    return MUNIT_OK;
}

/* Test saving to I/O with options */
static MunitResult test_technical_diver_save_to_io_with_options(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    char temp_path[256];
    const char* ext = strrchr(path, '.');
    if (ext != NULL)
    {
        size_t base_len = ext - path;
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.io-opts%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.io-opts", path);
    }

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_write_file(temp_path, &io) == SAIL_OK);

    struct sail_save_options* save_options;
    munit_assert(sail_alloc_save_options_from_features(codec_info->save_features, &save_options) == SAIL_OK);

    void* state = NULL;
    sail_status_t status = sail_start_saving_into_io_with_options(io, codec_info, save_options, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

    sail_destroy_save_options(save_options);
    sail_destroy_io(io);
    sail_destroy_image(image_to_save);
    sail_destroy_image(image);
    remove(temp_path);

    if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
        || status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        return MUNIT_SKIP;
    }

    munit_assert(status == SAIL_OK);

    return MUNIT_OK;
}

/* Test I/O callbacks - check that all required callbacks work */
static MunitResult test_technical_diver_io_callbacks(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_file(path, &io) == SAIL_OK);

    munit_assert(io->tolerant_read != NULL);
    munit_assert(io->strict_read != NULL);
    munit_assert(io->seek != NULL);
    munit_assert(io->tell != NULL);
    munit_assert(io->close != NULL);
    munit_assert(io->eof != NULL);

    size_t offset = 0;
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 0);

    munit_assert(io->seek(io->stream, 10, SEEK_SET) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 10);

    munit_assert(io->seek(io->stream, 5, SEEK_CUR) == SAIL_OK);
    munit_assert(io->tell(io->stream, &offset) == SAIL_OK);
    munit_assert(offset == 15);

    bool is_eof = true;
    munit_assert(io->eof(io->stream, &is_eof) == SAIL_OK);
    munit_assert(is_eof == false);

    char buffer[16];
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    munit_assert(io->strict_read(io->stream, buffer, sizeof(buffer)) == SAIL_OK);

    size_t read_size = 0;
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    munit_assert(io->tolerant_read(io->stream, buffer, sizeof(buffer), &read_size) == SAIL_OK);
    munit_assert(read_size > 0);

    sail_destroy_io(io);

    return MUNIT_OK;
}

/* Test I/O features - check seekable flag */
static MunitResult test_technical_diver_io_features(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_io* io_file;
    munit_assert(sail_alloc_io_read_file(path, &io_file) == SAIL_OK);
    munit_assert((io_file->features & SAIL_IO_FEATURE_SEEKABLE) != 0);
    sail_destroy_io(io_file);

    void* data;
    size_t data_size;
    munit_assert(sail_alloc_data_from_file_contents(path, &data, &data_size) == SAIL_OK);

    struct sail_io* io_mem;
    munit_assert(sail_alloc_io_read_memory(data, data_size, &io_mem) == SAIL_OK);
    munit_assert((io_mem->features & SAIL_IO_FEATURE_SEEKABLE) != 0);

    sail_destroy_io(io_mem);
    sail_free(data);

    return MUNIT_OK;
}

/* Test I/O round-trip: load with custom I/O -> save with custom I/O -> load again and compare metadata only */
static MunitResult test_technical_diver_io_roundtrip(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_io* io1;
    munit_assert(sail_alloc_io_read_file(path, &io1) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state1 = NULL;
    munit_assert(sail_start_loading_from_io(io1, codec_info, &state1) == SAIL_OK);

    struct sail_image* image1 = NULL;
    munit_assert(sail_load_next_frame(state1, &image1) == SAIL_OK);
    munit_assert(sail_stop_loading(state1) == SAIL_OK);
    sail_destroy_io(io1);

    if (codec_info->save_features == NULL)
    {
        sail_destroy_image(image1);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status = sail_convert_image_for_saving(image1, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image1);
        return MUNIT_SKIP;
    }

    char temp_path[256];
    const char* ext = strrchr(path, '.');
    if (ext != NULL)
    {
        size_t base_len = ext - path;
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.io-roundtrip%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.io-roundtrip", path);
    }

    struct sail_io* io2;
    munit_assert(sail_alloc_io_read_write_file(temp_path, &io2) == SAIL_OK);

    void* state2 = NULL;
    sail_status_t save_status = sail_start_saving_into_io(io2, codec_info, &state2);

    if (save_status == SAIL_OK)
    {
        save_status = sail_write_next_frame(state2, image_to_save);
    }

    if (state2 != NULL)
    {
        sail_stop_saving(state2);
    }

    sail_destroy_io(io2);
    sail_destroy_image(image_to_save);

    if (save_status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || save_status == SAIL_ERROR_UNDERLYING_CODEC
        || save_status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        sail_destroy_image(image1);
        remove(temp_path);
        return MUNIT_SKIP;
    }

    munit_assert(save_status == SAIL_OK);

    struct sail_io* io3;
    munit_assert(sail_alloc_io_read_file(temp_path, &io3) == SAIL_OK);

    void* state3 = NULL;
    munit_assert(sail_start_loading_from_io(io3, codec_info, &state3) == SAIL_OK);

    struct sail_image* image2 = NULL;
    munit_assert(sail_load_next_frame(state3, &image2) == SAIL_OK);
    munit_assert(sail_stop_loading(state3) == SAIL_OK);
    sail_destroy_io(io3);

    munit_assert(image2->width == image1->width);
    munit_assert(image2->height == image1->height);
    munit_assert(image2->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);

    sail_destroy_image(image1);
    sail_destroy_image(image2);
    remove(temp_path);

    return MUNIT_OK;
}

/* Test I/O write callbacks */
static MunitResult test_technical_diver_io_write_callbacks(const MunitParameter params[], void* user_data)
{
    (void)user_data;
    (void)params;

    size_t buffer_size = 1024;
    void* buffer;
    munit_assert(sail_malloc(buffer_size, &buffer) == SAIL_OK);

    struct sail_io* io;
    munit_assert(sail_alloc_io_read_write_memory(buffer, buffer_size, &io) == SAIL_OK);

    munit_assert(io->tolerant_write != NULL);
    munit_assert(io->strict_write != NULL);
    munit_assert(io->flush != NULL);

    const char test_data[] = "SAIL Test Data";
    munit_assert(io->strict_write(io->stream, test_data, sizeof(test_data)) == SAIL_OK);
    munit_assert(io->flush(io->stream) == SAIL_OK);

    size_t written = 0;
    munit_assert(io->seek(io->stream, 0, SEEK_SET) == SAIL_OK);
    munit_assert(io->tolerant_write(io->stream, test_data, sizeof(test_data), &written) == SAIL_OK);
    munit_assert(written == sizeof(test_data));

    sail_destroy_io(io);
    sail_free(buffer);

    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/load-from-io-file",         test_technical_diver_load_from_io_file,         NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-from-io-memory",       test_technical_diver_load_from_io_memory,       NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-from-io-with-options", test_technical_diver_load_from_io_with_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-to-io-file",           test_technical_diver_save_to_io_file,           NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-to-io-memory",         test_technical_diver_save_to_io_memory,         NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-to-io-with-options",   test_technical_diver_save_to_io_with_options,   NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/io-callbacks",              test_technical_diver_io_callbacks,              NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/io-features",               test_technical_diver_io_features,               NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/io-roundtrip",              test_technical_diver_io_roundtrip,              NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/io-write-callbacks",        test_technical_diver_io_write_callbacks,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/technical-diver-api", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
