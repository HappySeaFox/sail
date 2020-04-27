/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#ifndef SAIL_JPEG_IO_DEST_H
#define SAIL_JPEG_IO_DEST_H

#include <jpeglib.h>

#include "export.h"

struct sail_io;

struct sail_jpeg_destination_mgr {
    struct jpeg_destination_mgr pub; /* public fields */

    struct sail_io *io;                /* target stream */
    JOCTET *buffer;             /* start of buffer */
};

SAIL_HIDDEN void jpeg_sail_io_dest(j_compress_ptr cinfo, struct sail_io *io);

#endif
