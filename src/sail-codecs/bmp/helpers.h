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

#ifndef SAIL_BMP_HELPERS_H
#define SAIL_BMP_HELPERS_H

#include <stdint.h>

#include "common.h"
#include "error.h"
#include "export.h"
#include "pixel.h"

struct sail_iccp;
struct sail_io;

/* RLE markers. */
static const uint8_t SAIL_UNENCODED_RUN_MARKER    = 0;
static const uint8_t SAIL_END_OF_SCAN_LINE_MARKER = 0;
static const uint8_t SAIL_END_OF_RLE_DATA_MARKER  = 1;
static const uint8_t SAIL_DELTA_MARKER            = 2;

/*
 * V1: Device-Dependent Bitmap (DDB).
 */

/* File header. */
struct SailBmpDdbFileHeader
{
    uint16_t type; /* Always 2. Top bit set if discardable. */
};

/* Bitmap16. */
struct SailBmpDdbBitmap
{
    uint16_t type; /* Always 0. */
    uint16_t width;
    uint16_t height;
    uint16_t byte_width;
    uint8_t  planes; /* Always 1. */
    uint8_t  bit_count;
    uint32_t pixels; /* Always 0. */
};

/*
 * V2+: File header + DIB headers.
 */

/* File header. */
struct SailBmpDibFileHeader
{
    uint16_t type; /* "BM" */
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

/* DIB headers. */
struct SailBmpDibHeaderV2
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes; /* Always 1. */
    uint16_t bit_count;
};

struct SailBmpDibHeaderV3
{
    uint32_t compression;
    uint32_t bitmap_size;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};

struct SailBmpDibHeaderV4
{
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t color_space_type;
    int32_t  red_x;
    int32_t  red_y;
    int32_t  red_z;
    int32_t  green_x;
    int32_t  green_y;
    int32_t  green_z;
    int32_t  blue_x;
    int32_t  blue_y;
    int32_t  blue_z;
    uint32_t gamma_red;
    uint32_t gamma_green;
    uint32_t gamma_blue;
};

struct SailBmpDibHeaderV5
{
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t reserved;
};

enum SailBmpVersion
{
    SAIL_BMP_V1 = 1,
    SAIL_BMP_V2,
    SAIL_BMP_V3,
    SAIL_BMP_V4,
    SAIL_BMP_V5,
};

SAIL_HIDDEN sail_status_t bmp_private_read_ddb_file_header(struct sail_io *io, struct SailBmpDdbFileHeader *ddb_file_header);

SAIL_HIDDEN sail_status_t bmp_private_read_v1(struct sail_io *io, struct SailBmpDdbBitmap *v1);

SAIL_HIDDEN sail_status_t bmp_private_read_dib_file_header(struct sail_io *io, struct SailBmpDibFileHeader *fh);

SAIL_HIDDEN sail_status_t bmp_private_read_v2(struct sail_io *io, struct SailBmpDibHeaderV2 *v2);

SAIL_HIDDEN sail_status_t bmp_private_read_v3(struct sail_io *io, struct SailBmpDibHeaderV3 *v3);

SAIL_HIDDEN sail_status_t bmp_private_read_v4(struct sail_io *io, struct SailBmpDibHeaderV4 *v4);

SAIL_HIDDEN sail_status_t bmp_private_read_v5(struct sail_io *io, struct SailBmpDibHeaderV5 *v5);

SAIL_HIDDEN sail_status_t bmp_private_bit_count_to_pixel_format(uint16_t bit_count, enum SailPixelFormat *pixel_format);

SAIL_HIDDEN sail_status_t bmp_private_fetch_iccp(struct sail_io *io, long offset_of_data, uint32_t profile_size, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t bmp_private_skip_end_of_scan_line(struct sail_io *io);

SAIL_HIDDEN sail_status_t bmp_private_bytes_in_row(unsigned width, unsigned bit_count, unsigned *bytes_in_row);

SAIL_HIDDEN unsigned bmp_private_pad_bytes(unsigned bytes_in_row);

SAIL_HIDDEN sail_status_t bmp_private_fill_system_palette(unsigned bit_count, sail_rgb24_t **palette, unsigned *palette_count);

#endif
