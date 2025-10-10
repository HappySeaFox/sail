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

/* Test loading with NULL options (should use defaults) */
static MunitResult test_deep_diver_load_with_null_options(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file_with_options(path, codec_info, NULL, &state) == SAIL_OK);
    munit_assert_not_null(state);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test loading with custom load options */
static MunitResult test_deep_diver_load_with_custom_options(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file_with_options(path, codec_info, load_options, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

/* Test loading from memory with options */
static MunitResult test_deep_diver_load_from_memory_with_options(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* data;
    size_t data_size;
    munit_assert(sail_alloc_data_from_file_contents(path, &data, &data_size) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_memory_with_options(data, data_size, codec_info, load_options, &state)
                 == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_load_options(load_options);
    sail_free(data);

    return MUNIT_OK;
}

/* Test saving with NULL options (should use defaults) */
static MunitResult test_deep_diver_save_with_null_options(const MunitParameter params[], void* user_data)
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
    sail_status_t conv_status        = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

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
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.null-opts%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.null-opts", path);
    }

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file_with_options(temp_path, codec_info, NULL, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

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

/* Test saving with custom save options */
static MunitResult test_deep_diver_save_with_custom_options(const MunitParameter params[], void* user_data)
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
    sail_status_t conv_status        = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

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
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.custom-opts%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.custom-opts", path);
    }

    struct sail_save_options* save_options;
    munit_assert(sail_alloc_save_options_from_features(codec_info->save_features, &save_options) == SAIL_OK);

    if (codec_info->save_features->compression_level != NULL)
    {
        save_options->compression_level = codec_info->save_features->compression_level->max_level;
    }

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file_with_options(temp_path, codec_info, save_options, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

    sail_destroy_save_options(save_options);
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

/* Test saving to memory with options */
static MunitResult test_deep_diver_save_to_memory_with_options(const MunitParameter params[], void* user_data)
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
    sail_status_t conv_status        = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    struct sail_save_options* save_options;
    munit_assert(sail_alloc_save_options_from_features(codec_info->save_features, &save_options) == SAIL_OK);

    size_t buffer_size = 1024 * 1024;
    void* buffer;
    munit_assert(sail_malloc(buffer_size, &buffer) == SAIL_OK);

    void* state = NULL;
    sail_status_t status =
        sail_start_saving_into_memory_with_options(buffer, buffer_size, codec_info, save_options, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    if (state != NULL)
    {
        sail_stop_saving(state);
    }

    sail_destroy_save_options(save_options);
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

/* Test sail_stop_saving_with_written() to get bytes written */
static MunitResult test_deep_diver_stop_saving_with_written(const MunitParameter params[], void* user_data)
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
    sail_status_t conv_status        = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    size_t buffer_size = 1024 * 1024;
    void* buffer;
    munit_assert(sail_malloc(buffer_size, &buffer) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_memory_with_options(buffer, buffer_size, codec_info, NULL, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image_to_save);
    }

    size_t written = 0;
    if (state != NULL)
    {
        sail_stop_saving_with_written(state, &written);
    }

    sail_destroy_image(image_to_save);
    sail_destroy_image(image);
    sail_free(buffer);

    if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
        || status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        return MUNIT_SKIP;
    }

    munit_assert(status == SAIL_OK);
    munit_assert(written > 0);
    munit_assert(written <= buffer_size);

    return MUNIT_OK;
}

/* Test that options are properly copied (deep copy) */
static MunitResult test_deep_diver_options_are_copied(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file_with_options(path, codec_info, load_options, &state) == SAIL_OK);

    sail_destroy_load_options(load_options);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test various compression levels */
static MunitResult test_deep_diver_compression_levels(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL || codec_info->save_features->compression_level == NULL)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status        = sail_convert_image_for_saving(image, codec_info->save_features, &image_to_save);

    if (conv_status != SAIL_OK)
    {
        sail_destroy_image(image);
        return MUNIT_SKIP;
    }

    double levels[] = {codec_info->save_features->compression_level->min_level,
                       codec_info->save_features->compression_level->default_level,
                       codec_info->save_features->compression_level->max_level};

    for (size_t i = 0; i < sizeof(levels) / sizeof(levels[0]); i++)
    {
        struct sail_save_options* save_options;
        munit_assert(sail_alloc_save_options_from_features(codec_info->save_features, &save_options) == SAIL_OK);
        save_options->compression_level = levels[i];

        char temp_path[256];
        const char* ext = strrchr(path, '.');
        if (ext != NULL)
        {
            size_t base_len = ext - path;
            snprintf(temp_path, sizeof(temp_path), "%.*s.test.comp-%zu%s", (int)base_len, path, i, ext);
        }
        else
        {
            snprintf(temp_path, sizeof(temp_path), "%s.test.comp-%zu", path, i);
        }

        void* state          = NULL;
        sail_status_t status = sail_start_saving_into_file_with_options(temp_path, codec_info, save_options, &state);

        if (status == SAIL_OK)
        {
            status = sail_write_next_frame(state, image_to_save);
        }

        if (state != NULL)
        {
            sail_stop_saving(state);
        }

        sail_destroy_save_options(save_options);
        remove(temp_path);

        if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
            || status == SAIL_ERROR_NOT_IMPLEMENTED)
        {
            sail_destroy_image(image_to_save);
            sail_destroy_image(image);
            return MUNIT_SKIP;
        }

        munit_assert(status == SAIL_OK);
    }

    sail_destroy_image(image_to_save);
    sail_destroy_image(image);

    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/load-with-null-options",        test_deep_diver_load_with_null_options,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-with-custom-options",      test_deep_diver_load_with_custom_options,      NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-from-memory-with-options", test_deep_diver_load_from_memory_with_options, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-with-null-options",        test_deep_diver_save_with_null_options,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-with-custom-options",      test_deep_diver_save_with_custom_options,      NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-to-memory-with-options",   test_deep_diver_save_to_memory_with_options,   NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/stop-saving-with-written",      test_deep_diver_stop_saving_with_written,      NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/options-are-copied",            test_deep_diver_options_are_copied,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/compression-levels",            test_deep_diver_compression_levels,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/deep-diver-api", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
