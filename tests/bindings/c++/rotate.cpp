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

#include <cstring>

#include <sail-c++/sail-c++.h>

#include "munit.h"

static sail::image create_test_image(unsigned width, unsigned height, SailPixelFormat pixel_format)
{
    sail::image image(pixel_format, width, height);

    const unsigned bytes_per_pixel = sail::image::bits_per_pixel(pixel_format) / 8;
    uint8_t* pixels                = static_cast<uint8_t*>(image.pixels());

    for (unsigned row = 0; row < height; row++)
    {
        for (unsigned col = 0; col < width; col++)
        {
            uint8_t value  = static_cast<uint8_t>((row * width + col) % 256);
            uint8_t* pixel = pixels + (row * width + col) * bytes_per_pixel;

            for (unsigned b = 0; b < bytes_per_pixel; b++)
            {
                pixel[b] = value;
            }
        }
    }

    return image;
}

/* Tests 90 degrees clockwise rotation. */
static MunitResult test_rotate_90(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_true(original.is_valid());

    sail::image rotated = original.rotate_to(SAIL_ORIENTATION_ROTATED_90);
    munit_assert_true(rotated.is_valid());

    munit_assert_uint(rotated.width(), ==, 3);
    munit_assert_uint(rotated.height(), ==, 4);
    munit_assert_int(rotated.pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    const unsigned bytes_per_pixel = 3;
    const uint8_t* orig_pixels     = static_cast<const uint8_t*>(original.pixels());
    const uint8_t* rot_pixels      = static_cast<const uint8_t*>(rotated.pixels());

    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_02  = rot_pixels + (0 * 3 + 2) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_02);

    const uint8_t* orig_03 = orig_pixels + (0 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_32  = rot_pixels + (3 * 3 + 2) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_03, rot_32);

    return MUNIT_OK;
}

/* Tests 180 degrees rotation. */
static MunitResult test_rotate_180(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP32_RGBA);
    munit_assert_true(original.is_valid());

    sail::image rotated = original.rotate_to(SAIL_ORIENTATION_ROTATED_180);
    munit_assert_true(rotated.is_valid());

    munit_assert_uint(rotated.width(), ==, 4);
    munit_assert_uint(rotated.height(), ==, 3);
    munit_assert_int(rotated.pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);

    const unsigned bytes_per_pixel = 4;
    const uint8_t* orig_pixels     = static_cast<const uint8_t*>(original.pixels());
    const uint8_t* rot_pixels      = static_cast<const uint8_t*>(rotated.pixels());

    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_23  = rot_pixels + (2 * 4 + 3) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_23);

    const uint8_t* orig_23 = orig_pixels + (2 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_00  = rot_pixels + (0 * 4 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_23, rot_00);

    return MUNIT_OK;
}

/* Tests 270 degrees clockwise rotation. */
static MunitResult test_rotate_270(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_true(original.is_valid());

    sail::image rotated = original.rotate_to(SAIL_ORIENTATION_ROTATED_270);
    munit_assert_true(rotated.is_valid());

    munit_assert_uint(rotated.width(), ==, 3);
    munit_assert_uint(rotated.height(), ==, 4);
    munit_assert_int(rotated.pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    const unsigned bytes_per_pixel = 3;
    const uint8_t* orig_pixels     = static_cast<const uint8_t*>(original.pixels());
    const uint8_t* rot_pixels      = static_cast<const uint8_t*>(rotated.pixels());

    const uint8_t* orig_00 = orig_pixels + (0 * 4 + 0) * bytes_per_pixel;
    const uint8_t* rot_30  = rot_pixels + (3 * 3 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_00, rot_30);

    const uint8_t* orig_03 = orig_pixels + (0 * 4 + 3) * bytes_per_pixel;
    const uint8_t* rot_00  = rot_pixels + (0 * 3 + 0) * bytes_per_pixel;
    munit_assert_memory_equal(bytes_per_pixel, orig_03, rot_00);

    return MUNIT_OK;
}

/* Tests in-place rotation using rotate() method. */
static MunitResult test_rotate_inplace(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image image = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP32_RGBA);
    munit_assert_true(image.is_valid());

    sail::image reference = image.rotate_to(SAIL_ORIENTATION_ROTATED_90);

    munit_assert_int(image.rotate(SAIL_ORIENTATION_ROTATED_90), ==, SAIL_OK);

    munit_assert_uint(image.width(), ==, 3);
    munit_assert_uint(image.height(), ==, 4);
    munit_assert_int(image.pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP32_RGBA);

    const size_t image_size = image.height() * image.bytes_per_line();
    munit_assert_memory_equal(image_size, image.pixels(), reference.pixels());

    return MUNIT_OK;
}

/* Tests rotate_to() method with output parameter. */
static MunitResult test_rotate_to_output(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_true(original.is_valid());

    sail::image rotated;
    munit_assert_int(original.rotate_to(SAIL_ORIENTATION_ROTATED_180, &rotated), ==, SAIL_OK);
    munit_assert_true(rotated.is_valid());

    munit_assert_uint(rotated.width(), ==, 4);
    munit_assert_uint(rotated.height(), ==, 3);
    munit_assert_int(rotated.pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    munit_assert_uint(original.width(), ==, 4);
    munit_assert_uint(original.height(), ==, 3);

    return MUNIT_OK;
}

/* Tests that palette is properly copied during rotation. */
static MunitResult test_rotate_with_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_true(original.is_valid());

    sail::arbitrary_data palette_data(16 * 3);
    for (std::size_t i = 0; i < palette_data.size(); i++)
    {
        palette_data[i] = static_cast<uint8_t>(i);
    }
    sail::palette palette(SAIL_PIXEL_FORMAT_BPP24_RGB, palette_data);
    original.set_palette(palette);

    munit_assert_uint(original.palette().color_count(), ==, 16);
    munit_assert_true(original.palette().is_valid());

    sail::image rotated = original.rotate_to(SAIL_ORIENTATION_ROTATED_90);
    munit_assert_true(rotated.is_valid());

    munit_assert_true(rotated.palette().is_valid());
    munit_assert_uint(rotated.palette().color_count(), ==, 16);
    munit_assert_int(rotated.palette().pixel_format(), ==, SAIL_PIXEL_FORMAT_BPP24_RGB);

    return MUNIT_OK;
}

/* Tests that four 90 degree rotations return the original image. */
static MunitResult test_rotate_multiple_times(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image original = create_test_image(4, 3, SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_true(original.is_valid());

    const size_t original_size    = original.pixels_size();
    uint8_t* original_pixels_copy = new uint8_t[original_size];
    std::memcpy(original_pixels_copy, original.pixels(), original_size);

    sail::image rotated = original;
    for (int i = 0; i < 4; i++)
    {
        munit_assert_int(rotated.rotate(SAIL_ORIENTATION_ROTATED_90), ==, SAIL_OK);
    }

    munit_assert_uint(rotated.width(), ==, 4);
    munit_assert_uint(rotated.height(), ==, 3);

    munit_assert_memory_equal(original_size, original_pixels_copy, rotated.pixels());

    delete[] original_pixels_copy;

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/rotate-90",           test_rotate_90,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-180",          test_rotate_180,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-270",          test_rotate_270,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-inplace",      test_rotate_inplace,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-to-output",    test_rotate_to_output,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-with-palette", test_rotate_with_palette,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/rotate-multiple",     test_rotate_multiple_times, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char*)"/bindings/c++/rotate", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
