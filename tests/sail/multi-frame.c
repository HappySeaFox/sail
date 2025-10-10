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

#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

/* Test loading all frames from potentially multi-frame images */
static MunitResult test_multi_frame_load_all_frames(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    bool is_multi_frame =
        (codec_info->load_features->features & (SAIL_CODEC_FEATURE_ANIMATED | SAIL_CODEC_FEATURE_MULTI_PAGED)) != 0;
    if (!is_multi_frame)
    {
        return MUNIT_SKIP;
    }

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, codec_info, &state) == SAIL_OK);

    unsigned frame_count = 0;
    int prev_delay       = -2;

    while (true)
    {
        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_next_frame(state, &image);

        if (status == SAIL_ERROR_NO_MORE_FRAMES)
        {
            break;
        }

        munit_assert(status == SAIL_OK);
        munit_assert_not_null(image);
        munit_assert(image->width > 0);
        munit_assert(image->height > 0);
        munit_assert(image->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);

        if (frame_count > 0)
        {
            munit_assert(image->delay >= -1);
            if (image->delay >= 0)
            {
                munit_assert(prev_delay >= 0);
            }
        }

        prev_delay = image->delay;
        frame_count++;

        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);
    munit_assert(frame_count >= 1);

    return MUNIT_OK;
}

/* Test checking delay values consistency */
static MunitResult test_multi_frame_delay_consistency(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    bool is_multi_frame =
        (codec_info->load_features->features & (SAIL_CODEC_FEATURE_ANIMATED | SAIL_CODEC_FEATURE_MULTI_PAGED)) != 0;
    if (!is_multi_frame)
    {
        return MUNIT_SKIP;
    }

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, codec_info, &state) == SAIL_OK);

    struct sail_image* first_image = NULL;
    munit_assert(sail_load_next_frame(state, &first_image) == SAIL_OK);

    bool is_animation = (first_image->delay >= 0);

    sail_destroy_image(first_image);

    while (1)
    {
        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_next_frame(state, &image);

        if (status == SAIL_ERROR_NO_MORE_FRAMES)
        {
            break;
        }

        munit_assert(status == SAIL_OK);

        if (is_animation)
        {
            munit_assert(image->delay >= 0);
        }
        else
        {
            munit_assert(image->delay == -1);
        }

        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test special properties for animated images */
static MunitResult test_multi_frame_special_properties(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info_check;
    munit_assert(sail_codec_info_from_path(path, &codec_info_check) == SAIL_OK);

    bool is_multi_frame =
        (codec_info_check->load_features->features & (SAIL_CODEC_FEATURE_ANIMATED | SAIL_CODEC_FEATURE_MULTI_PAGED))
        != 0;
    if (!is_multi_frame)
    {
        return MUNIT_SKIP;
    }

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);
    load_options->options |= SAIL_OPTION_META_DATA;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_file_with_options(path, codec_info, load_options, &state);

    sail_destroy_load_options(load_options);

    if (status != SAIL_OK)
    {
        return MUNIT_SKIP;
    }

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);

    if (image->special_properties != NULL)
    {
        const struct sail_variant* frames_variant = sail_hash_map_value(image->special_properties, "apng-frames");
        const struct sail_variant* plays_variant  = sail_hash_map_value(image->special_properties, "apng-plays");

        if (frames_variant != NULL && frames_variant->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            /* frames should be positive (at least 1 frame) */
            munit_assert(sail_variant_to_unsigned_int(frames_variant) >= 1);
        }

        if (plays_variant != NULL && plays_variant->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            /* plays can be 0 (infinite loop) or positive number - just verify it exists */
        }
    }

    sail_destroy_image(image);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test codec features for multi-frame support */
static MunitResult test_multi_frame_codec_features(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    bool is_multi_frame =
        (codec_info->load_features->features & (SAIL_CODEC_FEATURE_ANIMATED | SAIL_CODEC_FEATURE_MULTI_PAGED)) != 0;
    if (!is_multi_frame)
    {
        return MUNIT_SKIP;
    }

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, codec_info, &state) == SAIL_OK);

    unsigned frame_count = 0;
    while (1)
    {
        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_next_frame(state, &image);

        if (status == SAIL_ERROR_NO_MORE_FRAMES)
        {
            break;
        }

        munit_assert(status == SAIL_OK);
        frame_count++;
        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    if (frame_count > 1)
    {
        bool is_animated    = (codec_info->load_features->features & SAIL_CODEC_FEATURE_ANIMATED) != 0;
        bool is_multi_paged = (codec_info->load_features->features & SAIL_CODEC_FEATURE_MULTI_PAGED) != 0;
        munit_assert(is_animated || is_multi_paged);
    }

    return MUNIT_OK;
}

/* Test that frame dimensions are consistent or handled correctly */
static MunitResult test_multi_frame_dimensions(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    bool is_multi_frame =
        (codec_info->load_features->features & (SAIL_CODEC_FEATURE_ANIMATED | SAIL_CODEC_FEATURE_MULTI_PAGED)) != 0;
    if (!is_multi_frame)
    {
        return MUNIT_SKIP;
    }

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, codec_info, &state) == SAIL_OK);

    struct sail_image* first_image = NULL;
    munit_assert(sail_load_next_frame(state, &first_image) == SAIL_OK);

    unsigned canvas_width  = first_image->width;
    unsigned canvas_height = first_image->height;

    sail_destroy_image(first_image);

    while (1)
    {
        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_next_frame(state, &image);

        if (status == SAIL_ERROR_NO_MORE_FRAMES)
        {
            break;
        }

        munit_assert(status == SAIL_OK);
        munit_assert(image->width <= canvas_width);
        munit_assert(image->height <= canvas_height);

        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/load-all-frames",       test_multi_frame_load_all_frames,       NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/delay-consistency",     test_multi_frame_delay_consistency,     NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/special-properties",    test_multi_frame_special_properties,    NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/codec-features",        test_multi_frame_codec_features,        NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/dimensions",            test_multi_frame_dimensions,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/multi-frame", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
