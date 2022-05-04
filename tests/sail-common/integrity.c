/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include "sail-common.h"

#include "munit.h"

/*
 * Error macros.
 */
static sail_status_t check_value_is_2(int value) {
    if (value == 2) {
        return SAIL_OK;
    } else {
        return SAIL_ERROR_INVALID_ARGUMENT;
    }
}

static MunitResult test_error_macros(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    /* Check if SAIL_TRY_OR_EXECUTE() is able to continue loops. */
    {
        int result = 0;

        for (int i = 0; i < 4; i++) {
            SAIL_TRY_OR_EXECUTE(check_value_is_2(i),
                                /* on error */ continue);

            result++;
        }

        munit_assert_int(result, ==, 1);
    }

    /* Check SAIL_TRY_OR_SUPPRESS(). */
    {
        SAIL_TRY_OR_SUPPRESS(check_value_is_2(5));

    }

    return MUNIT_OK;
}

/*
 * Pixel formats.
 */
static MunitResult test_pixel_format_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_UNKNOWN), "UNKNOWN");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP1),   "BPP1");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP2),   "BPP2");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP4),   "BPP4");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP8),   "BPP8");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16),  "BPP16");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24),  "BPP24");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32),  "BPP32");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP48),  "BPP48");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64),  "BPP64");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP72),  "BPP72");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP96),  "BPP96");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP128), "BPP128");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP1_INDEXED),  "BPP1-INDEXED");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP2_INDEXED),  "BPP2-INDEXED");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP4_INDEXED),  "BPP4-INDEXED");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP8_INDEXED),  "BPP8-INDEXED");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_INDEXED), "BPP16-INDEXED");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE),  "BPP1-GRAYSCALE");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE),  "BPP2-GRAYSCALE");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE),  "BPP4-GRAYSCALE");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE),  "BPP8-GRAYSCALE");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE), "BPP16-GRAYSCALE");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA),  "BPP4-GRAYSCALE-ALPHA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA),  "BPP8-GRAYSCALE-ALPHA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA), "BPP16-GRAYSCALE-ALPHA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA), "BPP32-GRAYSCALE-ALPHA");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_RGB555), "BPP16-RGB555");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_BGR555), "BPP16-BGR555");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_RGB565), "BPP16-RGB565");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP16_BGR565), "BPP16-BGR565");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_RGB), "BPP24-RGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_BGR), "BPP24-BGR");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP48_RGB), "BPP48-RGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP48_BGR), "BPP48-BGR");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_RGBX), "BPP32-RGBX");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_BGRX), "BPP32-BGRX");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_XRGB), "BPP32-XRGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_XBGR), "BPP32-XBGR");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_RGBA), "BPP32-RGBA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_BGRA), "BPP32-BGRA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_ARGB), "BPP32-ARGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_ABGR), "BPP32-ABGR");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_RGBX), "BPP64-RGBX");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_BGRX), "BPP64-BGRX");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_XRGB), "BPP64-XRGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_XBGR), "BPP64-XBGR");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_RGBA), "BPP64-RGBA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_BGRA), "BPP64-BGRA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_ARGB), "BPP64-ARGB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_ABGR), "BPP64-ABGR");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_CMYK), "BPP32-CMYK");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_CMYK), "BPP64-CMYK");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_YCBCR), "BPP24-YCBCR");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_YCCK), "BPP32-YCCK");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_CIE_LAB), "BPP24-CIE-LAB");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP40_CIE_LAB), "BPP40-CIE-LAB");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_CIE_LUV), "BPP24-CIE-LUV");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP40_CIE_LUV), "BPP40-CIE-LUV");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP24_YUV), "BPP24-YUV");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP30_YUV), "BPP30-YUV");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP36_YUV), "BPP36-YUV");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP48_YUV), "BPP48-YUV");

    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP32_YUVA), "BPP32-YUVA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP40_YUVA), "BPP40-YUVA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP48_YUVA), "BPP48-YUVA");
    munit_assert_string_equal(sail_pixel_format_to_string(SAIL_PIXEL_FORMAT_BPP64_YUVA), "BPP64-YUVA");

    return MUNIT_OK;
}

static MunitResult test_pixel_format_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_pixel_format_from_string(NULL)   == SAIL_PIXEL_FORMAT_UNKNOWN);
    munit_assert(sail_pixel_format_from_string("Some") == SAIL_PIXEL_FORMAT_UNKNOWN);

    munit_assert(sail_pixel_format_from_string("UNKNOWN") == SAIL_PIXEL_FORMAT_UNKNOWN);

    munit_assert(sail_pixel_format_from_string("BPP1") ==   SAIL_PIXEL_FORMAT_BPP1);
    munit_assert(sail_pixel_format_from_string("BPP2") ==   SAIL_PIXEL_FORMAT_BPP2);
    munit_assert(sail_pixel_format_from_string("BPP4") ==   SAIL_PIXEL_FORMAT_BPP4);
    munit_assert(sail_pixel_format_from_string("BPP8") ==   SAIL_PIXEL_FORMAT_BPP8);
    munit_assert(sail_pixel_format_from_string("BPP16") ==  SAIL_PIXEL_FORMAT_BPP16);
    munit_assert(sail_pixel_format_from_string("BPP24") ==  SAIL_PIXEL_FORMAT_BPP24);
    munit_assert(sail_pixel_format_from_string("BPP32") ==  SAIL_PIXEL_FORMAT_BPP32);
    munit_assert(sail_pixel_format_from_string("BPP48") ==  SAIL_PIXEL_FORMAT_BPP48);
    munit_assert(sail_pixel_format_from_string("BPP64") ==  SAIL_PIXEL_FORMAT_BPP64);
    munit_assert(sail_pixel_format_from_string("BPP72") ==  SAIL_PIXEL_FORMAT_BPP72);
    munit_assert(sail_pixel_format_from_string("BPP96") ==  SAIL_PIXEL_FORMAT_BPP96);
    munit_assert(sail_pixel_format_from_string("BPP128") == SAIL_PIXEL_FORMAT_BPP128);

    munit_assert(sail_pixel_format_from_string("BPP1-INDEXED") ==  SAIL_PIXEL_FORMAT_BPP1_INDEXED);
    munit_assert(sail_pixel_format_from_string("BPP2-INDEXED") ==  SAIL_PIXEL_FORMAT_BPP2_INDEXED);
    munit_assert(sail_pixel_format_from_string("BPP4-INDEXED") ==  SAIL_PIXEL_FORMAT_BPP4_INDEXED);
    munit_assert(sail_pixel_format_from_string("BPP8-INDEXED") ==  SAIL_PIXEL_FORMAT_BPP8_INDEXED);
    munit_assert(sail_pixel_format_from_string("BPP16-INDEXED") == SAIL_PIXEL_FORMAT_BPP16_INDEXED);

    munit_assert(sail_pixel_format_from_string("BPP1-GRAYSCALE") ==  SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE);
    munit_assert(sail_pixel_format_from_string("BPP2-GRAYSCALE") ==  SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE);
    munit_assert(sail_pixel_format_from_string("BPP4-GRAYSCALE") ==  SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE);
    munit_assert(sail_pixel_format_from_string("BPP8-GRAYSCALE") ==  SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE);
    munit_assert(sail_pixel_format_from_string("BPP16-GRAYSCALE") == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE);

    munit_assert(sail_pixel_format_from_string("BPP4-GRAYSCALE-ALPHA") ==  SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA);
    munit_assert(sail_pixel_format_from_string("BPP8-GRAYSCALE-ALPHA") ==  SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA);
    munit_assert(sail_pixel_format_from_string("BPP16-GRAYSCALE-ALPHA") == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA);
    munit_assert(sail_pixel_format_from_string("BPP32-GRAYSCALE-ALPHA") == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA);

    munit_assert(sail_pixel_format_from_string("BPP16-RGB555") == SAIL_PIXEL_FORMAT_BPP16_RGB555);
    munit_assert(sail_pixel_format_from_string("BPP16-BGR555") == SAIL_PIXEL_FORMAT_BPP16_BGR555);
    munit_assert(sail_pixel_format_from_string("BPP16-RGB565") == SAIL_PIXEL_FORMAT_BPP16_RGB565);
    munit_assert(sail_pixel_format_from_string("BPP16-BGR565") == SAIL_PIXEL_FORMAT_BPP16_BGR565);

    munit_assert(sail_pixel_format_from_string("BPP24-RGB") == SAIL_PIXEL_FORMAT_BPP24_RGB);
    munit_assert(sail_pixel_format_from_string("BPP24-BGR") == SAIL_PIXEL_FORMAT_BPP24_BGR);

    munit_assert(sail_pixel_format_from_string("BPP48-RGB") == SAIL_PIXEL_FORMAT_BPP48_RGB);
    munit_assert(sail_pixel_format_from_string("BPP48-BGR") == SAIL_PIXEL_FORMAT_BPP48_BGR);

    munit_assert(sail_pixel_format_from_string("BPP32-RGBX") == SAIL_PIXEL_FORMAT_BPP32_RGBX);
    munit_assert(sail_pixel_format_from_string("BPP32-BGRX") == SAIL_PIXEL_FORMAT_BPP32_BGRX);
    munit_assert(sail_pixel_format_from_string("BPP32-XRGB") == SAIL_PIXEL_FORMAT_BPP32_XRGB);
    munit_assert(sail_pixel_format_from_string("BPP32-XBGR") == SAIL_PIXEL_FORMAT_BPP32_XBGR);
    munit_assert(sail_pixel_format_from_string("BPP32-RGBA") == SAIL_PIXEL_FORMAT_BPP32_RGBA);
    munit_assert(sail_pixel_format_from_string("BPP32-BGRA") == SAIL_PIXEL_FORMAT_BPP32_BGRA);
    munit_assert(sail_pixel_format_from_string("BPP32-ARGB") == SAIL_PIXEL_FORMAT_BPP32_ARGB);
    munit_assert(sail_pixel_format_from_string("BPP32-ABGR") == SAIL_PIXEL_FORMAT_BPP32_ABGR);

    munit_assert(sail_pixel_format_from_string("BPP64-RGBX") == SAIL_PIXEL_FORMAT_BPP64_RGBX);
    munit_assert(sail_pixel_format_from_string("BPP64-BGRX") == SAIL_PIXEL_FORMAT_BPP64_BGRX);
    munit_assert(sail_pixel_format_from_string("BPP64-XRGB") == SAIL_PIXEL_FORMAT_BPP64_XRGB);
    munit_assert(sail_pixel_format_from_string("BPP64-XBGR") == SAIL_PIXEL_FORMAT_BPP64_XBGR);
    munit_assert(sail_pixel_format_from_string("BPP64-RGBA") == SAIL_PIXEL_FORMAT_BPP64_RGBA);
    munit_assert(sail_pixel_format_from_string("BPP64-BGRA") == SAIL_PIXEL_FORMAT_BPP64_BGRA);
    munit_assert(sail_pixel_format_from_string("BPP64-ARGB") == SAIL_PIXEL_FORMAT_BPP64_ARGB);
    munit_assert(sail_pixel_format_from_string("BPP64-ABGR") == SAIL_PIXEL_FORMAT_BPP64_ABGR);

    munit_assert(sail_pixel_format_from_string("BPP32-CMYK") == SAIL_PIXEL_FORMAT_BPP32_CMYK);
    munit_assert(sail_pixel_format_from_string("BPP64-CMYK") == SAIL_PIXEL_FORMAT_BPP64_CMYK);

    munit_assert(sail_pixel_format_from_string("BPP24-YCBCR") == SAIL_PIXEL_FORMAT_BPP24_YCBCR);

    munit_assert(sail_pixel_format_from_string("BPP32-YCCK") == SAIL_PIXEL_FORMAT_BPP32_YCCK);

    munit_assert(sail_pixel_format_from_string("BPP24-CIE-LAB") == SAIL_PIXEL_FORMAT_BPP24_CIE_LAB);
    munit_assert(sail_pixel_format_from_string("BPP40-CIE-LAB") == SAIL_PIXEL_FORMAT_BPP40_CIE_LAB);

    munit_assert(sail_pixel_format_from_string("BPP24-CIE-LUV") == SAIL_PIXEL_FORMAT_BPP24_CIE_LUV);
    munit_assert(sail_pixel_format_from_string("BPP40-CIE-LUV") == SAIL_PIXEL_FORMAT_BPP40_CIE_LUV);

    munit_assert(sail_pixel_format_from_string("BPP24-YUV") == SAIL_PIXEL_FORMAT_BPP24_YUV);
    munit_assert(sail_pixel_format_from_string("BPP30-YUV") == SAIL_PIXEL_FORMAT_BPP30_YUV);
    munit_assert(sail_pixel_format_from_string("BPP36-YUV") == SAIL_PIXEL_FORMAT_BPP36_YUV);
    munit_assert(sail_pixel_format_from_string("BPP48-YUV") == SAIL_PIXEL_FORMAT_BPP48_YUV);

    munit_assert(sail_pixel_format_from_string("BPP32-YUVA") == SAIL_PIXEL_FORMAT_BPP32_YUVA);
    munit_assert(sail_pixel_format_from_string("BPP40-YUVA") == SAIL_PIXEL_FORMAT_BPP40_YUVA);
    munit_assert(sail_pixel_format_from_string("BPP48-YUVA") == SAIL_PIXEL_FORMAT_BPP48_YUVA);
    munit_assert(sail_pixel_format_from_string("BPP64-YUVA") == SAIL_PIXEL_FORMAT_BPP64_YUVA);

    return MUNIT_OK;
}

/*
 * Chroma subsampling.
 */
static MunitResult test_chroma_subsampling_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_UNKNOWN), "UNKNOWN");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_311),     "311");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_400),     "400");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_410),     "410");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_411),     "411");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_420),     "420");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_421),     "421");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_422),     "422");
    munit_assert_string_equal(sail_chroma_subsampling_to_string(SAIL_CHROMA_SUBSAMPLING_444),     "444");

    return MUNIT_OK;
}

static MunitResult test_chroma_subsampling_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_chroma_subsampling_from_string(NULL)      == SAIL_CHROMA_SUBSAMPLING_UNKNOWN);
    munit_assert(sail_chroma_subsampling_from_string("Some")    == SAIL_CHROMA_SUBSAMPLING_UNKNOWN);

    munit_assert(sail_chroma_subsampling_from_string("UNKNOWN") == SAIL_CHROMA_SUBSAMPLING_UNKNOWN);
    munit_assert(sail_chroma_subsampling_from_string("311")     == SAIL_CHROMA_SUBSAMPLING_311);
    munit_assert(sail_chroma_subsampling_from_string("400")     == SAIL_CHROMA_SUBSAMPLING_400);
    munit_assert(sail_chroma_subsampling_from_string("410")     == SAIL_CHROMA_SUBSAMPLING_410);
    munit_assert(sail_chroma_subsampling_from_string("411")     == SAIL_CHROMA_SUBSAMPLING_411);
    munit_assert(sail_chroma_subsampling_from_string("420")     == SAIL_CHROMA_SUBSAMPLING_420);
    munit_assert(sail_chroma_subsampling_from_string("422")     == SAIL_CHROMA_SUBSAMPLING_422);
    munit_assert(sail_chroma_subsampling_from_string("444")     == SAIL_CHROMA_SUBSAMPLING_444);

    return MUNIT_OK;
}

/*
 * Orientation.
 */
static MunitResult test_orientation_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_NORMAL),                            "NORMAL");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_ROTATED_90),                        "ROTATED-90");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_ROTATED_180),                       "ROTATED-180");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_ROTATED_270),                       "ROTATED-270");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY),             "MIRRORED-HORIZONTALLY");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_MIRRORED_VERTICALLY),               "MIRRORED-VERTICALLY");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_90),  "MIRRORED-HORIZONTALLY-ROTATED-90");
    munit_assert_string_equal(sail_orientation_to_string(SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_270), "MIRRORED-HORIZONTALLY-ROTATED-270");

    return MUNIT_OK;
}

static MunitResult test_orientation_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_orientation_from_string(NULL)                 == SAIL_ORIENTATION_NORMAL);
    munit_assert(sail_orientation_from_string("Some")               == SAIL_ORIENTATION_NORMAL);

    munit_assert(sail_orientation_from_string("NORMAL")                            == SAIL_ORIENTATION_NORMAL);
    munit_assert(sail_orientation_from_string("ROTATED-90")                        == SAIL_ORIENTATION_ROTATED_90);
    munit_assert(sail_orientation_from_string("ROTATED-180")                       == SAIL_ORIENTATION_ROTATED_180);
    munit_assert(sail_orientation_from_string("ROTATED-270")                       == SAIL_ORIENTATION_ROTATED_270);
    munit_assert(sail_orientation_from_string("MIRRORED-HORIZONTALLY")             == SAIL_ORIENTATION_MIRRORED_HORIZONTALLY);
    munit_assert(sail_orientation_from_string("MIRRORED-VERTICALLY")               == SAIL_ORIENTATION_MIRRORED_VERTICALLY);
    munit_assert(sail_orientation_from_string("MIRRORED-HORIZONTALLY-ROTATED-90")  == SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_90);
    munit_assert(sail_orientation_from_string("MIRRORED-HORIZONTALLY-ROTATED-270") == SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_270);

    return MUNIT_OK;
}

/*
 * Compression types.
 */
static MunitResult test_compression_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_UNKNOWN),       "UNKNOWN");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_NONE),          "NONE");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_ADOBE_DEFLATE), "ADOBE-DEFLATE");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_AV1),           "AV1");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_FAX3),    "CCITT-FAX3");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_FAX4),    "CCITT-FAX4");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_RLE),     "CCITT-RLE");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_RLEW),    "CCITT-RLEW");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_T4),      "CCITT-T4");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_CCITT_T6),      "CCITT-T6");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_DCS),           "DCS");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_DEFLATE),       "DEFLATE");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_IT8_BL),        "IT8-BL");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_IT8_CTPAD),     "IT8-CTPAD");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_IT8_LW),        "IT8-LW");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_IT8_MP),        "IT8-MP");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_JBIG),          "JBIG");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_JPEG),          "JPEG");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_JPEG_2000),     "JPEG-2000");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_JPEG_XL),       "JPEG-XL");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_JPEG_XR),       "JPEG-XR");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_LERC),          "LERC");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_LZMA),          "LZMA");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_LZW),           "LZW");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_NEXT),          "NEXT");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_OJPEG),         "OJPEG");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_PACKBITS),      "PACKBITS");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_PIXAR_FILM),    "PIXAR-FILM");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_PIXAR_LOG),     "PIXAR-LOG");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_RLE),           "RLE");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_SGI_LOG),       "SGI-LOG");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_SGI_LOG24),     "SGI-LOG24");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_T43),           "T43");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_T85),           "T85");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_THUNDERSCAN),   "THUNDERSCAN");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_WEBP),          "WEBP");
    munit_assert_string_equal(sail_compression_to_string(SAIL_COMPRESSION_ZSTD),          "ZSTD");

    return MUNIT_OK;
}

static MunitResult test_compression_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_compression_from_string(NULL)   == SAIL_COMPRESSION_UNKNOWN);
    munit_assert(sail_compression_from_string("Some") == SAIL_COMPRESSION_UNKNOWN);

    munit_assert(sail_compression_from_string("UNKNOWN")       == SAIL_COMPRESSION_UNKNOWN);
    munit_assert(sail_compression_from_string("NONE")          == SAIL_COMPRESSION_NONE);
    munit_assert(sail_compression_from_string("ADOBE-DEFLATE") == SAIL_COMPRESSION_ADOBE_DEFLATE);
    munit_assert(sail_compression_from_string("AV1")           == SAIL_COMPRESSION_AV1);
    munit_assert(sail_compression_from_string("CCITT-FAX3")    == SAIL_COMPRESSION_CCITT_FAX3);
    munit_assert(sail_compression_from_string("CCITT-FAX4")    == SAIL_COMPRESSION_CCITT_FAX4);
    munit_assert(sail_compression_from_string("CCITT-RLE")     == SAIL_COMPRESSION_CCITT_RLE);
    munit_assert(sail_compression_from_string("CCITT-RLEW")    == SAIL_COMPRESSION_CCITT_RLEW);
    munit_assert(sail_compression_from_string("CCITT-T4")      == SAIL_COMPRESSION_CCITT_T4);
    munit_assert(sail_compression_from_string("CCITT-T6")      == SAIL_COMPRESSION_CCITT_T6);
    munit_assert(sail_compression_from_string("DCS")           == SAIL_COMPRESSION_DCS);
    munit_assert(sail_compression_from_string("DEFLATE")       == SAIL_COMPRESSION_DEFLATE);
    munit_assert(sail_compression_from_string("IT8-BL")        == SAIL_COMPRESSION_IT8_BL);
    munit_assert(sail_compression_from_string("IT8-CTPAD")     == SAIL_COMPRESSION_IT8_CTPAD);
    munit_assert(sail_compression_from_string("IT8-LW")        == SAIL_COMPRESSION_IT8_LW);
    munit_assert(sail_compression_from_string("IT8-MP")        == SAIL_COMPRESSION_IT8_MP);
    munit_assert(sail_compression_from_string("JBIG")          == SAIL_COMPRESSION_JBIG);
    munit_assert(sail_compression_from_string("JPEG")          == SAIL_COMPRESSION_JPEG);
    munit_assert(sail_compression_from_string("JPEG-2000")     == SAIL_COMPRESSION_JPEG_2000);
    munit_assert(sail_compression_from_string("JPEG-XL")       == SAIL_COMPRESSION_JPEG_XL);
    munit_assert(sail_compression_from_string("JPEG-XR")       == SAIL_COMPRESSION_JPEG_XR);
    munit_assert(sail_compression_from_string("LERC")          == SAIL_COMPRESSION_LERC);
    munit_assert(sail_compression_from_string("LZMA")          == SAIL_COMPRESSION_LZMA);
    munit_assert(sail_compression_from_string("LZW")           == SAIL_COMPRESSION_LZW);
    munit_assert(sail_compression_from_string("NEXT")          == SAIL_COMPRESSION_NEXT);
    munit_assert(sail_compression_from_string("OJPEG")         == SAIL_COMPRESSION_OJPEG);
    munit_assert(sail_compression_from_string("PACKBITS")      == SAIL_COMPRESSION_PACKBITS);
    munit_assert(sail_compression_from_string("PIXAR-FILM")    == SAIL_COMPRESSION_PIXAR_FILM);
    munit_assert(sail_compression_from_string("PIXAR-LOG")     == SAIL_COMPRESSION_PIXAR_LOG);
    munit_assert(sail_compression_from_string("RLE")           == SAIL_COMPRESSION_RLE);
    munit_assert(sail_compression_from_string("SGI-LOG")       == SAIL_COMPRESSION_SGI_LOG);
    munit_assert(sail_compression_from_string("SGI-LOG24")     == SAIL_COMPRESSION_SGI_LOG24);
    munit_assert(sail_compression_from_string("T43")           == SAIL_COMPRESSION_T43);
    munit_assert(sail_compression_from_string("T85")           == SAIL_COMPRESSION_T85);
    munit_assert(sail_compression_from_string("THUNDERSCAN")   == SAIL_COMPRESSION_THUNDERSCAN);
    munit_assert(sail_compression_from_string("WEBP")          == SAIL_COMPRESSION_WEBP);
    munit_assert(sail_compression_from_string("ZSTD")          == SAIL_COMPRESSION_ZSTD);

    return MUNIT_OK;
}

/*
 * Meta data keys.
 */
static MunitResult test_meta_data_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_UNKNOWN),       "Unknown");

    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_ARTIST),           "Artist");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_AUTHOR),           "Author");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_COMMENT),          "Comment");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_COMPUTER),         "Computer");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_COPYRIGHT),        "Copyright");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_CREATION_TIME),    "Creation Time");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_DESCRIPTION),      "Description");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_DISCLAIMER),       "Disclaimer");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_DOCUMENT),         "Document");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_EXIF),             "EXIF");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_ID),               "ID");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_IPTC),             "IPTC");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_JOB),              "Job");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_LABEL),            "Label");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_MAKE),             "Make");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_MODEL),            "Model");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_NAME),             "Name");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_PRINTER),          "Printer");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_SOFTWARE),         "Software");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_SOFTWARE_VERSION), "Software Version");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_SOURCE),           "Source");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_TIME_CONSUMED),    "Time Consumed");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_TITLE),            "Title");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_URL),              "URL");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_WARNING),          "Warning");
    munit_assert_string_equal(sail_meta_data_to_string(SAIL_META_DATA_XMP),              "XMP");

    return MUNIT_OK;
}

static MunitResult test_meta_data_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_meta_data_from_string(NULL)   == SAIL_META_DATA_UNKNOWN);
    munit_assert(sail_meta_data_from_string("Some") == SAIL_META_DATA_UNKNOWN);

    munit_assert(sail_meta_data_from_string("Unknown") == SAIL_META_DATA_UNKNOWN);

    munit_assert(sail_meta_data_from_string("Artist")           == SAIL_META_DATA_ARTIST);
    munit_assert(sail_meta_data_from_string("Author")           == SAIL_META_DATA_AUTHOR);
    munit_assert(sail_meta_data_from_string("Comment")          == SAIL_META_DATA_COMMENT);
    munit_assert(sail_meta_data_from_string("Computer")         == SAIL_META_DATA_COMPUTER);
    munit_assert(sail_meta_data_from_string("Copyright")        == SAIL_META_DATA_COPYRIGHT);
    munit_assert(sail_meta_data_from_string("Creation Time")    == SAIL_META_DATA_CREATION_TIME);
    munit_assert(sail_meta_data_from_string("Description")      == SAIL_META_DATA_DESCRIPTION);
    munit_assert(sail_meta_data_from_string("Disclaimer")       == SAIL_META_DATA_DISCLAIMER);
    munit_assert(sail_meta_data_from_string("Document")         == SAIL_META_DATA_DOCUMENT);
    munit_assert(sail_meta_data_from_string("EXIF")             == SAIL_META_DATA_EXIF);
    munit_assert(sail_meta_data_from_string("ID")               == SAIL_META_DATA_ID);
    munit_assert(sail_meta_data_from_string("IPTC")             == SAIL_META_DATA_IPTC);
    munit_assert(sail_meta_data_from_string("Job")              == SAIL_META_DATA_JOB);
    munit_assert(sail_meta_data_from_string("Label")            == SAIL_META_DATA_LABEL);
    munit_assert(sail_meta_data_from_string("Make")             == SAIL_META_DATA_MAKE);
    munit_assert(sail_meta_data_from_string("Model")            == SAIL_META_DATA_MODEL);
    munit_assert(sail_meta_data_from_string("Name")             == SAIL_META_DATA_NAME);
    munit_assert(sail_meta_data_from_string("Printer")          == SAIL_META_DATA_PRINTER);
    munit_assert(sail_meta_data_from_string("Software")         == SAIL_META_DATA_SOFTWARE);
    munit_assert(sail_meta_data_from_string("Software Version") == SAIL_META_DATA_SOFTWARE_VERSION);
    munit_assert(sail_meta_data_from_string("Source")           == SAIL_META_DATA_SOURCE);
    munit_assert(sail_meta_data_from_string("Time Consumed")    == SAIL_META_DATA_TIME_CONSUMED);
    munit_assert(sail_meta_data_from_string("Title")            == SAIL_META_DATA_TITLE);
    munit_assert(sail_meta_data_from_string("URL")              == SAIL_META_DATA_URL);
    munit_assert(sail_meta_data_from_string("Warning")          == SAIL_META_DATA_WARNING);
    munit_assert(sail_meta_data_from_string("XMP")              == SAIL_META_DATA_XMP);

    return MUNIT_OK;
}

/*
 * Resolution Unit.
 */
static MunitResult test_resolution_unit_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_resolution_unit_to_string(SAIL_RESOLUTION_UNIT_UNKNOWN),    "Unknown");
    munit_assert_string_equal(sail_resolution_unit_to_string(SAIL_RESOLUTION_UNIT_MICROMETER), "Micrometer");
    munit_assert_string_equal(sail_resolution_unit_to_string(SAIL_RESOLUTION_UNIT_CENTIMETER), "Centimeter");
    munit_assert_string_equal(sail_resolution_unit_to_string(SAIL_RESOLUTION_UNIT_METER),      "Meter");
    munit_assert_string_equal(sail_resolution_unit_to_string(SAIL_RESOLUTION_UNIT_INCH),       "Inch");

    return MUNIT_OK;
}

static MunitResult test_resolution_unit_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_resolution_unit_from_string(NULL)   == SAIL_RESOLUTION_UNIT_UNKNOWN);
    munit_assert(sail_resolution_unit_from_string("Some") == SAIL_RESOLUTION_UNIT_UNKNOWN);

    munit_assert(sail_resolution_unit_from_string("Unknown")    == SAIL_RESOLUTION_UNIT_UNKNOWN);
    munit_assert(sail_resolution_unit_from_string("Micrometer") == SAIL_RESOLUTION_UNIT_MICROMETER);
    munit_assert(sail_resolution_unit_from_string("Centimeter") == SAIL_RESOLUTION_UNIT_CENTIMETER);
    munit_assert(sail_resolution_unit_from_string("Meter")      == SAIL_RESOLUTION_UNIT_METER);
    munit_assert(sail_resolution_unit_from_string("Inch")       == SAIL_RESOLUTION_UNIT_INCH);

    return MUNIT_OK;
}

/*
 * Codec features.
 */
static MunitResult test_codec_feature_to_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_UNKNOWN),     "UNKNOWN");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_STATIC),      "STATIC");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_ANIMATED),    "ANIMATED");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_MULTI_PAGED), "MULTI-PAGED");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_META_DATA),   "META-DATA");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_INTERLACED),  "INTERLACED");
    munit_assert_string_equal(sail_codec_feature_to_string(SAIL_CODEC_FEATURE_ICCP),        "ICCP");

    return MUNIT_OK;
}

static MunitResult test_codec_feature_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    munit_assert(sail_codec_feature_from_string(NULL)   == SAIL_CODEC_FEATURE_UNKNOWN);
    munit_assert(sail_codec_feature_from_string("Some") == SAIL_CODEC_FEATURE_UNKNOWN);

    munit_assert(sail_codec_feature_from_string("UNKNOWN")     == SAIL_CODEC_FEATURE_UNKNOWN);
    munit_assert(sail_codec_feature_from_string("STATIC")      == SAIL_CODEC_FEATURE_STATIC);
    munit_assert(sail_codec_feature_from_string("ANIMATED")    == SAIL_CODEC_FEATURE_ANIMATED);
    munit_assert(sail_codec_feature_from_string("MULTI-PAGED") == SAIL_CODEC_FEATURE_MULTI_PAGED);
    munit_assert(sail_codec_feature_from_string("META-DATA")   == SAIL_CODEC_FEATURE_META_DATA);
    munit_assert(sail_codec_feature_from_string("INTERLACED")  == SAIL_CODEC_FEATURE_INTERLACED);
    munit_assert(sail_codec_feature_from_string("ICCP")        == SAIL_CODEC_FEATURE_ICCP);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/error-macros", test_error_macros, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/pixel-format-to-string",   test_pixel_format_to_string,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pixel-format-from-string", test_pixel_format_from_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/chroma-subsampling-to-string",   test_chroma_subsampling_to_string,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/chroma-subsampling-from-string", test_chroma_subsampling_from_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/orientation-to-string",   test_orientation_to_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/orientation-from-string", test_orientation_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/compression-to-string",   test_compression_to_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/compression-from-string", test_compression_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/meta-data-to-string",   test_meta_data_to_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/meta-data-from-string", test_meta_data_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/resolution-unit-to-string",   test_resolution_unit_to_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/resolution-unit-from-string", test_resolution_unit_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { (char *)"/codec-feature-to-string",   test_codec_feature_to_string,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-feature-from-string", test_codec_feature_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/integrity",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
