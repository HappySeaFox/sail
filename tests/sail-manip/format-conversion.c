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

static MunitResult test_grayscale_alpha_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test BPP8_GRAYSCALE_ALPHA */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA));

    /* Test BPP16_GRAYSCALE_ALPHA */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_RGBA, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA));

    /* Test BPP32_GRAYSCALE_ALPHA */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_RGBA, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP48_RGB, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA));

    return MUNIT_OK;
}

static MunitResult test_rgb555_565_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test RGB555 */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_RGB555));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP16_RGB555));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_BGR, SAIL_PIXEL_FORMAT_BPP16_BGR555));

    /* Test RGB565 */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_RGB565));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP16_RGB565));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_BGR, SAIL_PIXEL_FORMAT_BPP16_BGR565));

    return MUNIT_OK;
}

static MunitResult test_cmyk_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test RGB to CMYK32 */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP32_CMYK));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP32_CMYK));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_BGR, SAIL_PIXEL_FORMAT_BPP32_CMYK));

    /* Test RGB to CMYK64 */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP48_RGB, SAIL_PIXEL_FORMAT_BPP64_CMYK));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_RGBA, SAIL_PIXEL_FORMAT_BPP64_CMYK));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP64_CMYK));

    /* Test CMYK64 input */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_CMYK, SAIL_PIXEL_FORMAT_BPP24_RGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_CMYK, SAIL_PIXEL_FORMAT_BPP48_RGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP64_CMYK, SAIL_PIXEL_FORMAT_BPP64_RGBA));

    return MUNIT_OK;
}

static MunitResult test_yuv_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test RGB to YUV24 */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP24_YUV));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP24_YUV));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP48_RGB, SAIL_PIXEL_FORMAT_BPP24_YUV));

    return MUNIT_OK;
}

static MunitResult test_rgba16_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test BPP16_RGBA and variants */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_RGBA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP16_RGBA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_BGR, SAIL_PIXEL_FORMAT_BPP16_BGRA));

    /* Test BPP16_RGBX and variants */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_RGBX));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP16_RGBX));

    /* Test BPP16_ARGB */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_ARGB, SAIL_PIXEL_FORMAT_BPP16_ARGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_ARGB));

    /* Test BPP16_XRGB */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_XRGB, SAIL_PIXEL_FORMAT_BPP16_XRGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP16_XRGB));

    return MUNIT_OK;
}

static MunitResult test_actual_conversion_grayscale_alpha(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image BPP32_RGBA */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 2;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with test data: RGBA */
    uint8_t* pixels = image->pixels;
    pixels[0]       = 255;
    pixels[1]       = 0;
    pixels[2]       = 0;
    pixels[3]       = 255; /* Red, opaque */
    pixels[4]       = 0;
    pixels[5]       = 255;
    pixels[6]       = 0;
    pixels[7]       = 128; /* Green, semi-transparent */
    pixels[8]       = 0;
    pixels[9]       = 0;
    pixels[10]      = 255;
    pixels[11]      = 255; /* Blue, opaque */
    pixels[12]      = 255;
    pixels[13]      = 255;
    pixels[14]      = 255;
    pixels[15]      = 0; /* White, transparent */

    /* Convert to BPP16_GRAYSCALE_ALPHA */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA);
    munit_assert_int(converted_image->width, ==, 2);
    munit_assert_int(converted_image->height, ==, 2);

    /* Verify first pixel is converted correctly (grayscale of red) */
    uint8_t* converted_pixels = converted_image->pixels;
    /* First pixel: Gray value should be around 76 (0.299*255), alpha 255 */
    munit_assert_int(converted_pixels[0], >, 70);
    munit_assert_int(converted_pixels[0], <, 82);
    munit_assert_int(converted_pixels[1], ==, 255);

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

static MunitResult test_actual_conversion_rgb555(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image BPP24_RGB */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 1;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with test data: RGB */
    uint8_t* pixels = image->pixels;
    pixels[0]       = 248;
    pixels[1]       = 0;
    pixels[2]       = 0; /* Red (should be 31 in 5-bit) */
    pixels[3]       = 0;
    pixels[4]       = 252;
    pixels[5]       = 0; /* Green (should be 31 in 5-bit) */

    /* Convert to BPP16_RGB555 */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP16_RGB555, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP16_RGB555);

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

static MunitResult test_actual_conversion_cmyk(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image BPP24_RGB */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 1;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with test data: RGB */
    uint8_t* pixels = image->pixels;
    pixels[0]       = 255;
    pixels[1]       = 0;
    pixels[2]       = 0; /* Pure Red */
    pixels[3]       = 0;
    pixels[4]       = 0;
    pixels[5]       = 0; /* Black */

    /* Convert to BPP32_CMYK */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP32_CMYK, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_CMYK);

    /* Verify CMYK conversion */
    uint8_t* cmyk_pixels = converted_image->pixels;
    /* Pure red: C=0, M=~255, Y=~255, K=0 (may have small rounding errors) */
    munit_assert_int(cmyk_pixels[0], ==, 0);  /* C */
    munit_assert_int(cmyk_pixels[1], >, 250); /* M should be close to 255 */
    munit_assert_int(cmyk_pixels[2], >, 250); /* Y should be close to 255 */
    munit_assert_int(cmyk_pixels[3], ==, 0);  /* K */

    /* Black: C=0, M=0, Y=0, K=255 */
    munit_assert_int(cmyk_pixels[4], ==, 0);   /* C */
    munit_assert_int(cmyk_pixels[5], ==, 0);   /* M */
    munit_assert_int(cmyk_pixels[6], ==, 0);   /* Y */
    munit_assert_int(cmyk_pixels[7], ==, 255); /* K */

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

/* Test floating point format conversions */
static MunitResult test_float_grayscale_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image BPP16_GRAYSCALE */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 2;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with test data */
    uint16_t* pixels = image->pixels;
    pixels[0]        = 0;     /* Black */
    pixels[1]        = 32767; /* Mid gray */
    pixels[2]        = 65535; /* White */
    pixels[3]        = 16383; /* Quarter gray */

    /* Convert to BPP32_GRAYSCALE_FLOAT */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT);
    munit_assert_int(converted_image->width, ==, 2);
    munit_assert_int(converted_image->height, ==, 2);

    /* Verify conversion - check that values are in range [0.0, 1.0] */
    float* float_pixels = converted_image->pixels;
    munit_assert(float_pixels[0] >= 0.0f && float_pixels[0] <= 0.01f);  /* ~0 */
    munit_assert(float_pixels[1] >= 0.49f && float_pixels[1] <= 0.51f); /* ~0.5 */
    munit_assert(float_pixels[2] >= 0.99f && float_pixels[2] <= 1.0f);  /* ~1.0 */
    munit_assert(float_pixels[3] >= 0.24f && float_pixels[3] <= 0.26f); /* ~0.25 */

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

static MunitResult test_float_rgb_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image BPP24_RGB */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 1;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with test data: pure red and pure blue */
    uint8_t* pixels = image->pixels;
    pixels[0]       = 255; /* R */
    pixels[1]       = 0;   /* G */
    pixels[2]       = 0;   /* B */
    pixels[3]       = 0;   /* R */
    pixels[4]       = 0;   /* G */
    pixels[5]       = 255; /* B */

    /* Convert to BPP96_RGB_FLOAT */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT);

    /* Verify conversion */
    float* float_pixels = converted_image->pixels;
    munit_assert(float_pixels[0] >= 0.99f && float_pixels[0] <= 1.0f); /* R = 1.0 */
    munit_assert(float_pixels[1] <= 0.01f);                            /* G = 0.0 */
    munit_assert(float_pixels[2] <= 0.01f);                            /* B = 0.0 */
    munit_assert(float_pixels[3] <= 0.01f);                            /* R = 0.0 */
    munit_assert(float_pixels[4] <= 0.01f);                            /* G = 0.0 */
    munit_assert(float_pixels[5] >= 0.99f && float_pixels[5] <= 1.0f); /* B = 1.0 */

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

static MunitResult test_float_to_integer_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a test image BPP96_RGB_FLOAT */
    struct sail_image* image;
    munit_assert_int(sail_alloc_image(&image), ==, SAIL_OK);

    image->width          = 2;
    image->height         = 1;
    image->pixel_format   = SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT;
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    const size_t pixels_size = (size_t)image->height * image->bytes_per_line;
    munit_assert_int(sail_malloc(pixels_size, &image->pixels), ==, SAIL_OK);

    /* Fill with float data */
    float* pixels = image->pixels;
    pixels[0]     = 1.0f;  /* R */
    pixels[1]     = 0.5f;  /* G */
    pixels[2]     = 0.0f;  /* B */
    pixels[3]     = 0.25f; /* R */
    pixels[4]     = 0.75f; /* G */
    pixels[5]     = 1.0f;  /* B */

    /* Convert to BPP24_RGB */
    struct sail_image* converted_image;
    munit_assert_int(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted_image), ==, SAIL_OK);

    munit_assert_not_null(converted_image);
    munit_assert_int(converted_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    /* Verify conversion */
    uint8_t* uint8_pixels = converted_image->pixels;
    munit_assert_int(uint8_pixels[0], ==, 255);                     /* R = 1.0 -> 255 */
    munit_assert(uint8_pixels[1] >= 127 && uint8_pixels[1] <= 128); /* G = 0.5 -> ~127 */
    munit_assert_int(uint8_pixels[2], ==, 0);                       /* B = 0.0 -> 0 */
    munit_assert(uint8_pixels[3] >= 63 && uint8_pixels[3] <= 64);   /* R = 0.25 -> ~64 */
    munit_assert(uint8_pixels[4] >= 191 && uint8_pixels[4] <= 192); /* G = 0.75 -> ~191 */
    munit_assert_int(uint8_pixels[5], ==, 255);                     /* B = 1.0 -> 255 */

    sail_destroy_image(image);
    sail_destroy_image(converted_image);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/grayscale-alpha",        test_grayscale_alpha_conversion,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgb555-565",             test_rgb555_565_conversion,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/cmyk",                   test_cmyk_conversion,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/yuv",                    test_yuv_conversion,                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rgba16",                 test_rgba16_conversion,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/actual-grayscale-alpha", test_actual_conversion_grayscale_alpha, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/actual-rgb555",          test_actual_conversion_rgb555,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/actual-cmyk",            test_actual_conversion_cmyk,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/float-grayscale",        test_float_grayscale_conversion,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/float-rgb",              test_float_rgb_conversion,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/float-to-integer",       test_float_to_integer_conversion,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/format-conversion", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
