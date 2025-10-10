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

#include <sail-common/sail-common.h>
#include <sail-manip/sail-manip.h>
#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

/* Test RGB to BGR conversion */
static MunitResult test_conversions_rgb_to_bgr(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    enum SailPixelFormat original_format = image->pixel_format;
    struct sail_image* converted = NULL;

    if (original_format == SAIL_PIXEL_FORMAT_BPP24_RGB || original_format == SAIL_PIXEL_FORMAT_BPP24_BGR) {
        enum SailPixelFormat target = (original_format == SAIL_PIXEL_FORMAT_BPP24_RGB) ?
                                      SAIL_PIXEL_FORMAT_BPP24_BGR : SAIL_PIXEL_FORMAT_BPP24_RGB;

        sail_status_t status = sail_convert_image(image, target, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == target);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test RGBA to RGB conversion (alpha removal) */
static MunitResult test_conversions_rgba_to_rgb(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {

        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test RGB to RGBA conversion (alpha addition) */
static MunitResult test_conversions_rgb_to_rgba(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) {

        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP32_RGBA, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test RGB to Grayscale conversion */
static MunitResult test_conversions_rgb_to_grayscale(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) {

        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test Grayscale to RGB conversion */
static MunitResult test_conversions_grayscale_to_rgb(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test RGB to Indexed conversion */
static MunitResult test_conversions_rgb_to_indexed(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA ||
        image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {

        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            munit_assert_not_null(converted->palette);
            munit_assert(converted->palette->color_count <= 256);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test Indexed to RGB conversion */
static MunitResult test_conversions_indexed_to_rgb(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);

    if (sail_is_indexed(image->pixel_format) && image->palette != NULL) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted);

        munit_assert(status == SAIL_OK);
        munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert(converted->width == image->width);
        munit_assert(converted->height == image->height);
        sail_destroy_image(converted);
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test bit depth upscaling (8->16 bit) */
static MunitResult test_conversions_bit_depth_upscale(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE);
            munit_assert(converted->width == image->width);
            munit_assert(converted->height == image->height);
            sail_destroy_image(converted);
        }
    } else if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP48_RGB, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP48_RGB);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test bit depth downscaling (16->8 bit) */
static MunitResult test_conversions_bit_depth_downscale(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
            sail_destroy_image(converted);
        }
    } else if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP48_RGB) {
        struct sail_image* converted = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted);

        if (status == SAIL_OK) {
            munit_assert(converted->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB);
            sail_destroy_image(converted);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

/* Test conversion round-trip (RGB->Grayscale->RGB) */
static MunitResult test_conversions_roundtrip(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES[0], &image) == SAIL_OK);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB) {
        struct sail_image* gray = NULL;
        sail_status_t status = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, &gray);

        if (status == SAIL_OK) {
            struct sail_image* back_to_rgb = NULL;
            status = sail_convert_image(gray, SAIL_PIXEL_FORMAT_BPP24_RGB, &back_to_rgb);

            if (status == SAIL_OK) {
                munit_assert(back_to_rgb->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB);
                munit_assert(back_to_rgb->width == image->width);
                munit_assert(back_to_rgb->height == image->height);
                sail_destroy_image(back_to_rgb);
            }

            sail_destroy_image(gray);
        }
    }

    sail_destroy_image(image);
    return MUNIT_OK;
}

// clang-format off
static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/bit-depth-downscale",  test_conversions_bit_depth_downscale,  NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/bit-depth-upscale",    test_conversions_bit_depth_upscale,    NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/grayscale-to-rgb",     test_conversions_grayscale_to_rgb,     NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/indexed-to-rgb",       test_conversions_indexed_to_rgb,       NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/rgb-to-bgr",           test_conversions_rgb_to_bgr,           NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/rgb-to-grayscale",     test_conversions_rgb_to_grayscale,     NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/rgb-to-indexed",       test_conversions_rgb_to_indexed,       NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/rgba-to-rgb",          test_conversions_rgba_to_rgb,          NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/rgb-to-rgba",          test_conversions_rgb_to_rgba,          NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
    { (char *)"/roundtrip",            test_conversions_roundtrip,            NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/pixel-conversions", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
