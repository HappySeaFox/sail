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
    rgb_image->width          = 16;
    rgb_image->height         = 16;
    rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);

    munit_assert_int(sail_malloc((size_t)rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==,
                     SAIL_OK);

    /* Fill with gradient pattern */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx    = (y * rgb_image->bytes_per_line) + (x * 3);
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
    munit_assert_int(indexed_image->palette->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);
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
    rgb_image->width          = 32;
    rgb_image->height         = 32;
    rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);

    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with colorful pattern */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx    = (y * rgb_image->bytes_per_line) + (x * 3);
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
    rgb_image->width          = 16;
    rgb_image->height         = 16;
    rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with simple colors */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx    = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (x < 8) ? 255 : 0;        /* R */
            pixels[idx + 1] = (y < 8) ? 255 : 0;        /* G */
            pixels[idx + 2] = ((x + y) < 16) ? 255 : 0; /* B */
        }
    }

    /* Convert to BPP8_INDEXED (256 colors) */
    struct sail_image* indexed256 = NULL;
    munit_assert_int(sail_convert_image(rgb_image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, &indexed256), ==, SAIL_OK);
    munit_assert_int(indexed256->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP8_INDEXED);

    /* Re-quantize to BPP4_INDEXED (16 colors) */
    struct sail_image* indexed16 = NULL;
    munit_assert_int(sail_convert_image(indexed256, SAIL_PIXEL_FORMAT_BPP4_INDEXED, &indexed16), ==, SAIL_OK);
    munit_assert_int(indexed16->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP4_INDEXED);
    munit_assert_true(indexed16->palette->color_count <= 16);

    /* Re-quantize to BPP1_INDEXED (2 colors) */
    struct sail_image* indexed2 = NULL;
    munit_assert_int(sail_convert_image(indexed16, SAIL_PIXEL_FORMAT_BPP1_INDEXED, &indexed2), ==, SAIL_OK);
    munit_assert_int(indexed2->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP1_INDEXED);
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
    rgb_image->width          = 64;
    rgb_image->height         = 64;
    rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    /* Fill with smooth gradient (good for testing dithering) */
    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx    = (y * rgb_image->bytes_per_line) + (x * 3);
            pixels[idx + 0] = (unsigned char)(x * 4); /* R: 0-252 gradient */
            pixels[idx + 1] = (unsigned char)(y * 4); /* G: 0-252 gradient */
            pixels[idx + 2] = 128;                    /* B: constant */
        }
    }

    /* Quantize without dithering */
    struct sail_image* indexed_no_dither = NULL;
    munit_assert_int(sail_quantize_image(rgb_image, SAIL_PIXEL_FORMAT_BPP4_INDEXED, false, &indexed_no_dither), ==,
                     SAIL_OK);
    munit_assert_int(indexed_no_dither->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP4_INDEXED);

    /* Quantize with dithering */
    struct sail_image* indexed_with_dither = NULL;
    munit_assert_int(sail_quantize_image(rgb_image, SAIL_PIXEL_FORMAT_BPP4_INDEXED, true, &indexed_with_dither), ==,
                     SAIL_OK);
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

/*
 * Test that output format always matches requested format regardless of color count.
 * This is the fix for the issue where BPP8_INDEXED was incorrectly converted to
 * BPP1_INDEXED when the image had few colors.
 */
static MunitResult test_output_format_matches_request(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct
    {
        unsigned color_count;    /* Number of unique colors in test image */
        const char* description; /* Test description */
    } test_images[] = {
        {2, "2 colors (black & white)"},
        {3, "3 colors"},
        {5, "5 colors"},
        {8, "8 colors"},
        {15, "15 colors"},
        {20, "20 colors"},
        {100, "100 colors"},
    };

    enum SailPixelFormat requested_formats[] = {
        SAIL_PIXEL_FORMAT_BPP1_INDEXED,
        SAIL_PIXEL_FORMAT_BPP2_INDEXED,
        SAIL_PIXEL_FORMAT_BPP4_INDEXED,
        SAIL_PIXEL_FORMAT_BPP8_INDEXED,
    };

    const char* format_names[] = {
        "BPP1_INDEXED",
        "BPP2_INDEXED",
        "BPP4_INDEXED",
        "BPP8_INDEXED",
    };

    unsigned max_colors_for_format[] = {2, 4, 16, 256};

    for (unsigned img = 0; img < sizeof(test_images) / sizeof(test_images[0]); img++)
    {
        /* Create RGB image with specific number of colors */
        struct sail_image* rgb_image = NULL;
        munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
        rgb_image->width          = 16;
        rgb_image->height         = 16;
        rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
        rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
        munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

        /* Fill with exactly N colors */
        unsigned char* pixels = (unsigned char*)rgb_image->pixels;
        unsigned total_pixels = rgb_image->width * rgb_image->height;
        for (unsigned i = 0; i < total_pixels; i++)
        {
            unsigned color_idx = i % test_images[img].color_count;
            unsigned y         = i / rgb_image->width;
            unsigned x         = i % rgb_image->width;
            unsigned idx       = (y * rgb_image->bytes_per_line) + (x * 3);

            /* Generate distinct colors */
            pixels[idx + 0] = (unsigned char)((color_idx * 50) % 256);        /* R */
            pixels[idx + 1] = (unsigned char)((color_idx * 100 + 50) % 256);  /* G */
            pixels[idx + 2] = (unsigned char)((color_idx * 150 + 100) % 256); /* B */
        }

        /* Test quantization to each indexed format */
        for (unsigned fmt = 0; fmt < sizeof(requested_formats) / sizeof(requested_formats[0]); fmt++)
        {
            /* Skip if image has more colors than format can hold */
            if (test_images[img].color_count > max_colors_for_format[fmt])
            {
                continue;
            }

            struct sail_image* indexed_image = NULL;
            sail_status_t status = sail_quantize_image(rgb_image, requested_formats[fmt], false, &indexed_image);
            munit_assert_int(status, ==, SAIL_OK);

            /* The key assertion: output format MUST match requested format */
            if (indexed_image->pixel_format != requested_formats[fmt])
            {
                printf("\nFAILED: Image with %s requested %s but got %s\n", test_images[img].description,
                       format_names[fmt], sail_pixel_format_to_string(indexed_image->pixel_format));
                sail_destroy_image(indexed_image);
                sail_destroy_image(rgb_image);
                munit_assert_int(indexed_image->pixel_format, ==, requested_formats[fmt]);
            }

            /* Verify palette exists and has reasonable color count */
            munit_assert_not_null(indexed_image->palette);
            munit_assert_true(indexed_image->palette->color_count <= max_colors_for_format[fmt]);
            munit_assert_true(indexed_image->palette->color_count >= 1);

            sail_destroy_image(indexed_image);
        }

        sail_destroy_image(rgb_image);
    }

    return MUNIT_OK;
}

/*
 * Test edge case: image with 2 colors must still output as BPP8 when requested
 */
static MunitResult test_few_colors_bpp8_output(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Create RGB image with only 2 colors (black and white checkerboard) */
    struct sail_image* rgb_image = NULL;
    munit_assert_int(sail_alloc_image(&rgb_image), ==, SAIL_OK);
    rgb_image->width          = 10;
    rgb_image->height         = 10;
    rgb_image->pixel_format   = SAIL_PIXEL_FORMAT_BPP24_RGB;
    rgb_image->bytes_per_line = sail_bytes_per_line(rgb_image->width, rgb_image->pixel_format);
    munit_assert_int(sail_malloc(rgb_image->bytes_per_line * rgb_image->height, &rgb_image->pixels), ==, SAIL_OK);

    unsigned char* pixels = (unsigned char*)rgb_image->pixels;
    for (unsigned y = 0; y < rgb_image->height; y++)
    {
        for (unsigned x = 0; x < rgb_image->width; x++)
        {
            unsigned idx        = (y * rgb_image->bytes_per_line) + (x * 3);
            unsigned char color = ((x + y) % 2 == 0) ? 0 : 255;
            pixels[idx + 0]     = color; /* R */
            pixels[idx + 1]     = color; /* G */
            pixels[idx + 2]     = color; /* B */
        }
    }

    /* Request BPP8_INDEXED even though image has only 2 colors */
    struct sail_image* indexed_image = NULL;
    munit_assert_int(sail_quantize_image(rgb_image, SAIL_PIXEL_FORMAT_BPP8_INDEXED, false, &indexed_image), ==,
                     SAIL_OK);

    /* Must be BPP8_INDEXED, not BPP1_INDEXED */
    munit_assert_int(indexed_image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP8_INDEXED);

    /* Palette should have 2 colors */
    munit_assert_not_null(indexed_image->palette);
    munit_assert_int(indexed_image->palette->color_count, ==, 2);

    sail_destroy_image(rgb_image);
    sail_destroy_image(indexed_image);

    return MUNIT_OK;
}

// clang-format off
static MunitTest tests[] = {
    { (char *)"/rgb-to-indexed",                test_rgb_to_indexed_conversion,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-to-rgb",                test_indexed_to_rgb_conversion,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-roundtrip",             test_indexed_roundtrip,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-color-counts",          test_indexed_color_counts,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/indexed-requantization",        test_indexed_requantization,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/floyd-steinberg-dithering",     test_floyd_steinberg_dithering,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/output-format-matches-request", test_output_format_matches_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/few-colors-bpp8-output",        test_few_colors_bpp8_output,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    (char *)"/indexed-conversion", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
}

