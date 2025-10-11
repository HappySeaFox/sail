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

#include "munit.h"

/*
 * This file contains binary compatibility tests to ensure that enum values
 * remain stable across versions. Changing enum values would break binary
 * compatibility with existing compiled code.
 *
 * For each enum, we test 3 representative values: middle, 3/4, and last.
 */

/*
 * SailStatus enum values.
 */
static MunitResult test_status_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Middle: SAIL_ERROR_EOF */
    munit_assert_int(SAIL_ERROR_EOF, ==, 16);

    /* 3/4: SAIL_ERROR_CODEC_NOT_FOUND */
    munit_assert_int(SAIL_ERROR_CODEC_NOT_FOUND, ==, 301);

    /* Last: SAIL_ERROR_CONFLICTING_OPERATION */
    munit_assert_int(SAIL_ERROR_CONFLICTING_OPERATION, ==, 403);

    return MUNIT_OK;
}

/*
 * SailPixelFormat enum values.
 */
static MunitResult test_pixel_format_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_PIXEL_FORMAT_BPP48_RGB, ==, 33);
    munit_assert_int(SAIL_PIXEL_FORMAT_BPP64_YUVA, ==, 76);
    munit_assert_int(SAIL_PIXEL_FORMAT_BPP32_BGRA_1010102, ==, 96);

    return MUNIT_OK;
}

/*
 * SailChromaSubsampling enum values.
 */
static MunitResult test_chroma_subsampling_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_CHROMA_SUBSAMPLING_411, ==, 4);
    munit_assert_int(SAIL_CHROMA_SUBSAMPLING_421, ==, 6);
    munit_assert_int(SAIL_CHROMA_SUBSAMPLING_444, ==, 8);

    return MUNIT_OK;
}

/*
 * SailOrientation enum values.
 */
static MunitResult test_orientation_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY, ==, 4);
    munit_assert_int(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_90, ==, 6);
    munit_assert_int(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_270, ==, 7);

    return MUNIT_OK;
}

/*
 * SailCompression enum values.
 */
static MunitResult test_compression_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_COMPRESSION_LERC, ==, 21);
    munit_assert_int(SAIL_COMPRESSION_JPEG_LS, ==, 58);
    munit_assert_int(SAIL_COMPRESSION_VVC, ==, 65);

    return MUNIT_OK;
}

/*
 * SailMetaData enum values.
 */
static MunitResult test_meta_data_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_META_DATA_LABEL, ==, 15);
    munit_assert_int(SAIL_META_DATA_SOURCE, ==, 22);
    munit_assert_int(SAIL_META_DATA_XMP, ==, 27);

    return MUNIT_OK;
}

/*
 * SailResolutionUnit enum values.
 */
static MunitResult test_resolution_unit_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_RESOLUTION_UNIT_CENTIMETER, ==, 2);
    munit_assert_int(SAIL_RESOLUTION_UNIT_METER, ==, 3);
    munit_assert_int(SAIL_RESOLUTION_UNIT_INCH, ==, 4);

    return MUNIT_OK;
}

/*
 * SailCodecFeature enum values (bit flags).
 */
static MunitResult test_codec_feature_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_CODEC_FEATURE_MULTI_PAGED, ==, 1 << 3);
    munit_assert_int(SAIL_CODEC_FEATURE_ICCP, ==, 1 << 6);
    munit_assert_int(SAIL_CODEC_FEATURE_SOURCE_IMAGE, ==, 1 << 7);

    return MUNIT_OK;
}

/*
 * SailOption enum values (bit flags).
 */
static MunitResult test_option_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_OPTION_INTERLACED, ==, 1 << 1);
    munit_assert_int(SAIL_OPTION_ICCP, ==, 1 << 2);
    munit_assert_int(SAIL_OPTION_SOURCE_IMAGE, ==, 1 << 3);

    return MUNIT_OK;
}

/*
 * SailVariantType enum values.
 */
static MunitResult test_variant_type_binary_compatibility(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert_int(SAIL_VARIANT_TYPE_LONG, ==, 7);
    munit_assert_int(SAIL_VARIANT_TYPE_STRING, ==, 11);
    munit_assert_int(SAIL_VARIANT_TYPE_INVALID, ==, 13);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/status",               test_status_binary_compatibility,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pixel-format",         test_pixel_format_binary_compatibility,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/chroma-subsampling",   test_chroma_subsampling_binary_compatibility,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/orientation",          test_orientation_binary_compatibility,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/compression",          test_compression_binary_compatibility,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/meta-data",            test_meta_data_binary_compatibility,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/resolution-unit",      test_resolution_unit_binary_compatibility,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-feature",        test_codec_feature_binary_compatibility,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/option",               test_option_binary_compatibility,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/variant-type",         test_variant_type_binary_compatibility,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/binary-compatibility", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
