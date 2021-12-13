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

#ifndef SAIL_ICO_HELPERS_H
#define SAIL_ICO_HELPERS_H

#include <stdint.h>

#include "error.h"
#include "export.h"

struct sail_io;

/* File header. */
struct SailIcoHeader
{
    uint16_t reserved;
    uint16_t type; /* 1 = ICO, 2 = CUR. */
    uint16_t images_count;
};

struct SailIcoDirEntry
{
    uint8_t width;
    uint8_t height;
    uint8_t color_count; /* 0 when full color. */
    uint8_t reserved;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t image_size;
    uint32_t image_offset;
};

enum SailIcoImageType
{
    SAIL_ICO_IMAGE_BMP,
    SAIL_ICO_IMAGE_PNG,
};

SAIL_HIDDEN sail_status_t ico_private_read_header(struct sail_io *io, struct SailIcoHeader *header);

SAIL_HIDDEN sail_status_t ico_private_read_dir_entry(struct sail_io *io, struct SailIcoDirEntry *dir_entry);

SAIL_HIDDEN sail_status_t ico_private_probe_image_type(struct sail_io *io, enum SailIcoImageType *ico_image_type);

#endif
