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

#ifndef SAIL_JPEG_IO_SRC_H
#define SAIL_JPEG_IO_SRC_H

#include <stdio.h>

#include <jpeglib.h>

#include "export.h"

struct sail_io;

/*
 * Expanded data source object for input.
 */
struct sail_jpeg_source_mgr {
    struct jpeg_source_mgr pub;   /* public fields */

    struct sail_io *io;           /* source stream */
    JOCTET *buffer;               /* start of buffer */
    boolean start_of_file;        /* have we gotten any data yet? */
};

SAIL_HIDDEN void jpeg_private_sail_io_src(j_decompress_ptr cinfo, struct sail_io *io);

#endif
