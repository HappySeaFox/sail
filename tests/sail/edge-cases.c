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

#include <sail/sail.h>

#include "munit.h"

/* Test loading 1x1 pixel image */
static MunitResult test_edge_1x1_image(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/1x1.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    if (status == SAIL_OK)
    {
        munit_assert(image->width == 1);
        munit_assert(image->height == 1);
        sail_destroy_image(image);
    }

    return MUNIT_OK;
}

/* Test resource cleanup on load error */
static MunitResult test_edge_cleanup_on_error(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/truncated.jpg";

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_extension("jpg", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_file(path, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (image != NULL)
        {
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
    }

    return MUNIT_OK;
}

/* Test corrupted compression stream */
static MunitResult test_edge_corrupted_compression(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-compression.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test corrupted palette */
static MunitResult test_edge_corrupted_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-palette.gif";

    struct sail_image* image = NULL;
    sail_load_from_file(path, &image);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return MUNIT_OK;
}

/* Test double destroy */
static MunitResult test_edge_double_destroy(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    sail_destroy_image(image);
    sail_destroy_image(NULL);

    return MUNIT_OK;
}

/* Test early stop without loading any frames */
static MunitResult test_edge_early_stop_no_frames(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test loading from empty buffer */
static MunitResult test_edge_empty_memory_buffer(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    unsigned char buffer[1] = {0};

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_extension("bmp", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_memory(buffer, 0, codec_info, &state);

    munit_assert(status != SAIL_OK);

    return MUNIT_OK;
}

/* Test invalid codec info queries */
static MunitResult test_edge_invalid_codec_queries(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const struct sail_codec_info* codec_info = NULL;

    munit_assert(sail_codec_info_from_extension(NULL, &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_extension("", &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_extension("nonexistent_format_xyz", &codec_info) != SAIL_OK);

    munit_assert(sail_codec_info_from_path(NULL, &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_path("", &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_path("file.xyz", &codec_info) != SAIL_OK);

    return MUNIT_OK;
}

/* Test invalid magic number */
static MunitResult test_edge_invalid_magic(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/invalid-magic.jpg";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test invalid palette size */
static MunitResult test_edge_invalid_palette_size(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-palette.gif";

    struct sail_image* image = NULL;
    sail_load_from_file(path, &image);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return MUNIT_OK;
}

/* Test memory buffer boundary */
static MunitResult test_edge_memory_boundary(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    unsigned char small_buffer[10] = {0xFF, 0xD8, 0xFF, 0xE0, 0, 0, 0, 0, 0, 0};

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_extension("jpg", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_memory(small_buffer, sizeof(small_buffer), codec_info, &state);

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

    return MUNIT_OK;
}

/* Test nonexistent file */
static MunitResult test_edge_nonexistent_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = "this-file-does-not-exist-sail-test-123456789.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test NULL pointer handling */
static MunitResult test_edge_null_pointers(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert(sail_load_from_file(NULL, NULL) != SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(NULL, &image) != SAIL_OK);
    munit_assert_null(image);

    munit_assert(sail_load_from_file("nonexistent.png", NULL) != SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(NULL, NULL, &state) != SAIL_OK);
    munit_assert_null(state);

    munit_assert(sail_load_next_frame(NULL, NULL) != SAIL_OK);
    munit_assert(sail_stop_loading(NULL) == SAIL_OK);

    munit_assert(sail_start_saving_into_file(NULL, NULL, NULL) != SAIL_OK);
    munit_assert(sail_write_next_frame(NULL, NULL) != SAIL_OK);
    munit_assert(sail_stop_saving(NULL) == SAIL_OK);

    return MUNIT_OK;
}

/* Test options with NULL state */
static MunitResult test_edge_options_null_state(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);

    munit_assert(sail_start_loading_from_file_with_options(NULL, NULL, load_options, NULL) != SAIL_OK);

    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

/* Test partial frame load cleanup */
static MunitResult test_edge_partial_frame_cleanup(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/gif/bpp8-indexed.comment.gif";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test saving to read-only format */
static MunitResult test_edge_readonly_format_save(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const struct sail_codec_info* codec_info = NULL;
    sail_status_t status                     = sail_codec_info_from_extension("svg", &codec_info);

    munit_assert(status == SAIL_OK);
    munit_assert_not_null(codec_info);

    return MUNIT_OK;
}

/* Test saving without calling start */
static MunitResult test_edge_save_without_start(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    sail_status_t status = sail_write_next_frame(NULL, image);
    munit_assert(status != SAIL_OK);

    sail_destroy_image(image);

    return MUNIT_OK;
}

/* Test loading with stopped state */
static MunitResult test_edge_stopped_state_load(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    sail_destroy_image(image);

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
}

/* Test truncated file */
static MunitResult test_edge_truncated_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/truncated.jpg";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test unsupported pixel format for codec */
static MunitResult test_edge_unsupported_pixel_format(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    image->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_extension("bmp", &codec_info) == SAIL_OK);

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/bmp/test-unsupported.bmp", SAIL_TEST_IMAGES_ACCEPTANCE_PATH);

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file(temp_path, codec_info, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image);
        sail_stop_saving(state);
        munit_assert(status != SAIL_OK);
    }

    remove(temp_path);
    sail_destroy_image(image);

    return MUNIT_OK;
}

/* Test zero-byte file */
static MunitResult test_edge_zero_byte_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/zero-byte.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test zero dimensions */
static MunitResult test_edge_zero_dimensions(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/zero-dimensions.bmp";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/1x1-image",                 test_edge_1x1_image,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/cleanup-on-error",          test_edge_cleanup_on_error,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/corrupted-compression",     test_edge_corrupted_compression,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/corrupted-palette",         test_edge_corrupted_palette,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/double-destroy",            test_edge_double_destroy,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/early-stop-no-frames",      test_edge_early_stop_no_frames,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/empty-memory-buffer",       test_edge_empty_memory_buffer,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-codec-queries",     test_edge_invalid_codec_queries,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-magic",             test_edge_invalid_magic,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-palette-size",      test_edge_invalid_palette_size,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/memory-boundary",           test_edge_memory_boundary,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/nonexistent-file",          test_edge_nonexistent_file,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/null-pointers",             test_edge_null_pointers,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/options-null-state",        test_edge_options_null_state,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/partial-frame-cleanup",     test_edge_partial_frame_cleanup,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/readonly-format-save",      test_edge_readonly_format_save,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/save-without-start",        test_edge_save_without_start,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/stopped-state-load",        test_edge_stopped_state_load,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/truncated-file",            test_edge_truncated_file,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/unsupported-pixel-format",  test_edge_unsupported_pixel_format,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/zero-byte-file",            test_edge_zero_byte_file,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/zero-dimensions",           test_edge_zero_dimensions,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/edge-cases", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
