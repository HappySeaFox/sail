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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "io.h"

void png_private_my_read_fn(png_structp png_ptr, png_bytep bytes, png_size_t bytes_size) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);

    sail_status_t err = io->strict_read(io->stream, bytes, bytes_size);

    if (err != SAIL_OK) {
        png_error(png_ptr, "Failed to read from the I/O stream");
    }
}

void png_private_my_write_fn(png_structp png_ptr, png_bytep bytes, png_size_t bytes_size) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);

    sail_status_t err = io->strict_write(io->stream, bytes, bytes_size);

    if (err != SAIL_OK) {
        png_error(png_ptr, "Failed to write to the I/O stream");
    }
}

void png_private_my_flush_fn(png_structp png_ptr) {

    if (png_ptr == NULL) {
        return;
    }

    struct sail_io *io = (struct sail_io *)png_get_io_ptr(png_ptr);

    sail_status_t err = io->flush(io->stream);

    if (err != SAIL_OK) {
        png_error(png_ptr, "Failed to flush the I/O stream");
    }
}
