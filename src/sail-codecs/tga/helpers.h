/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_TGA_HELPERS_H
#define SAIL_TGA_HELPERS_H

#include <stdint.h>

#include "common.h"
#include "error.h"
#include "export.h"

struct sail_io;
struct sail_meta_data_node;
struct sail_palette;

enum TgaColorMapType {

    TGA_HAS_NO_COLOR_MAP = 0,
    TGA_HAS_COLOR_MAP    = 1,
};

enum TgaImageType {

    TGA_NO_IMAGE       = 0,
    TGA_INDEXED        = 1,
    TGA_TRUE_COLOR     = 2,
    TGA_GRAY           = 3,
    TGA_INDEXED_RLE    = 9,
    TGA_TRUE_COLOR_RLE = 10,
    TGA_GRAY_RLE       = 11,
};

struct TgaFileHeader
{
    uint8_t  id_length;
    uint8_t  color_map_type; /* See TgaColorMapType. */
    uint8_t  image_type;     /* See TgaImageType. */
    uint16_t first_color_map_entry_index;
    uint16_t color_map_elements;
    uint8_t  color_map_entry_size; /* 15, 16, 24, 32. */
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t  bpp; /* 8, 16, 24, 32.  */
    uint8_t  descriptor; /* bits 3-0: n of alpha bits, bit 4: flipped H, bit 5 = flipped V. */
};

struct TgaFooter
{
    uint32_t extension_area_offset;
    uint32_t developer_area_offset;
    uint8_t  signature[18];    /* "TRUEVISION-XFILE.\0" */
};

SAIL_HIDDEN sail_status_t tga_private_read_file_header(struct sail_io *io, struct TgaFileHeader *file_header);

SAIL_HIDDEN sail_status_t tga_private_read_file_footer(struct sail_io *io, struct TgaFooter *footer);

SAIL_HIDDEN enum SailPixelFormat tga_private_sail_pixel_format(int image_type, int bpp);

SAIL_HIDDEN enum SailPixelFormat tga_private_palette_bpp_to_sail_pixel_format(int bpp);

SAIL_HIDDEN sail_status_t tga_private_fetch_id(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_meta_data_node **meta_data_node);

SAIL_HIDDEN sail_status_t tga_private_fetch_extension(struct sail_io *io, double *gamma, struct sail_meta_data_node **meta_data_node);

SAIL_HIDDEN sail_status_t tga_private_fetch_palette(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_palette **palette);

#endif
