/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_JPEG_IO_SRC_H
#define SAIL_JPEG_IO_SRC_H

#include <setjmp.h>

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

SAIL_HIDDEN void init_source(j_decompress_ptr cinfo);

SAIL_HIDDEN boolean fill_input_buffer(j_decompress_ptr cinfo);

SAIL_HIDDEN void skip_input_data(j_decompress_ptr cinfo, long num_bytes);

SAIL_HIDDEN void term_source(j_decompress_ptr cinfo);

SAIL_HIDDEN void jpeg_sail_io_src(j_decompress_ptr cinfo, struct sail_io *io);

#endif
