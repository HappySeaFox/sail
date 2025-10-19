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

/* Test loading a single frame using Advanced API */
static MunitResult test_advanced_load_single_frame_from_file(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert_not_null(state);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);
    munit_assert(image->width > 0);
    munit_assert(image->height > 0);
    munit_assert_not_null(image->pixels);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test loading with explicit codec info */
static MunitResult test_advanced_load_with_codec_info(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, codec_info, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test loading from memory buffer */
static MunitResult test_advanced_load_from_memory(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* data;
    size_t data_size;
    munit_assert(sail_alloc_data_from_file_contents(path, &data, &data_size) == SAIL_OK);
    munit_assert_not_null(data);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_memory(data, data_size, codec_info, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    munit_assert_not_null(image);

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_free(data);

    return MUNIT_OK;
}

/* Test that attempting to load after EOF returns SAIL_ERROR_NO_MORE_FRAMES or succeeds for multi-frame */
static MunitResult test_advanced_load_no_more_frames(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    sail_destroy_image(image);

    image                = NULL;
    sail_status_t status = sail_load_next_frame(state, &image);

    if (status == SAIL_OK)
    {
        sail_destroy_image(image);
    }
    else
    {
        munit_assert(status == SAIL_ERROR_NO_MORE_FRAMES);
        munit_assert_null(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test early stop - stop loading after start but before loading any frames */
static MunitResult test_advanced_early_stop_loading(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test stop_loading with NULL state (should do nothing and not crash) */
static MunitResult test_advanced_stop_loading_null(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert(sail_stop_loading(NULL) == SAIL_OK);

    return MUNIT_OK;
}

/* Test saving a single frame using Advanced API */
static MunitResult test_advanced_save_single_frame_to_file(const MunitParameter params[], void* user_data)
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
        snprintf(temp_path, sizeof(temp_path), "%.*s.test%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test", path);
    }

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file(temp_path, codec_info, &state);

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

/* Test saving to memory buffer */
static MunitResult test_advanced_save_to_memory(const MunitParameter params[], void* user_data)
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
    sail_status_t status = sail_start_saving_into_memory(buffer, buffer_size, codec_info, &state);

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
    sail_free(buffer);

    if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNDERLYING_CODEC
        || status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        return MUNIT_SKIP;
    }

    munit_assert(status == SAIL_OK);

    return MUNIT_OK;
}

/* Test early stop - stop saving after start but before writing any frames */
static MunitResult test_advanced_early_stop_saving(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL)
    {
        return MUNIT_SKIP;
    }

    char temp_path[256];
    const char* ext = strrchr(path, '.');
    if (ext != NULL)
    {
        size_t base_len = ext - path;
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.early%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.early", path);
    }

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file(temp_path, codec_info, &state);

    if (status != SAIL_OK)
    {
        remove(temp_path);
        return MUNIT_SKIP;
    }

    sail_status_t stop_status = sail_stop_saving(state);

    if (stop_status == SAIL_ERROR_NO_MORE_FRAMES || stop_status == SAIL_ERROR_UNDERLYING_CODEC
        || stop_status == SAIL_ERROR_NOT_IMPLEMENTED)
    {
        remove(temp_path);
        return MUNIT_SKIP;
    }

    munit_assert(stop_status == SAIL_OK);
    remove(temp_path);

    return MUNIT_OK;
}

/* Test stop_saving with NULL state */
static MunitResult test_advanced_stop_saving_null(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert(sail_stop_saving(NULL) == SAIL_OK);

    return MUNIT_OK;
}

/* Test round-trip: load -> save -> load again and compare metadata only */
static MunitResult test_advanced_roundtrip(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    /* Load with META_DATA option to preserve special properties. */
    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);
    load_options->options |= SAIL_OPTION_META_DATA;

    void* state1 = NULL;
    munit_assert(sail_start_loading_from_file_with_options(path, NULL, load_options, &state1) == SAIL_OK);

    struct sail_image* image1 = NULL;
    munit_assert(sail_load_next_frame(state1, &image1) == SAIL_OK);
    munit_assert(sail_stop_loading(state1) == SAIL_OK);

    sail_destroy_load_options(load_options);

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    if (codec_info->save_features == NULL)
    {
        sail_destroy_image(image1);
        return MUNIT_SKIP;
    }

    struct sail_image* image_to_save = NULL;
    sail_status_t conv_status        = sail_convert_image_for_saving(image1, codec_info->save_features, &image_to_save);

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
        snprintf(temp_path, sizeof(temp_path), "%.*s.test.roundtrip%s", (int)base_len, path, ext);
    }
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s.test.roundtrip", path);
    }

    /* Prepare save options with tuning from special properties. */
    struct sail_save_options* save_options = NULL;
    munit_assert(sail_alloc_save_options_from_features(codec_info->save_features, &save_options) == SAIL_OK);

    if (image_to_save->source_image != NULL && image_to_save->source_image->special_properties != NULL)
    {
        /* Copy special properties to tuning hash map for roundtrip. */
        munit_assert(sail_copy_hash_map(image_to_save->source_image->special_properties, &save_options->tuning)
                     == SAIL_OK);
    }

    void* state2              = NULL;
    sail_status_t save_status = sail_start_saving_into_file_with_options(temp_path, codec_info, save_options, &state2);

    if (save_status == SAIL_OK)
    {
        save_status = sail_write_next_frame(state2, image_to_save);
    }

    if (state2 != NULL)
    {
        sail_stop_saving(state2);
    }

    sail_destroy_save_options(save_options);
    sail_destroy_image(image_to_save);

    if (save_status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || save_status == SAIL_ERROR_UNDERLYING_CODEC)
    {
        sail_destroy_image(image1);
        remove(temp_path);
        return MUNIT_SKIP;
    }

    munit_assert(save_status == SAIL_OK);

    void* state3 = NULL;
    munit_assert(sail_start_loading_from_file(temp_path, NULL, &state3) == SAIL_OK);

    struct sail_image* image2 = NULL;
    munit_assert(sail_load_next_frame(state3, &image2) == SAIL_OK);
    munit_assert(sail_stop_loading(state3) == SAIL_OK);

    munit_assert(image2->width == image1->width);
    munit_assert(image2->height == image1->height);
    munit_assert(image2->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);

    sail_destroy_image(image1);
    sail_destroy_image(image2);
    remove(temp_path);

    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/load-single-frame-from-file", test_advanced_load_single_frame_from_file, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-with-codec-info",        test_advanced_load_with_codec_info,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-from-memory",            test_advanced_load_from_memory,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/load-no-more-frames",         test_advanced_load_no_more_frames,         NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/early-stop-loading",          test_advanced_early_stop_loading,          NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/stop-loading-null",           test_advanced_stop_loading_null,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/save-single-frame-to-file",   test_advanced_save_single_frame_to_file,   NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/save-to-memory",              test_advanced_save_to_memory,              NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/early-stop-saving",           test_advanced_early_stop_saving,           NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/stop-saving-null",            test_advanced_stop_saving_null,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/roundtrip",                   test_advanced_roundtrip,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/advanced-api", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
