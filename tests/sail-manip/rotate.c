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

/* Helper function to create a simple test image with a pattern */
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

    /* Allocate pixels */
    const size_t pixels_size = image->height * image->bytes_per_line;
    SAIL_TRY(sail_malloc(pixels_size, &image->pixels));

    /* Fill with a simple pattern: each pixel value is row * width + col */
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

static MunitResult test_rotate_90(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* rotated  = NULL;

    /* Create a 4x3 test image */
    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Rotate 90 degrees clockwise */
    munit_assert_int(sail_rotate_image(original, SAIL_ORIENTATION_ROTATED_90, &rotated), ==, SAIL_OK);

    /* Check dimensions are swapped */
    munit_assert_uint(rotated->width, ==, 3);
    munit_assert_uint(rotated->height, ==, 4);
    munit_assert_int(rotated->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    /* Verify pixel transformation: new[x][y] = old[height-1-y][x] */
    const unsigned bytes_per_pixel = 3;
    const uint8_t* orig_pixels     = (const uint8_t*)original->pixels;
    const uint8_t* rot_pixels      = (const uint8_t*)rotated->pixels;

    /* Check a few key pixels */
    /* Original [0][0] should be at rotated [0][2] */
    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_02  = rot_pixels + (0 * 3 + 2) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_02);

    /* Original [0][3] should be at rotated [3][2] */
    const uint8_t* orig_03 = orig_pixels + (0 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_32  = rot_pixels + (3 * 3 + 2) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_03, rot_32);

    sail_destroy_image(original);
    sail_destroy_image(rotated);

    return MUNIT_OK;
}

static MunitResult test_rotate_180(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* rotated  = NULL;

    /* Create a 4x3 test image */
    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP32_RGBA, &original), ==, SAIL_OK);

    /* Rotate 180 degrees */
    munit_assert_int(sail_rotate_image(original, SAIL_ORIENTATION_ROTATED_180, &rotated), ==, SAIL_OK);

    /* Check dimensions remain the same */
    munit_assert_uint(rotated->width, ==, 4);
    munit_assert_uint(rotated->height, ==, 3);
    munit_assert_int(rotated->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);

    /* Verify pixel transformation: new[x][y] = old[width-1-x][height-1-y] */
    const unsigned bytes_per_pixel = 4;
    const uint8_t* orig_pixels     = (const uint8_t*)original->pixels;
    const uint8_t* rot_pixels      = (const uint8_t*)rotated->pixels;

    /* Original [0][0] should be at rotated [3][2] (bottom-right) */
    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_23  = rot_pixels + (2 * 4 + 3) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_23);

    /* Original [2][3] (bottom-right) should be at rotated [0][0] (top-left) */
    const uint8_t* orig_23 = orig_pixels + (2 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_00  = rot_pixels + (0 * 4 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_23, rot_00);

    sail_destroy_image(original);
    sail_destroy_image(rotated);

    return MUNIT_OK;
}

static MunitResult test_rotate_270(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* rotated  = NULL;

    /* Create a 4x3 test image */
    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Rotate 270 degrees clockwise (90 CCW) */
    munit_assert_int(sail_rotate_image(original, SAIL_ORIENTATION_ROTATED_270, &rotated), ==, SAIL_OK);

    /* Check dimensions are swapped */
    munit_assert_uint(rotated->width, ==, 3);
    munit_assert_uint(rotated->height, ==, 4);
    munit_assert_int(rotated->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    /* Verify pixel transformation: new[x][y] = old[y][width-1-x] */
    const unsigned bytes_per_pixel = 3;
    const uint8_t* orig_pixels     = (const uint8_t*)original->pixels;
    const uint8_t* rot_pixels      = (const uint8_t*)rotated->pixels;

    /* Original [0][0] should be at rotated [3][0] */
    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_30  = rot_pixels + (3 * 3 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_30);

    /* Original [0][3] should be at rotated [0][0] */
    const uint8_t* orig_03 = orig_pixels + (0 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_00  = rot_pixels + (0 * 3 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_03, rot_00);

    sail_destroy_image(original);
    sail_destroy_image(rotated);

    return MUNIT_OK;
}

static MunitResult test_rotate_180_inplace(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* image     = NULL;
    struct sail_image* reference = NULL;

    /* Create a 4x3 test image */
    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP32_RGBA, &image), ==, SAIL_OK);

    /* Create reference for comparison */
    munit_assert_int(sail_rotate_image(image, SAIL_ORIENTATION_ROTATED_180, &reference), ==, SAIL_OK);

    /* Rotate 180 degrees in-place */
    munit_assert_int(sail_rotate_image_180_inplace(image), ==, SAIL_OK);

    /* Check dimensions remain the same */
    munit_assert_uint(image->width, ==, 4);
    munit_assert_uint(image->height, ==, 3);
    munit_assert_int(image->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);

    /* Compare with reference */
    const size_t image_size = image->height * image->bytes_per_line;
    munit_assert_memory_equal(image_size, image->pixels, reference->pixels);

    sail_destroy_image(image);
    sail_destroy_image(reference);

    return MUNIT_OK;
}

static MunitResult test_rotate_with_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* rotated  = NULL;

    /* Create a test image */
    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Add a palette */
    struct sail_palette* palette = NULL;
    munit_assert_int(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 16, &palette), ==, SAIL_OK);
    original->palette = palette;

    /* Rotate */
    munit_assert_int(sail_rotate_image(original, SAIL_ORIENTATION_ROTATED_90, &rotated), ==, SAIL_OK);

    /* Verify palette was copied */
    munit_assert_ptr_not_null(rotated->palette);
    munit_assert_uint(rotated->palette->color_count, ==, 16);
    munit_assert_int(rotated->palette->pixel_format, ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    sail_destroy_image(original);
    sail_destroy_image(rotated);

    return MUNIT_OK;
}

static MunitResult test_rotate_invalid_angle(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_image* original = NULL;
    struct sail_image* rotated  = NULL;

    munit_assert_int(create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB, &original), ==, SAIL_OK);

    /* Try to rotate with invalid angle */
    munit_assert_int(sail_rotate_image(original, SAIL_ORIENTATION_NORMAL, &rotated), ==, SAIL_ERROR_INVALID_ARGUMENT);
    munit_assert_ptr_null(rotated);

    sail_destroy_image(original);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char*)"/rotate-90",            test_rotate_90,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/rotate-180",           test_rotate_180,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/rotate-270",           test_rotate_270,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/rotate-180-inplace",   test_rotate_180_inplace,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/rotate-with-palette",  test_rotate_with_palette,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*)"/rotate-invalid-angle", test_rotate_invalid_angle, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char*)"/rotate", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
