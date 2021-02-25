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

#include "sail-common.h"

#include "io.h"

tmsize_t tiff_private_my_read_proc(thandle_t client_data, void *buffer, tmsize_t buffer_size) {

    struct sail_io *io = (struct sail_io *)client_data;
    size_t nbytes;

    sail_status_t err = io->tolerant_read(io->stream, buffer, buffer_size, &nbytes);

    if (err != SAIL_OK) {
        TIFFError(NULL, "Failed to read from the I/O stream: %d", err);
        return (tmsize_t)-1;
    }

    return (tmsize_t)nbytes;
}

tmsize_t tiff_private_my_write_proc(thandle_t client_data, void *buffer, tmsize_t buffer_size) {

    struct sail_io *io = (struct sail_io *)client_data;
    size_t nbytes;

    sail_status_t err = io->tolerant_write(io->stream, buffer, buffer_size, &nbytes);

    if (err != SAIL_OK) {
        TIFFError(NULL, "Failed to write to the I/O stream: %d", err);
        return (tmsize_t)-1;
    }

    return (tmsize_t)nbytes;
}

toff_t tiff_private_my_seek_proc(thandle_t client_data, toff_t offset, int whence) {

    struct sail_io *io = (struct sail_io *)client_data;

    sail_status_t err = io->seek(io->stream, (long)offset, whence);

    if (err != SAIL_OK) {
        TIFFError(NULL, "Failed to seek the I/O stream: %d", err);
        return (toff_t)-1;
    }

    size_t new_offset;
    err = io->tell(io->stream, &new_offset);

    if (err != SAIL_OK) {
        TIFFError(NULL, "Failed to get the current position of the I/O stream: %d", err);
        return (toff_t)-1;
    }

    return (toff_t)new_offset;
}

int tiff_private_my_dummy_close_proc(thandle_t client_data) {

    (void)client_data;

    return 0;
}

toff_t tiff_private_my_dummy_size_proc(thandle_t client_data) {

    (void)client_data;

    return (toff_t)-1;
}
