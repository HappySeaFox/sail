/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <sail/sail.h>

#include "munit.h"

/*
 * Tests for the max_width, max_height, and max_pixels load options shared between test suites.
 * They take the image path from the "path" test parameter.
 */

/*
 * Codecs that reject too large images in load_init() before allocating memory for them
 * or decoding them. Without that early check, the final check in SAIL would still reject
 * the images, so these tests check where the rejection happens.
 *
 * RAW skips the early check for half size processing and for Fuji SuperCCD images, so SAIL
 * rejects such images later in sail_load_next_frame(). These tests don't enable half size
 * processing and don't run on Fuji SuperCCD images. The edge cases suite checks
 * edge-cases/fuji-superccd.raf separately.
 */
static const char* const dimension_limits_codecs_rejecting_in_load_init[] = {
    "AVIF", "FLI", "GIF", "JPEG", "PNG", "RAW", "WEBP", NULL,
};

static inline bool dimension_limits_codec_rejects_in_load_init(const struct sail_codec_info* codec_info)
{
    for (const char* const* name = dimension_limits_codecs_rejecting_in_load_init; *name != NULL; name++)
    {
        if (strcmp(codec_info->name, *name) == 0)
        {
            return true;
        }
    }

    return false;
}

/* Fetches the dimensions of the first frame. Returns false if the image cannot be probed. */
static inline bool dimension_limits_probe_dimensions(const char* path, unsigned* width, unsigned* height)
{
    struct sail_image* image = NULL;

    if (sail_probe_file(path, &image, NULL) != SAIL_OK)
    {
        return false;
    }

    *width  = image->width;
    *height = image->height;

    sail_destroy_image(image);

    return true;
}

/* Probes the image with the specified dimension limits. */
static inline sail_status_t dimension_limits_probe(const char* path,
                                                   const struct sail_codec_info* codec_info,
                                                   unsigned max_width,
                                                   unsigned max_height,
                                                   uint64_t max_pixels)
{
    struct sail_io* io;
    munit_assert(sail_alloc_io_read_file(path, &io) == SAIL_OK);

    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);
    load_options->max_width  = max_width;
    load_options->max_height = max_height;
    load_options->max_pixels = max_pixels;

    struct sail_image* image   = NULL;
    const sail_status_t status = sail_probe_io_with_options(io, codec_info, load_options, &image);

    sail_destroy_image(image);
    sail_destroy_load_options(load_options);
    sail_destroy_io(io);

    return status;
}

/*
 * Loads the first frame of the image with the specified dimension limits.
 * Sets failed_in_start to true if starting the loading failed.
 */
static inline sail_status_t dimension_limits_load(const char* path,
                                                  const struct sail_codec_info* codec_info,
                                                  unsigned max_width,
                                                  unsigned max_height,
                                                  uint64_t max_pixels,
                                                  bool* failed_in_start)
{
    struct sail_load_options* load_options;
    munit_assert(sail_alloc_load_options_from_features(codec_info->load_features, &load_options) == SAIL_OK);
    load_options->max_width  = max_width;
    load_options->max_height = max_height;
    load_options->max_pixels = max_pixels;

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_file_with_options(path, codec_info, load_options, &state);

    *failed_in_start = (status != SAIL_OK);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);
        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);
    sail_destroy_load_options(load_options);

    return status;
}

/* Checks that loading with the specified dimension limits rejects the image as early as expected. */
static inline void dimension_limits_assert_load_rejected(const char* path,
                                                         const struct sail_codec_info* codec_info,
                                                         unsigned max_width,
                                                         unsigned max_height,
                                                         uint64_t max_pixels)
{
    bool failed_in_start;

    munit_assert_int(dimension_limits_load(path, codec_info, max_width, max_height, max_pixels, &failed_in_start), ==,
                     SAIL_ERROR_IMAGE_TOO_LARGE);

    if (dimension_limits_codec_rejects_in_load_init(codec_info))
    {
        munit_assert_true(failed_in_start);
    }
}

/* Test probing honors dimension limits */
static inline MunitResult test_dimension_limits_probe(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    unsigned width;
    unsigned height;

    if (!dimension_limits_probe_dimensions(path, &width, &height))
    {
        return MUNIT_SKIP;
    }

    const uint64_t pixels = (uint64_t)width * height;

    munit_assert_int(dimension_limits_probe(path, codec_info, 0, 0, 0), ==, SAIL_OK);
    munit_assert_int(dimension_limits_probe(path, codec_info, width, height, pixels), ==, SAIL_OK);

    /* 0 means no limit, so 1 pixel wide or tall images cannot exceed a limit. */
    if (width > 1)
    {
        munit_assert_int(dimension_limits_probe(path, codec_info, width - 1, 0, 0), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }
    if (height > 1)
    {
        munit_assert_int(dimension_limits_probe(path, codec_info, 0, height - 1, 0), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }
    if (pixels > 1)
    {
        munit_assert_int(dimension_limits_probe(path, codec_info, 0, 0, pixels - 1), ==, SAIL_ERROR_IMAGE_TOO_LARGE);
    }

    return MUNIT_OK;
}

/* Test loading honors dimension limits */
static inline MunitResult test_dimension_limits_load(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    unsigned width;
    unsigned height;

    if (!dimension_limits_probe_dimensions(path, &width, &height))
    {
        return MUNIT_SKIP;
    }

    const uint64_t pixels = (uint64_t)width * height;

    bool failed_in_start;
    munit_assert_int(dimension_limits_load(path, codec_info, 0, 0, 0, &failed_in_start), ==, SAIL_OK);
    munit_assert_int(dimension_limits_load(path, codec_info, width, height, pixels, &failed_in_start), ==, SAIL_OK);

    /* 0 means no limit, so 1 pixel wide or tall images cannot exceed a limit. */
    if (width > 1)
    {
        dimension_limits_assert_load_rejected(path, codec_info, width - 1, 0, 0);
    }
    if (height > 1)
    {
        dimension_limits_assert_load_rejected(path, codec_info, 0, height - 1, 0);
    }
    if (pixels > 1)
    {
        dimension_limits_assert_load_rejected(path, codec_info, 0, 0, pixels - 1);
    }

    return MUNIT_OK;
}
