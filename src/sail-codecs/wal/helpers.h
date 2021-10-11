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

#ifndef SAIL_WAL_HELPERS_H
#define SAIL_WAL_HELPERS_H

#include "common.h"
#include "error.h"
#include "export.h"

struct WalFileHeader
{
    char name[32];
    unsigned width;
    unsigned height;
    int offset[4];
    char next_name[32];
    unsigned flags;
    unsigned contents;
    unsigned value;
};

SAIL_HIDDEN sail_status_t wal_private_read_file_header(struct sail_io *io, struct WalFileHeader *wal_header);

SAIL_HIDDEN sail_status_t wal_private_assign_palette(struct sail_image *image);

SAIL_HIDDEN sail_status_t wal_private_assign_meta_data(const struct WalFileHeader *wal_header, struct sail_meta_data_node **meta_data_node);

#endif
