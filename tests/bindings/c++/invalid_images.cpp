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

#include <sail-c++/sail-c++.h>

#include "munit.h"

/* Test invalid image attribute access */
static MunitResult test_invalid_image_attributes(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img;

    munit_assert_false(invalid_img.is_valid());

    munit_assert(invalid_img.width() == 0);
    munit_assert(invalid_img.height() == 0);
    munit_assert(invalid_img.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN);
    munit_assert(invalid_img.bits_per_pixel() == 0);
    munit_assert(invalid_img.bytes_per_line() == 0);
    munit_assert(invalid_img.pixels() == nullptr);
    munit_assert_false(invalid_img.is_indexed());

    return MUNIT_OK;
}

/* Test invalid image conversion */
static MunitResult test_invalid_image_conversion(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img;
    munit_assert_false(invalid_img.is_valid());

    sail::image converted = invalid_img.convert_to(SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert_false(converted.is_valid());
    munit_assert(converted.width() == 0);
    munit_assert(converted.height() == 0);
    munit_assert(converted.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN);

    return MUNIT_OK;
}

/* Test invalid image scan line access */
static MunitResult test_invalid_image_scan_line(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img;
    munit_assert_false(invalid_img.is_valid());

    munit_assert(invalid_img.scan_line(0) == nullptr);
    munit_assert(invalid_img.scan_line(100) == nullptr); // Even for out-of-bounds

    return MUNIT_OK;
}

/* Test invalid image comparison */
static MunitResult test_invalid_image_comparison(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img1;
    sail::image invalid_img2;
    munit_assert_false(invalid_img1.is_valid());
    munit_assert_false(invalid_img2.is_valid());

    munit_assert(invalid_img1.width() == invalid_img2.width());
    munit_assert(invalid_img1.height() == invalid_img2.height());
    munit_assert(invalid_img1.pixel_format() == invalid_img2.pixel_format());

    sail::image valid_img(SAIL_PIXEL_FORMAT_BPP24_RGB, 1, 1);
    if (valid_img.is_valid())
    {
        munit_assert_false(invalid_img1.width() == valid_img.width());
        munit_assert_false(invalid_img1.height() == valid_img.height());
        munit_assert_false(invalid_img1.pixel_format() == valid_img.pixel_format());
    }

    return MUNIT_OK;
}

/* Test invalid image save */
static MunitResult test_invalid_image_save(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img;
    munit_assert_false(invalid_img.is_valid());

    std::string extension;
    std::vector<sail::codec_info> codecs = sail::codec_info::list();

    bool found_codec = false;
    for (const auto& codec_info : codecs)
    {
        if (codec_info.is_valid() && !codec_info.save_features().pixel_formats().empty())
        {
            if (!codec_info.extensions().empty())
            {
                extension   = codec_info.extensions().front();
                found_codec = true;
                break;
            }
        }
    }

    if (!found_codec)
    {
        return MUNIT_SKIP;
    }

    char* temp_path = nullptr;
    munit_assert(sail_temp_file_path("sail_test_invalid", &temp_path) == SAIL_OK);

    std::string output_path = std::string(temp_path) + "." + extension;
    sail_free(temp_path);

    sail::image_output output(output_path);
    sail_status_t result = output.next_frame(invalid_img);
    munit_assert(result != SAIL_OK);

    return MUNIT_OK;
}

/* Test invalid codec info */
static MunitResult test_invalid_codec_info(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::codec_info invalid_codec;
    munit_assert_false(invalid_codec.is_valid());

    munit_assert_string_equal(invalid_codec.name().c_str(), "");
    munit_assert_string_equal(invalid_codec.description().c_str(), "");
    munit_assert_string_equal(invalid_codec.version().c_str(), "");

    // Test features access - skip these tests as they may cause crashes
    // sail::save_features save_features = invalid_codec.save_features();
    // sail::load_features load_features = invalid_codec.load_features();

    return MUNIT_OK;
}

/* Test invalid image input */
static MunitResult test_invalid_image_input(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    std::string extension;
    std::vector<sail::codec_info> codecs = sail::codec_info::list();

    bool found_codec = false;
    for (const auto& codec_info : codecs)
    {
        if (codec_info.is_valid() && codec_info.load_features().features() != 0)
        {
            if (!codec_info.extensions().empty())
            {
                extension   = codec_info.extensions().front();
                found_codec = true;
                break;
            }
        }
    }

    if (!found_codec)
    {
        return MUNIT_SKIP;
    }

    bool exception_thrown = false;
    try
    {
        sail::image_input input("/non/existent/file." + extension);
    }
    catch (...)
    {
        exception_thrown = true;
    }

    munit_assert_true(exception_thrown);

    return MUNIT_OK;
}

/* Test invalid image input with empty path */
static MunitResult test_invalid_image_input_empty_path(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    bool exception_thrown = false;
    try
    {
        sail::image_input input("");
    }
    catch (...)
    {
        exception_thrown = true;
    }

    munit_assert_true(exception_thrown);

    return MUNIT_OK;
}

/* Test invalid image output */
static MunitResult test_invalid_image_output(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    std::string extension;
    std::vector<sail::codec_info> codecs = sail::codec_info::list();

    bool found_codec = false;
    for (const auto& codec_info : codecs)
    {
        if (codec_info.is_valid() && !codec_info.save_features().pixel_formats().empty())
        {
            if (!codec_info.extensions().empty())
            {
                extension   = codec_info.extensions().front();
                found_codec = true;
                break;
            }
        }
    }

    if (!found_codec)
    {
        return MUNIT_SKIP;
    }

    bool exception_thrown = false;
    try
    {
        sail::image_output output("/invalid/path/that/does/not/exist/test." + extension);
    }
    catch (...)
    {
        exception_thrown = true;
    }

    munit_assert_true(exception_thrown);

    return MUNIT_OK;
}

/* Test invalid image output with empty path */
static MunitResult test_invalid_image_output_empty_path(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    bool exception_thrown = false;
    try
    {
        sail::image_output output("");
    }
    catch (...)
    {
        exception_thrown = true;
    }

    munit_assert_true(exception_thrown);

    return MUNIT_OK;
}

/* Test codec info from invalid extension */
static MunitResult test_codec_info_invalid_extension(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::codec_info codec = sail::codec_info::from_extension("invalid");
    munit_assert_false(codec.is_valid());

    sail::codec_info codec2 = sail::codec_info::from_extension("");
    munit_assert_false(codec2.is_valid());

    return MUNIT_OK;
}

/* Test codec info from invalid path */
static MunitResult test_codec_info_invalid_path(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::codec_info codec = sail::codec_info::from_path("/non/existent/file.invalid");
    munit_assert_false(codec.is_valid());

    sail::codec_info codec2 = sail::codec_info::from_path("");
    munit_assert_false(codec2.is_valid());

    return MUNIT_OK;
}

/* Test mixed valid/invalid operations */
static MunitResult test_mixed_valid_invalid_operations(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail::image invalid_img;
    sail::image valid_img(SAIL_PIXEL_FORMAT_BPP24_RGB, 1, 1);

    munit_assert_false(invalid_img.is_valid());

    if (valid_img.is_valid())
    {
        sail::image converted = valid_img.convert_to(SAIL_PIXEL_FORMAT_BPP24_RGB);
        munit_assert_true(converted.is_valid());

        munit_assert_false(invalid_img.width() == valid_img.width());
        munit_assert_false(invalid_img.height() == valid_img.height());
        munit_assert_false(invalid_img.pixel_format() == valid_img.pixel_format());

        munit_assert(valid_img.scan_line(0) != nullptr);
        munit_assert(invalid_img.scan_line(0) == nullptr);
    }

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/invalid-image-attributes",        test_invalid_image_attributes,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-conversion",        test_invalid_image_conversion,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-scan-line",         test_invalid_image_scan_line,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-comparison",        test_invalid_image_comparison,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-save",              test_invalid_image_save,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-codec-info",              test_invalid_codec_info,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-input",             test_invalid_image_input,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-input-empty-path",  test_invalid_image_input_empty_path,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-output",            test_invalid_image_output,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-image-output-empty-path", test_invalid_image_output_empty_path, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-info-invalid-extension",    test_codec_info_invalid_extension,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-info-invalid-path",         test_codec_info_invalid_path,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/mixed-valid-invalid-operations",  test_mixed_valid_invalid_operations,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/invalid-images", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
