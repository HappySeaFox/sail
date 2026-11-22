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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sail/sail.h>

#include "munit.h"

#include "dimension-limits.h"

/* Test loading 1x1 pixel image */
static MunitResult test_edge_1x1_image(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PNG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/1x1.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    if (status == SAIL_OK)
    {
        munit_assert(image->width == 1);
        munit_assert(image->height == 1);
        sail_destroy_image(image);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test resource cleanup on load error */
static MunitResult test_edge_cleanup_on_error(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_JPEG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/truncated.jpg";

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_name("JPEG", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_file(path, codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (image != NULL)
        {
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test corrupted compression stream */
static MunitResult test_edge_corrupted_compression(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PNG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-compression.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test corrupted palette */
static MunitResult test_edge_corrupted_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_GIF
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-palette.gif";

    struct sail_image* image = NULL;
    sail_load_from_file(path, &image);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test double destroy */
static MunitResult test_edge_double_destroy(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    sail_destroy_image(image);
    sail_destroy_image(NULL);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test early stop without loading any frames */
static MunitResult test_edge_early_stop_no_frames(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test loading from empty buffer */
static MunitResult test_edge_empty_memory_buffer(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    unsigned char buffer[1] = {0};

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_name("BMP", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_memory(buffer, 0, codec_info, &state);

    munit_assert(status != SAIL_OK);

    return MUNIT_OK;
}

/*
 * Test an animation whose frame duration doesn't fit into the frame delay.
 *
 * The image is bpp24-yuv-animated.avif with the media timescale set to 1 and the sample delta
 * set to 0xFFFFFFFF, which makes libavif report a duration of ~4.3e9 seconds.
 */
static MunitResult test_edge_huge_frame_duration(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_AVIF
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-frame-duration.avif";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);

    for (;;)
    {
        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_next_frame(state, &image);

        if (status != SAIL_OK)
        {
            munit_assert_null(image);
            break;
        }

        munit_assert_not_null(image);
        munit_assert(image->delay >= 0);

        sail_destroy_image(image);
    }

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test invalid codec info queries */
static MunitResult test_edge_invalid_codec_queries(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const struct sail_codec_info* codec_info = NULL;

    munit_assert(sail_codec_info_from_extension(NULL, &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_extension("", &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_extension("nonexistent_format_xyz", &codec_info) != SAIL_OK);

    munit_assert(sail_codec_info_from_path(NULL, &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_path("", &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_path("file.xyz", &codec_info) != SAIL_OK);

    munit_assert(sail_codec_info_from_name(NULL, &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_name("", &codec_info) != SAIL_OK);
    munit_assert(sail_codec_info_from_name("nonexistent_codec_xyz", &codec_info) != SAIL_OK);

    return MUNIT_OK;
}

/* Test invalid magic number */
static MunitResult test_edge_invalid_magic(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_JPEG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/invalid-magic.jpg";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test invalid palette size */
static MunitResult test_edge_invalid_palette_size(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_GIF
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/corrupted-palette.gif";

    struct sail_image* image = NULL;
    sail_load_from_file(path, &image);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test memory buffer boundary */
static MunitResult test_edge_memory_boundary(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_JPEG
    unsigned char small_buffer[10] = {0xFF, 0xD8, 0xFF, 0xE0, 0, 0, 0, 0, 0, 0};

    const struct sail_codec_info* codec_info = NULL;
    munit_assert(sail_codec_info_from_name("JPEG", &codec_info) == SAIL_OK);

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_memory(small_buffer, sizeof(small_buffer), codec_info, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (image != NULL)
        {
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
        munit_assert(status != SAIL_OK);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test nonexistent file */
static MunitResult test_edge_nonexistent_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = "this-file-does-not-exist-sail-test-123456789.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
}

/* Test NULL pointer handling */
static MunitResult test_edge_null_pointers(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    munit_assert(sail_load_from_file(NULL, NULL) != SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(NULL, &image) != SAIL_OK);
    munit_assert_null(image);

    munit_assert(sail_load_from_file("nonexistent.png", NULL) != SAIL_OK);

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(NULL, NULL, &state) != SAIL_OK);
    munit_assert_null(state);

    munit_assert(sail_load_next_frame(NULL, NULL) != SAIL_OK);
    munit_assert(sail_stop_loading(NULL) == SAIL_OK);

    munit_assert(sail_start_saving_into_file(NULL, NULL, NULL) != SAIL_OK);
    munit_assert(sail_write_next_frame(NULL, NULL) != SAIL_OK);
    munit_assert(sail_stop_saving(NULL) == SAIL_OK);

    return MUNIT_OK;
}

/*
 * Test a PSD that says its pixels are palette indexes but carries no palette.
 *
 * Such an image must not reach the caller: everything that draws indexed pixels dereferences
 * the palette.
 */
static MunitResult test_edge_psd_missing_palette(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PSD
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/indexed-without-palette.psd";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status == SAIL_ERROR_MISSING_PALETTE);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/*
 * Test a PSD whose RLE runs claim more bytes than a scan line holds.
 *
 * The image is 1-bit, so a scan line takes eight times less than the number of pixels in it.
 */
static MunitResult test_edge_psd_rle_overrun(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PSD
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/rle-overrun.psd";

    struct sail_image* image = NULL;

    munit_assert(sail_load_from_file(path, &image) == SAIL_OK);
    munit_assert_not_null(image);
    munit_assert(image->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED);

    sail_destroy_image(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test that RLE compressed PSD images decode exactly like their uncompressed counterparts */
static MunitResult test_edge_psd_rle_vs_uncompressed(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PSD
    static const char* const pairs[][2] = {
        {SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/psd/bpp1-indexed.psd",
         SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/psd/bpp1-indexed.rle.psd"},
        {SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/psd/bpp16-grayscale.psd",
         SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/psd/bpp16-grayscale.rle.psd"},
    };

    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++)
    {
        struct sail_image* uncompressed = NULL;
        struct sail_image* rle          = NULL;

        munit_assert(sail_load_from_file(pairs[i][0], &uncompressed) == SAIL_OK);
        munit_assert(sail_load_from_file(pairs[i][1], &rle) == SAIL_OK);

        munit_assert(uncompressed->width == rle->width);
        munit_assert(uncompressed->height == rle->height);
        munit_assert(uncompressed->pixel_format == rle->pixel_format);
        munit_assert(uncompressed->bytes_per_line == rle->bytes_per_line);
        munit_assert(memcmp(uncompressed->pixels, rle->pixels, (size_t)rle->height * rle->bytes_per_line) == 0);

        sail_destroy_image(uncompressed);
        sail_destroy_image(rle);
    }

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test options with NULL state */
static MunitResult test_edge_options_null_state(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    struct sail_load_options* load_options = NULL;
    munit_assert(sail_alloc_load_options(&load_options) == SAIL_OK);

    munit_assert(sail_start_loading_from_file_with_options(NULL, NULL, load_options, NULL) != SAIL_OK);

    sail_destroy_load_options(load_options);

    return MUNIT_OK;
}

/* Test partial frame load cleanup */
static MunitResult test_edge_partial_frame_cleanup(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_GIF
    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/gif/bpp8-indexed.comment.gif";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);
    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test dimension limits with a Fuji SuperCCD RAW image that LibRaw rotates by 45 degrees */
static MunitResult test_edge_raw_fuji_superccd_dimension_limits(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_RAW
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/fuji-superccd.raf";

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    /*
     * Before the processing, LibRaw reports the sizes of a 64x63 canvas rotated by 45 degrees.
     * The final image after rotating the canvas back is 43x45. Its 1935 pixels are fewer
     * than the 4032 pixels of the canvas.
     */
    struct sail_image* image = NULL;
    munit_assert(sail_probe_file(path, &image, NULL) == SAIL_OK);
    munit_assert_uint(image->width, ==, 43);
    munit_assert_uint(image->height, ==, 45);
    sail_destroy_image(image);

    /* Limits between the final image and the canvas sizes must not reject the image. */
    bool failed_in_start;
    munit_assert_int(dimension_limits_load(path, codec_info, 50, 50, 0, &failed_in_start), ==, SAIL_OK);
    munit_assert_int(dimension_limits_probe(path, codec_info, 50, 50, 0), ==, SAIL_OK);
    munit_assert_int(dimension_limits_load(path, codec_info, 0, 0, 2000, &failed_in_start), ==, SAIL_OK);
    munit_assert_int(dimension_limits_probe(path, codec_info, 0, 0, 2000), ==, SAIL_OK);
    munit_assert_int(dimension_limits_load(path, codec_info, 43, 45, 43 * 45, &failed_in_start), ==, SAIL_OK);

    /* The codec skips the early check for such images, so SAIL rejects them after the processing. */
    munit_assert_int(dimension_limits_load(path, codec_info, 42, 0, 0, &failed_in_start), ==,
                     SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert_false(failed_in_start);
    munit_assert_int(dimension_limits_load(path, codec_info, 0, 44, 0, &failed_in_start), ==,
                     SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert_false(failed_in_start);
    munit_assert_int(dimension_limits_load(path, codec_info, 0, 0, 43 * 45 - 1, &failed_in_start), ==,
                     SAIL_ERROR_IMAGE_TOO_LARGE);
    munit_assert_false(failed_in_start);
    munit_assert_int(dimension_limits_probe(path, codec_info, 42, 0, 0), ==, SAIL_ERROR_IMAGE_TOO_LARGE);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/*
 * Images with huge dimensions in their headers and almost no pixel data. Codecs must reject them
 * with SAIL_ERROR_IMAGE_TOO_LARGE when the dimension limits are set, before they allocate memory
 * for the pixels or decode them. Never load them without limits.
 *
 * Images named huge-width.* have just a huge width.
 */
static const char* const test_images_huge_dimensions[] = {
#ifdef SAIL_HAVE_BUILTIN_AVIF
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.avif",
#endif
#ifdef SAIL_HAVE_BUILTIN_FLI
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.fli",
#endif
#ifdef SAIL_HAVE_BUILTIN_GIF
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.gif",
#endif
#ifdef SAIL_HAVE_BUILTIN_JBIG
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-width.jbig",
#endif
#ifdef SAIL_HAVE_BUILTIN_JPEG
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.jpeg",
#endif
#ifdef SAIL_HAVE_BUILTIN_JPEG2000
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.jp2",
#endif
#ifdef SAIL_HAVE_BUILTIN_PNG
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.png",
#endif
#ifdef SAIL_HAVE_BUILTIN_QOI
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.qoi",
#endif
#ifdef SAIL_HAVE_BUILTIN_WEBP
    SAIL_TEST_IMAGES_EDGE_CASES_PATH "/huge-dimensions.webp",
#endif
    NULL,
};

/* Dimension limits to load images with huge dimensions with. Every set triggers the rejection on its own. */
struct huge_dimensions_limits
{
    unsigned max_width;
    unsigned max_height;
    uint64_t max_pixels;
};

static const struct huge_dimensions_limits huge_dimensions_limits[] = {
    {1000, 0, 0},
    {0, 1000, 0},
    {0, 0, 1000000},
};

/*
 * Returns the number of the limit sets from huge_dimensions_limits[] that apply to the image.
 * Images named huge-width.* have just a huge width, and their height is known only after decoding.
 */
static size_t huge_dimensions_limits_count(const char* path)
{
    return (strstr(path, "/huge-width.") != NULL) ? 1
                                                  : sizeof(huge_dimensions_limits) / sizeof(huge_dimensions_limits[0]);
}

/*
 * Test loading images with huge dimensions in their headers and almost no pixel data.
 *
 * Codecs must reject them before allocating memory for the pixels or decoding them. The final check
 * in SAIL doesn't help here:
 *   - Codecs that allocate memory in load_init() must fail in it.
 *   - Codecs that decode the whole image in seek_next_frame() would fail with another error
 *     while decoding the missing pixel data or allocating memory for it.
 */
static MunitResult test_edge_huge_dimensions_load(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    for (size_t i = 0; i < huge_dimensions_limits_count(path); i++)
    {
        const struct huge_dimensions_limits* limits = &huge_dimensions_limits[i];

        dimension_limits_assert_load_rejected(path, codec_info, limits->max_width, limits->max_height,
                                              limits->max_pixels);
    }

    return MUNIT_OK;
}

/* Test probing images with huge dimensions in their headers and almost no pixel data */
static MunitResult test_edge_huge_dimensions_probe(const MunitParameter params[], void* user_data)
{
    (void)user_data;

    const char* path = munit_parameters_get(params, "path");

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_path(path, &codec_info) == SAIL_OK);

    for (size_t i = 0; i < huge_dimensions_limits_count(path); i++)
    {
        const struct huge_dimensions_limits* limits = &huge_dimensions_limits[i];

        munit_assert_int(
            dimension_limits_probe(path, codec_info, limits->max_width, limits->max_height, limits->max_pixels), ==,
            SAIL_ERROR_IMAGE_TOO_LARGE);
    }

    return MUNIT_OK;
}

/* Test saving to read-only format */
static MunitResult test_edge_readonly_format_save(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_SVG
    const struct sail_codec_info* codec_info = NULL;
    sail_status_t status                     = sail_codec_info_from_name("SVG", &codec_info);

    munit_assert(status == SAIL_OK);
    munit_assert_not_null(codec_info);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test saving without calling start */
static MunitResult test_edge_save_without_start(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    sail_status_t status = sail_write_next_frame(NULL, image);
    munit_assert(status != SAIL_OK);

    sail_destroy_image(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test loading with stopped state */
static MunitResult test_edge_stopped_state_load(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    const char* path = SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp";

    void* state = NULL;
    munit_assert(sail_start_loading_from_file(path, NULL, &state) == SAIL_OK);

    struct sail_image* image = NULL;
    munit_assert(sail_load_next_frame(state, &image) == SAIL_OK);
    sail_destroy_image(image);

    munit_assert(sail_stop_loading(state) == SAIL_OK);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test truncated file */
static MunitResult test_edge_truncated_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_JPEG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/truncated.jpg";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test unsupported pixel format for codec */
static MunitResult test_edge_unsupported_pixel_format(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    struct sail_image* image = NULL;
    munit_assert(sail_load_from_file(SAIL_TEST_IMAGES_ACCEPTANCE_PATH "/bmp/bpp24-bgr.bmp", &image) == SAIL_OK);

    image->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;

    const struct sail_codec_info* codec_info;
    munit_assert(sail_codec_info_from_name("BMP", &codec_info) == SAIL_OK);

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/bmp/test-unsupported.bmp", SAIL_TEST_IMAGES_ACCEPTANCE_PATH);

    void* state          = NULL;
    sail_status_t status = sail_start_saving_into_file(temp_path, codec_info, &state);

    if (status == SAIL_OK)
    {
        status = sail_write_next_frame(state, image);
        sail_stop_saving(state);
        munit_assert(status != SAIL_OK);
    }

    remove(temp_path);
    sail_destroy_image(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test zero-byte file */
static MunitResult test_edge_zero_byte_file(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_PNG
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/zero-byte.png";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

/* Test zero dimensions */
static MunitResult test_edge_zero_dimensions(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

#ifdef SAIL_HAVE_BUILTIN_BMP
    const char* path = SAIL_TEST_IMAGES_EDGE_CASES_PATH "/zero-dimensions.bmp";

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(path, &image);

    munit_assert(status != SAIL_OK);
    munit_assert_null(image);

    return MUNIT_OK;
#else
    return MUNIT_SKIP;
#endif
}

// clang-format off
static MunitParameterEnum test_params_huge_dimensions[] = {
    { (char *)"path", (char **)test_images_huge_dimensions },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/1x1-image",                          test_edge_1x1_image,                          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/cleanup-on-error",                   test_edge_cleanup_on_error,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/corrupted-compression",              test_edge_corrupted_compression,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/corrupted-palette",                  test_edge_corrupted_palette,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/double-destroy",                     test_edge_double_destroy,                     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/early-stop-no-frames",               test_edge_early_stop_no_frames,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/empty-memory-buffer",                test_edge_empty_memory_buffer,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/huge-dimensions-load",               test_edge_huge_dimensions_load,               NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params_huge_dimensions },
    { (char *)"/huge-dimensions-probe",              test_edge_huge_dimensions_probe,              NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params_huge_dimensions },
    { (char *)"/huge-frame-duration",                test_edge_huge_frame_duration,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-codec-queries",              test_edge_invalid_codec_queries,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-magic",                      test_edge_invalid_magic,                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/invalid-palette-size",               test_edge_invalid_palette_size,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/memory-boundary",                    test_edge_memory_boundary,                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/nonexistent-file",                   test_edge_nonexistent_file,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/null-pointers",                      test_edge_null_pointers,                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/options-null-state",                 test_edge_options_null_state,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/partial-frame-cleanup",              test_edge_partial_frame_cleanup,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/psd-missing-palette",                test_edge_psd_missing_palette,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/psd-rle-overrun",                    test_edge_psd_rle_overrun,                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/psd-rle-vs-uncompressed",            test_edge_psd_rle_vs_uncompressed,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/raw-fuji-superccd-dimension-limits", test_edge_raw_fuji_superccd_dimension_limits, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/readonly-format-save",               test_edge_readonly_format_save,               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/save-without-start",                 test_edge_save_without_start,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/stopped-state-load",                 test_edge_stopped_state_load,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/truncated-file",                     test_edge_truncated_file,                     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/unsupported-pixel-format",           test_edge_unsupported_pixel_format,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/zero-byte-file",                     test_edge_zero_byte_file,                     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/zero-dimensions",                    test_edge_zero_dimensions,                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/edge-cases", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
