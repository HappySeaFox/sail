/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#ifndef SAIL_PSD_HELPERS_H
#define SAIL_PSD_HELPERS_H

#include <stdint.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

/* PSD pixel formats. */
enum SailPsdMode {
    SAIL_PSD_MODE_BITMAP       = 0,
    SAIL_PSD_MODE_GRAYSCALE    = 1,
    SAIL_PSD_MODE_INDEXED      = 2,
    SAIL_PSD_MODE_RGB          = 3,
    SAIL_PSD_MODE_CMYK         = 4,
    SAIL_PSD_MODE_MULTICHANNEL = 7,
    SAIL_PSD_MODE_DUOTONE      = 8,
    SAIL_PSD_MODE_LAB          = 9,
};

/* PSD compressions. */
enum SailPsdCompression {
    SAIL_PSD_COMPRESSION_NONE                   = 0,
    SAIL_PSD_COMPRESSION_RLE                    = 1,
    SAIL_PSD_COMPRESSION_ZIP_WITHOUT_PREDICTION = 2,
    SAIL_PSD_COMPRESSION_ZIP_WITH_PREDICTION    = 3,
};

struct sail_io;

SAIL_HIDDEN sail_status_t psd_private_get_big_endian_uint16_t(struct sail_io *io, uint16_t *v);

SAIL_HIDDEN sail_status_t psd_private_get_big_endian_uint32_t(struct sail_io *io, uint32_t *v);

SAIL_HIDDEN sail_status_t psd_private_sail_pixel_format(enum SailPsdMode mode, uint16_t channels, uint16_t depth, enum SailPixelFormat *result);

SAIL_HIDDEN enum SailCompression psd_private_sail_compression(enum SailPsdCompression compression);

#endif
