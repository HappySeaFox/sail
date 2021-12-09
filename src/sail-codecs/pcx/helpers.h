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

#ifndef SAIL_PCX_HELPERS_H
#define SAIL_PCX_HELPERS_H

#include <stdint.h>

#include "common.h"
#include "error.h"
#include "export.h"

enum SailPcxVersion
{
    SAIL_PCX_V0 = 0,
    SAIL_PCX_V2 = 2,
    SAIL_PCX_V3 = 3,
    SAIL_PCX_V4 = 4,
    SAIL_PCX_V5 = 5,
};

enum SailPcxEncoding
{
    SAIL_PCX_NO_ENCODING  = 0,
    SAIL_PCX_RLE_ENCODING = 1,
};

enum SailPcxPaletteInfo
{
    SAIL_PCX_PALETTE_COLOR     = 1,
    SAIL_PCX_PALETTE_GRAYSCALE = 2,
};

struct SailPcxHeader
{
    uint8_t  id;
    uint8_t  version;
    uint8_t  encoding;
    uint8_t  bits_per_plane; /* 1, 2, 4, 8. */
    uint16_t xmin;
    uint16_t ymin;
    uint16_t xmax;
    uint16_t ymax;
    uint16_t hdpi;
    uint16_t vdpi;
    uint8_t  palette[48];
    uint8_t  reserved;
    uint8_t  planes; /* 1, 3, 4. */
    uint16_t bytes_per_line; /* Per plane. */
    uint16_t palette_info;
    uint16_t hscreen_size;
    uint16_t vscreen_size;
    uint8_t  filler[54];
};

SAIL_HIDDEN sail_status_t pcx_private_read_header(struct sail_io *io, struct SailPcxHeader *header);

SAIL_HIDDEN sail_status_t pcx_private_sail_pixel_format(unsigned bits_per_plane, unsigned planes, enum SailPcxPaletteInfo palette_info, enum SailPixelFormat *result);

SAIL_HIDDEN sail_status_t pcx_private_build_palette(enum SailPixelFormat pixel_format, struct sail_io *io, uint8_t palette16[48], struct sail_palette **palette);

SAIL_HIDDEN sail_status_t pcx_private_read_uncompressed(struct sail_io *io, unsigned bytes_per_plane_to_read, unsigned planes, unsigned char *buffer, struct sail_image *image);

#endif
