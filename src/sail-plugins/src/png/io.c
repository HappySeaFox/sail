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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "io.h"

void my_read_fn(png_structp png_ptr, png_bytep bytes, png_size_t bytes_size) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);
    size_t nbytes;

    sail_error_t err = io->read(io->stream, bytes, 1, bytes_size, &nbytes);

    if (err != 0 || nbytes != bytes_size) {
        png_error(png_ptr, "Failed to read from the I/O stream");
    }
}

void my_write_fn(png_structp png_ptr, png_bytep bytes, png_size_t bytes_size) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);
    size_t nbytes;

    sail_error_t err = io->write(io->stream, bytes, 1, bytes_size, &nbytes);

    if (err != 0 || nbytes != bytes_size) {
        png_error(png_ptr, "Failed to write to the I/O stream");
    }
}

void my_flush_fn(png_structp png_ptr) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);

    sail_error_t err = io->flush(io->stream);

    if (err != 0) {
        png_error(png_ptr, "Failed to flush the I/O stream");
    }
}
