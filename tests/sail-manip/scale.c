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

#include <stdio.h>
#include <string.h>

#include <sail-manip/sail-manip.h>
#include <sail/sail.h>

#include "munit.h"

/* Helper function to create a simple test image with a pattern. */
static sail_status_t create_test_image(unsigned width,
                                       unsigned height,
                                       enum SailPixelFormat pixel_format,
                                       struct sail_image** image_output)
{
    struct sail_image* image = NULL;
    SAIL_TRY(sail_alloc_image(&image));

    image->width          = width;
    image->height         = height;
    image->pixel_format   = pixel_format;
    image->bytes_per_line = sail_bytes_per_line(width, pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    SAIL_TRY(sail_malloc(pixels_size, &image->pixels));

    /* Fill with a simple pattern: each pixel value is row * width + col. */
    const unsigned bytes_per_pixel = sail_bits_per_pixel(pixel_format) / 8;
    uint8_t* pixels                = (uint8_t*)image->pixels;

    for (unsigned row = 0; row < height; row++)
    {
        for (unsigned col = 0; col < width; col++)
        {
            uint8_t value  = (uint8_t)((row * width + col) % 256);
            uint8_t* pixel = pixels + (row * width + col) * bytes_per_pixel;

            for (unsigned b = 0; b < bytes_per_pixel; b++)
            {
                pixel[b] = value;
            }
        }
    }

    *image_output = image;
    return SAIL_OK;
}

static MunitResult test_scale_down(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    original->delay = 50;
    original->gamma = 2.2f;

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 50, 50, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_uint(scaled->width, ==, 50);
        munit_assert_uint(scaled->height, ==, 50);
        munit_assert_int(scaled->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_uint(scaled->delay, ==, 50);
        munit_assert_float(scaled->gamma, ==, 2.2f);
        munit_assert_uint(scaled->bytes_per_line, ==, sail_bytes_per_line(50, SAIL_PIXEL_FORMAT_BPP24_RGB));
        munit_assert_ptr_not_null(scaled->pixels);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_up(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(50, 50, SAIL_PIXEL_FORMAT_BPP32_RGBA, &original), ==, SAIL_OK);

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 200, 200, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_uint(scaled->width, ==, 200);
        munit_assert_uint(scaled->height, ==, 200);
        munit_assert_int(scaled->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);
        munit_assert_uint(scaled->bytes_per_line, ==, sail_bytes_per_line(200, SAIL_PIXEL_FORMAT_BPP32_RGBA));
        munit_assert_ptr_not_null(scaled->pixels);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_aspect_ratio(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    /* Create a 100x50 test image (2:1 aspect ratio). */
    munit_assert_int(create_test_image(100, 50, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        /* Scale to 200x100 (maintaining aspect ratio). */
        munit_assert_int(sail_scale_image(original, 200, 100, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_uint(scaled->width, ==, 200);
        munit_assert_uint(scaled->height, ==, 100);
        munit_assert_int(scaled->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_different_algorithms(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 50, 50, algorithms[i], &scaled), ==, SAIL_OK);
        munit_assert_uint(scaled->width, ==, 50);
        munit_assert_uint(scaled->height, ==, 50);
        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_preserve_properties(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    original->delay = 100;
    original->gamma = 1.8f;

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 50, 50, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_int(scaled->pixel_format, ==, original->pixel_format);
        munit_assert_uint(scaled->delay, ==, original->delay);
        munit_assert_float(scaled->gamma, ==, original->gamma);
        munit_assert_uint(scaled->width, ==, 50);
        munit_assert_uint(scaled->height, ==, 50);
        munit_assert_uint(original->width, ==, 100);
        munit_assert_uint(original->height, ==, 100);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_with_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &original), ==, SAIL_OK);

    struct sail_palette* palette = NULL;
    munit_assert_int(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 256, &palette), ==, SAIL_OK);
    original->palette = palette;

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 50, 50, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_uint(scaled->width, ==, 50);
        munit_assert_uint(scaled->height, ==, 50);
        /* When scaling indexed images, palette may be regenerated during conversion. */
        munit_assert_ptr_not_null(scaled->palette);
        munit_assert_uint(scaled->palette->color_count, >, 0);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_with_iccp(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Create a random ICC profile. */
    struct sail_iccp* iccp = NULL;
    munit_assert_int(sail_alloc_iccp(&iccp), ==, SAIL_OK);

    const size_t iccp_size = 512;
    munit_assert_int(sail_malloc(iccp_size, &iccp->data), ==, SAIL_OK);
    iccp->size = iccp_size;

    /* Fill with random data. */
    uint8_t* iccp_data = (uint8_t*)iccp->data;
    for (size_t i = 0; i < iccp_size; i++)
    {
        iccp_data[i] = (uint8_t)(i % 256);
    }

    original->iccp = iccp;

    /* Test all scaling algorithms. */
    enum SailScaling algorithms[] = {
        SAIL_SCALING_NEAREST_NEIGHBOR,
        SAIL_SCALING_BILINEAR,
        SAIL_SCALING_BICUBIC,
        SAIL_SCALING_LANCZOS
    };

    for (unsigned i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++)
    {
        munit_assert_int(sail_scale_image(original, 50, 50, algorithms[i], &scaled), ==, SAIL_OK);

        munit_assert_uint(scaled->width, ==, 50);
        munit_assert_uint(scaled->height, ==, 50);
        munit_assert_ptr_not_null(scaled->iccp);
        munit_assert_uint(scaled->iccp->size, ==, iccp_size);
        munit_assert_memory_equal(iccp_size, scaled->iccp->data, iccp->data);

        sail_destroy_image(scaled);
    }

    sail_destroy_image(original);

    return MUNIT_OK;
}

static MunitResult test_scale_invalid_dimensions(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* scaled   = NULL;

    munit_assert_int(create_test_image(100, 100, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Try to scale to zero dimensions. */
    munit_assert_int(sail_scale_image(original, 0, 50, SAIL_SCALING_BILINEAR, &scaled), ==,
                     SAIL_ERROR_INVALID_ARGUMENT);
    munit_assert_ptr_null(scaled);

    munit_assert_int(sail_scale_image(original, 50, 0, SAIL_SCALING_BILINEAR, &scaled), ==,
                     SAIL_ERROR_INVALID_ARGUMENT);
    munit_assert_ptr_null(scaled);

    sail_destroy_image(original);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char*)"/scale-down",                 test_scale_down,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-up",                   test_scale_up,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-aspect-ratio",         test_scale_aspect_ratio,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-different-algorithms", test_scale_different_algorithms, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-preserve-properties",  test_scale_preserve_properties,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-with-palette",         test_scale_with_palette,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-with-iccp",            test_scale_with_iccp,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/scale-invalid-dimensions",   test_scale_invalid_dimensions,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char*)"/scale", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
