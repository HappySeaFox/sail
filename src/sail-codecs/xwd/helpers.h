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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_io;
struct sail_image;
struct sail_palette;

/* XWD file format constants. */
#define XWD_FILE_VERSION 7
#define XWD_HEADER_SIZE 100

/* XWD pixmap formats. */
#define XYBitmap 0
#define XYPixmap 1
#define ZPixmap  2

/* Visual classes. */
#define StaticGray  0
#define GrayScale   1
#define StaticColor 2
#define PseudoColor 3
#define TrueColor   4
#define DirectColor 5

/* Byte order. */
#define LSBFirst 0
#define MSBFirst 1

/* Bitmap bit order. */
#define LeastSignificant 0
#define MostSignificant  1

/*
 * XWD file header structure (100 bytes, 25 uint32_t fields).
 */
struct XWDFileHeader {
    uint32_t header_size;          /* Size of the entire file header (in bytes). */
    uint32_t file_version;         /* XWD file version (always 7). */
    uint32_t pixmap_format;        /* Pixmap format (XYBitmap, XYPixmap, ZPixmap). */
    uint32_t pixmap_depth;         /* Pixmap depth. */
    uint32_t pixmap_width;         /* Pixmap width. */
    uint32_t pixmap_height;        /* Pixmap height. */
    uint32_t xoffset;              /* Bitmap x offset. */
    uint32_t byte_order;           /* Byte order (LSBFirst or MSBFirst). */
    uint32_t bitmap_unit;          /* Bitmap unit. */
    uint32_t bitmap_bit_order;     /* Bitmap bit order. */
    uint32_t bitmap_pad;           /* Bitmap scanline pad. */
    uint32_t bits_per_pixel;       /* Bits per pixel. */
    uint32_t bytes_per_line;       /* Bytes per scanline. */
    uint32_t visual_class;         /* Visual class. */
    uint32_t red_mask;             /* Red mask. */
    uint32_t green_mask;           /* Green mask. */
    uint32_t blue_mask;            /* Blue mask. */
    uint32_t bits_per_rgb;         /* Bits per RGB. */
    uint32_t colormap_entries;     /* Number of colormap entries. */
    uint32_t ncolors;              /* Number of colors. */
    uint32_t window_width;         /* Window width. */
    uint32_t window_height;        /* Window height. */
    uint32_t window_x;             /* Window x coordinate. */
    uint32_t window_y;             /* Window y coordinate. */
    uint32_t window_border_width;  /* Window border width. */
};

/*
 * XWD color entry (12 bytes).
 */
struct XWDColor {
    uint32_t pixel;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint8_t flags;
    uint8_t pad;
};

SAIL_HIDDEN sail_status_t xwd_private_read_header(struct sail_io *io,
                                                   struct XWDFileHeader *header);

SAIL_HIDDEN sail_status_t xwd_private_write_header(struct sail_io *io,
                                                    const struct XWDFileHeader *header);

SAIL_HIDDEN sail_status_t xwd_private_read_colormap(struct sail_io *io,
                                                     uint32_t ncolors,
                                                     bool byte_swap,
                                                     struct XWDColor **colormap);

SAIL_HIDDEN sail_status_t xwd_private_write_colormap(struct sail_io *io,
                                                      const struct XWDColor *colormap,
                                                      uint32_t ncolors);

SAIL_HIDDEN sail_status_t xwd_private_read_pixels(struct sail_io *io,
                                                   const struct XWDFileHeader *header,
                                                   const struct XWDColor *colormap,
                                                   struct sail_image *image);

SAIL_HIDDEN sail_status_t xwd_private_write_pixels(struct sail_io *io,
                                                    const struct XWDFileHeader *header,
                                                    const struct sail_image *image);

SAIL_HIDDEN enum SailPixelFormat xwd_private_pixel_format_from_header(const struct XWDFileHeader *header);

SAIL_HIDDEN sail_status_t xwd_private_header_from_image(const struct sail_image *image,
                                                         struct XWDFileHeader *header);

SAIL_HIDDEN sail_status_t xwd_private_palette_to_colormap(const struct sail_palette *palette,
                                                           struct XWDColor **colormap,
                                                           uint32_t *ncolors);

SAIL_HIDDEN bool xwd_private_is_native_byte_order(uint32_t byte_order);
