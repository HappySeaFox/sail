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

#include <sail-manip/sail-manip.h>
#include <sail/sail.h>

#include "munit.h"

/*
 * Test conversion TO indexed formats (using Wu quantization)
 */
static MunitResult test_rgb_to_indexed_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test RGB24 to BPP8_INDEXED (256 colors) */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP8_INDEXED));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_BGR, SAIL_PIXEL_FORMAT_BPP8_INDEXED));

    /* Test RGBA32 to BPP8_INDEXED (256 colors) */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP8_INDEXED));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_BGRA, SAIL_PIXEL_FORMAT_BPP8_INDEXED));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBX, SAIL_PIXEL_FORMAT_BPP8_INDEXED));

    /* Test RGB24 to BPP4_INDEXED (16 colors) */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP4_INDEXED));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP32_RGBA, SAIL_PIXEL_FORMAT_BPP4_INDEXED));

    /* Test RGB24 to BPP1_INDEXED (2 colors, monochrome) */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP24_RGB, SAIL_PIXEL_FORMAT_BPP1_INDEXED));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, SAIL_PIXEL_FORMAT_BPP1_INDEXED));

    return MUNIT_OK;
}

/*
 * Test conversion FROM indexed formats back to RGB
 */
static MunitResult test_indexed_to_rgb_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Test BPP8_INDEXED to RGB formats */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP24_RGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP24_BGR));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP32_RGBA));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP8_INDEXED, SAIL_PIXEL_FORMAT_BPP32_BGRA));

    /* Test BPP4_INDEXED to RGB formats */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP24_RGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP4_INDEXED, SAIL_PIXEL_FORMAT_BPP32_RGBA));

    /* Test BPP1_INDEXED to RGB formats */
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP24_RGB));
    munit_assert_true(sail_can_convert(SAIL_PIXEL_FORMAT_BPP1_INDEXED, SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE));

    return MUNIT_OK;
}

/*
 * Test actual RGB -> Indexed -> RGB round-trip conversion
 */
static MunitResult test_indexed_roundtrip(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a simple test image 16x16 RGB24 */
    struct sail_image* rgb_image = NULL;
    munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
    rgb_image->width = 16;
    rgb_image->height = 16;
    rgb_image->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);

    munit_assert_int(sail_malloc((size_t)rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==,
                     SAIL_OK);

    /* Fill with gradient pattern */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (unsigned char)(x * 16); /* R */
            pixels[idx + 1] = (unsigned char)(y * 16); /* G */
            pixels[idx + 2] = 128;                     /* B */
        }
    }

    /* Convert RGB -> BPP8_INDEXED (256 colors) */
    struct sail_image* indexed_image = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &indexed_image), ==, SAIL_OK);

    /* Verify indexed image properties */
    munit_assert_int(indexed_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP8_INDEXED);
    munit_assert_not_null(indexed_image->palette);
    munit_assert_int(indexed_image->palette->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);
    munit_assert_true(indexed_image->palette->color_count > 0);
    munit_assert_true(indexed_image->palette->color_count <= 256);

    /* Convert back: BPP8_INDEXED -> RGB24 */
    struct sail_image* rgb_back_image = NULL;
    munit_assert_int(sail_convert_image(indexed_image, SAIL_PIXEL_FORMAT_BPP24_RGB, &rgb_back_image), ==, SAIL_OK);

    /* Verify RGB back image */
    munit_assert_int(rgb_back_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_int(rgb_back_image->width, ==, 16);
    munit_assert_int(rgb_back_image->height, ==, 16);

    /* Cleanup */
    sail_destroy_image(rgb_image);
    sail_destroy_image(indexed_image);
    sail_destroy_image(rgb_back_image);

    return MUNIT_OK;
}

/*
 * Test quantization with different color counts
 */
static MunitResult test_indexed_color_counts(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a colorful test image 32x32 RGB24 */
    struct sail_image* rgb_image = NULL;
    munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
    rgb_image->width = 32;
    rgb_image->height = 32;
    rgb_image->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);

    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with colorful pattern */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (unsigned char)(x * 8);
            pixels[idx + 1] = (unsigned char)(y * 8);
            pixels[idx + 2] = (unsigned char)((x + y) * 4);
        }
    }

    /* Test BPP8_INDEXED (up to 256 colors) */
    struct sail_image* indexed256 = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &indexed256), ==, SAIL_OK);
    munit_assert_not_null(indexed256->palette);
    munit_assert_true(indexed256->palette->color_count > 0);
    munit_assert_true(indexed256->palette->color_count <= 256);

    /* Test BPP4_INDEXED (up to 16 colors) */
    struct sail_image* indexed16 = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &indexed16), ==, SAIL_OK);
    munit_assert_not_null(indexed16->palette);
    munit_assert_true(indexed16->palette->color_count > 0);
    munit_assert_true(indexed16->palette->color_count <= 16);

    /* Test BPP1_INDEXED (2 colors) */
    struct sail_image* indexed2 = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &indexed2), ==, SAIL_OK);
    munit_assert_not_null(indexed2->palette);
    munit_assert_true(indexed2->palette->color_count > 0);
    munit_assert_true(indexed2->palette->color_count <= 2);

    /* Cleanup */
    sail_destroy_image(rgb_image);
    sail_destroy_image(indexed256);
    sail_destroy_image(indexed16);
    sail_destroy_image(indexed2);

    return MUNIT_OK;
}

/*
 * Test re-quantization (indexed to indexed conversion)
 */
static MunitResult test_indexed_requantization(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create RGB image */
    struct sail_image* rgb_image = NULL;
    munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
    rgb_image->width = 16;
    rgb_image->height = 16;
    rgb_image->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with simple colors */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (x < 8) ? 255 : 0;        /* R */
            pixels[idx + 1] = (y < 8) ? 255 : 0;        /* G */
            pixels[idx + 2] = ((x + y) < 16) ? 255 : 0; /* B */
        }
    }

    /* Convert to BPP8_INDEXED (256 colors) */
    struct sail_image* indexed256 = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &indexed256), ==, SAIL_OK);

    /* Re-quantize to BPP4_INDEXED (16 colors) */
    struct sail_image* indexed16 = NULL;
    munit_assert_int(sail_convert_image(indexed256, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &indexed16), ==, SAIL_OK);
    munit_assert_true(indexed16->palette->color_count <= 16);

    /* Re-quantize to BPP1_INDEXED (2 colors) */
    struct sail_image* indexed2 = NULL;
    munit_assert_int(sail_convert_image(indexed16, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &indexed2), ==, SAIL_OK);
    munit_assert_true(indexed2->palette->color_count <= 2);

    /* Cleanup */
    sail_destroy_image(rgb_image);
    sail_destroy_image(indexed256);
    sail_destroy_image(indexed16);
    sail_destroy_image(indexed2);

    return MUNIT_OK;
}

/*
 * Test Floyd-Steinberg dithering
 */
static MunitResult test_floyd_steinberg_dithering(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create a gradient test image 64x64 RGB24 */
    struct sail_image* rgb_image = NULL;
    munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
    rgb_image->width = 64;
    rgb_image->height = 64;
    rgb_image->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with smooth gradient (good for testing dithering) */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (unsigned char)(x * 4); /* R: 0-252 gradient */
            pixels[idx + 1] = (unsigned char)(y * 4); /* G: 0-252 gradient */
            pixels[idx + 2] = 128;                    /* B: constant */
        }
    }

    /* Quantize without dithering */
    struct sail_image* indexed_no_dither = NULL;
    munit_assert_int(sail_quantize_image(rgb_image, 16, false, &indexed_no_dither), ==, SAIL_OK);
    munit_assert_int(indexed_no_dither->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP4_INDEXED);

    /* Quantize with dithering */
    struct sail_image* indexed_with_dither = NULL;
    munit_assert_int(sail_quantize_image(rgb_image, 16, true, &indexed_with_dither), ==, SAIL_OK);
    munit_assert_int(indexed_with_dither->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP4_INDEXED);

    /* Both should have the same palette size */
    munit_assert_int(indexed_no_dither->palette->color_count, ==, indexed_with_dither->palette->color_count);

    /* Dithered image should have different pixel data (error diffusion changes indices) */
    /* Note: We can't easily verify quality improvement, but we can check it ran without errors */

    /* Cleanup */
    sail_destroy_image(rgb_image);
    sail_destroy_image(indexed_no_dither);
    sail_destroy_image(indexed_with_dither);

    return MUNIT_OK;
}

// clang-format off
static MunitTest tests[] = {
    { (char *)"/rgb-to-indexed",            test_rgb_to_indexed_conversion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-to-rgb",            test_indexed_to_rgb_conversion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-roundtrip",         test_indexed_roundtrip,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-color-counts",      test_indexed_color_counts,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-requantization",    test_indexed_requantization,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/floyd-steinberg-dithering", test_floyd_steinberg_dithering, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL,                                 NULL,                           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    (char *)"/indexed-conversion",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[]) {
    return munit_suite_main(&suite, NULL, argc, argv);
}

