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

#include <string.h>

#include <sail-manip/sail-manip.h>

#include "fast_conversions.h"

/*
 * Fast-path conversions: direct pixel format transformations without intermediate RGBA.
 * These provide significant performance improvements (10-20x) for common conversion pairs.
 */

/* RGB24 ↔ BGR24: Simple byte swap */
static bool fast_convert_rgb24_bgr24(const struct sail_image* image_input, struct sail_image* image_output)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint8_t* scan_input = sail_scan_line(image_input, row);
        uint8_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[0] = scan_input[2]; /* B ← R or R ← B */
            scan_output[1] = scan_input[1]; /* G ← G */
            scan_output[2] = scan_input[0]; /* R ← B or B ← R */

            scan_input  += 3;
            scan_output += 3;
        }
    }

    return true;
}

/* RGB48 ↔ BGR48: Simple word swap */
static bool fast_convert_rgb48_bgr48(const struct sail_image* image_input, struct sail_image* image_output)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[0] = scan_input[2];
            scan_output[1] = scan_input[1];
            scan_output[2] = scan_input[0];

            scan_input  += 3;
            scan_output += 3;
        }
    }

    return true;
}

/* RGBA32 variants: RGBA ↔ BGRA, ARGB, ABGR */
static bool fast_convert_rgba32_variants(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int a_in,
                                         int r_out,
                                         int g_out,
                                         int b_out,
                                         int a_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint8_t* scan_input = sail_scan_line(image_input, row);
        uint8_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];
            scan_output[a_out] = scan_input[a_in];

            scan_input  += 4;
            scan_output += 4;
        }
    }

    return true;
}

/* RGBA64 variants */
static bool fast_convert_rgba64_variants(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int a_in,
                                         int r_out,
                                         int g_out,
                                         int b_out,
                                         int a_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];
            scan_output[a_out] = scan_input[a_in];

            scan_input  += 4;
            scan_output += 4;
        }
    }

    return true;
}

/* RGBA32 → RGB24: Drop alpha channel */
static bool fast_convert_rgba32_to_rgb24(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int r_out,
                                         int g_out,
                                         int b_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint8_t* scan_input = sail_scan_line(image_input, row);
        uint8_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];

            scan_input  += 4;
            scan_output += 3;
        }
    }

    return true;
}

/* RGBA64 → RGB48: Drop alpha channel */
static bool fast_convert_rgba64_to_rgb48(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int r_out,
                                         int g_out,
                                         int b_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];

            scan_input  += 4;
            scan_output += 3;
        }
    }

    return true;
}

/* RGB24 → RGBA32: Add opaque alpha */
static bool fast_convert_rgb24_to_rgba32(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int r_out,
                                         int g_out,
                                         int b_out,
                                         int a_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint8_t* scan_input = sail_scan_line(image_input, row);
        uint8_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];
            scan_output[a_out] = 255; /* Opaque */

            scan_input  += 3;
            scan_output += 4;
        }
    }

    return true;
}

/* RGB48 → RGBA64: Add opaque alpha */
static bool fast_convert_rgb48_to_rgba64(const struct sail_image* image_input,
                                         struct sail_image* image_output,
                                         int r_in,
                                         int g_in,
                                         int b_in,
                                         int r_out,
                                         int g_out,
                                         int b_out,
                                         int a_out)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            scan_output[r_out] = scan_input[r_in];
            scan_output[g_out] = scan_input[g_in];
            scan_output[b_out] = scan_input[b_in];
            scan_output[a_out] = 65535; /* Opaque */

            scan_input  += 3;
            scan_output += 4;
        }
    }

    return true;
}

/* RGB555 ↔ BGR555: Swap color bits */
static bool fast_convert_rgb555_bgr555(const struct sail_image* image_input, struct sail_image* image_output)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            const uint16_t pixel = *scan_input++;
            /* RGB555: bits 0-4 (R), 5-9 (G), 10-14 (B) → BGR555: swap R and B */
            *scan_output++ = ((pixel & 0x001F) << 10) | /* R → B */
                             (pixel & 0x03E0) |         /* G stays */
                             ((pixel & 0x7C00) >> 10);  /* B → R */
        }
    }

    return true;
}

/* RGB565 ↔ BGR565: Swap color bits */
static bool fast_convert_rgb565_bgr565(const struct sail_image* image_input, struct sail_image* image_output)
{
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < image_input->height; row++)
    {
        const uint16_t* scan_input = sail_scan_line(image_input, row);
        uint16_t* scan_output      = sail_scan_line(image_output, row);

        for (unsigned column = 0; column < image_input->width; column++)
        {
            const uint16_t pixel = *scan_input++;
            /* RGB565: bits 0-4 (R), 5-10 (G), 11-15 (B) → BGR565: swap R and B */
            *scan_output++ = ((pixel & 0x001F) << 11) | /* R → B */
                             (pixel & 0x07E0) |         /* G stays */
                             ((pixel & 0xF800) >> 11);  /* B → R */
        }
    }

    return true;
}

/* Identical format: direct memcpy */
static bool fast_convert_identical(const struct sail_image* image_input, struct sail_image* image_output)
{
    const size_t total_size = (size_t)image_input->height * image_input->bytes_per_line;
    memcpy(image_output->pixels, image_input->pixels, total_size);

    return true;
}

/* Main fast-path dispatcher */
bool sail_try_fast_conversion(const struct sail_image* image_input,
                              struct sail_image* image_output,
                              enum SailPixelFormat output_pixel_format)
{
    const enum SailPixelFormat input_format = image_input->pixel_format;

    /* Fast-path 1: Identical formats - just memcpy */
    if (input_format == output_pixel_format)
    {
        return fast_convert_identical(image_input, image_output);
    }

    /* Fast-path 2: RGB24 ↔ BGR24 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP24_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP24_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB))
    {
        return fast_convert_rgb24_bgr24(image_input, image_output);
    }

    /* Fast-path 3: RGB48 ↔ BGR48 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP48_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP48_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_RGB))
    {
        return fast_convert_rgb48_bgr48(image_input, image_output);
    }

    /* Fast-path 4: RGBA32 ↔ BGRA32 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP32_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA)
        || (input_format == SAIL_PIXEL_FORMAT_BPP32_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA))
    {
        return fast_convert_rgba32_variants(image_input, image_output, 0, 1, 2, 3, 2, 1, 0, 3);
    }

    /* Fast-path 5: RGBA32 ↔ ARGB32 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP32_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB)
        || (input_format == SAIL_PIXEL_FORMAT_BPP32_ARGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA))
    {
        return fast_convert_rgba32_variants(image_input, image_output, 0, 1, 2, 3, 1, 2, 3, 0);
    }

    /* Fast-path 6: RGBA32 ↔ ABGR32 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP32_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP32_ABGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA))
    {
        return fast_convert_rgba32_variants(image_input, image_output, 0, 1, 2, 3, 3, 2, 1, 0);
    }

    /* Fast-path 7: BGRA32 ↔ ARGB32 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP32_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB)
        || (input_format == SAIL_PIXEL_FORMAT_BPP32_ARGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA))
    {
        return fast_convert_rgba32_variants(image_input, image_output, 2, 1, 0, 3, 1, 2, 3, 0);
    }

    /* Fast-path 8: BGRA32 ↔ ABGR32 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP32_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP32_ABGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA))
    {
        return fast_convert_rgba32_variants(image_input, image_output, 2, 1, 0, 3, 3, 2, 1, 0);
    }

    /* Fast-path 9: RGBA64 ↔ BGRA64 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP64_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA)
        || (input_format == SAIL_PIXEL_FORMAT_BPP64_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_RGBA))
    {
        return fast_convert_rgba64_variants(image_input, image_output, 0, 1, 2, 3, 2, 1, 0, 3);
    }

    /* Fast-path 10: RGBA64 ↔ ARGB64 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP64_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_ARGB)
        || (input_format == SAIL_PIXEL_FORMAT_BPP64_ARGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_RGBA))
    {
        return fast_convert_rgba64_variants(image_input, image_output, 0, 1, 2, 3, 1, 2, 3, 0);
    }

    /* Fast-path 11: RGBA64 ↔ ABGR64 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP64_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP64_ABGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_RGBA))
    {
        return fast_convert_rgba64_variants(image_input, image_output, 0, 1, 2, 3, 3, 2, 1, 0);
    }

    /* Fast-path 12: BGRA64 ↔ ARGB64 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP64_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_ARGB)
        || (input_format == SAIL_PIXEL_FORMAT_BPP64_ARGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA))
    {
        return fast_convert_rgba64_variants(image_input, image_output, 2, 1, 0, 3, 1, 2, 3, 0);
    }

    /* Fast-path 13: BGRA64 ↔ ABGR64 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP64_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR)
        || (input_format == SAIL_PIXEL_FORMAT_BPP64_ABGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA))
    {
        return fast_convert_rgba64_variants(image_input, image_output, 2, 1, 0, 3, 3, 2, 1, 0);
    }

    /* Fast-path 14: RGBA32 → RGB24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 0, 1, 2, 0, 1, 2);
    }

    /* Fast-path 15: RGBA32 → BGR24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 0, 1, 2, 2, 1, 0);
    }

    /* Fast-path 16: BGRA32 → RGB24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 2, 1, 0, 0, 1, 2);
    }

    /* Fast-path 17: BGRA32 → BGR24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 2, 1, 0, 2, 1, 0);
    }

    /* Fast-path 18: ARGB32 → RGB24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_ARGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 1, 2, 3, 0, 1, 2);
    }

    /* Fast-path 19: ABGR32 → BGR24 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP32_ABGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR)
    {
        return fast_convert_rgba32_to_rgb24(image_input, image_output, 3, 2, 1, 2, 1, 0);
    }

    /* Fast-path 20: RGBA64 → RGB48 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP64_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_RGB)
    {
        return fast_convert_rgba64_to_rgb48(image_input, image_output, 0, 1, 2, 0, 1, 2);
    }

    /* Fast-path 21: RGBA64 → BGR48 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP64_RGBA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR)
    {
        return fast_convert_rgba64_to_rgb48(image_input, image_output, 0, 1, 2, 2, 1, 0);
    }

    /* Fast-path 22: BGRA64 → RGB48 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP64_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_RGB)
    {
        return fast_convert_rgba64_to_rgb48(image_input, image_output, 2, 1, 0, 0, 1, 2);
    }

    /* Fast-path 23: BGRA64 → BGR48 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP64_BGRA && output_pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR)
    {
        return fast_convert_rgba64_to_rgb48(image_input, image_output, 2, 1, 0, 2, 1, 0);
    }

    /* Fast-path 24: RGB24 → RGBA32 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP24_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA)
    {
        return fast_convert_rgb24_to_rgba32(image_input, image_output, 0, 1, 2, 0, 1, 2, 3);
    }

    /* Fast-path 25: RGB24 → BGRA32 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP24_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA)
    {
        return fast_convert_rgb24_to_rgba32(image_input, image_output, 0, 1, 2, 2, 1, 0, 3);
    }

    /* Fast-path 26: BGR24 → RGBA32 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP24_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA)
    {
        return fast_convert_rgb24_to_rgba32(image_input, image_output, 2, 1, 0, 0, 1, 2, 3);
    }

    /* Fast-path 27: BGR24 → BGRA32 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP24_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA)
    {
        return fast_convert_rgb24_to_rgba32(image_input, image_output, 2, 1, 0, 2, 1, 0, 3);
    }

    /* Fast-path 28: RGB48 → RGBA64 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP48_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_RGBA)
    {
        return fast_convert_rgb48_to_rgba64(image_input, image_output, 0, 1, 2, 0, 1, 2, 3);
    }

    /* Fast-path 29: RGB48 → BGRA64 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP48_RGB && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA)
    {
        return fast_convert_rgb48_to_rgba64(image_input, image_output, 0, 1, 2, 2, 1, 0, 3);
    }

    /* Fast-path 30: BGR48 → RGBA64 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP48_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_RGBA)
    {
        return fast_convert_rgb48_to_rgba64(image_input, image_output, 2, 1, 0, 0, 1, 2, 3);
    }

    /* Fast-path 31: BGR48 → BGRA64 */
    if (input_format == SAIL_PIXEL_FORMAT_BPP48_BGR && output_pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA)
    {
        return fast_convert_rgb48_to_rgba64(image_input, image_output, 2, 1, 0, 2, 1, 0, 3);
    }

    /* Fast-path 32: RGB555 ↔ BGR555 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP16_RGB555 && output_pixel_format == SAIL_PIXEL_FORMAT_BPP16_BGR555)
        || (input_format == SAIL_PIXEL_FORMAT_BPP16_BGR555 && output_pixel_format == SAIL_PIXEL_FORMAT_BPP16_RGB555))
    {
        return fast_convert_rgb555_bgr555(image_input, image_output);
    }

    /* Fast-path 33: RGB565 ↔ BGR565 */
    if ((input_format == SAIL_PIXEL_FORMAT_BPP16_RGB565 && output_pixel_format == SAIL_PIXEL_FORMAT_BPP16_BGR565)
        || (input_format == SAIL_PIXEL_FORMAT_BPP16_BGR565 && output_pixel_format == SAIL_PIXEL_FORMAT_BPP16_RGB565))
    {
        return fast_convert_rgb565_bgr565(image_input, image_output);
    }

    /* No fast-path available - use standard conversion */
    return false;
}
