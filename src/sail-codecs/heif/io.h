/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#pragma once

#include <libheif/heif.h>

#include <sail-common/export.h>

struct sail_io;

struct sail_heif_reader_context
{
    struct sail_io* io;
    void* buffer;
    size_t buffer_size;
};

struct sail_heif_writer_context
{
    struct sail_io* io;
};

SAIL_HIDDEN int64_t heif_private_reader_get_position(void* user_data);

SAIL_HIDDEN int heif_private_reader_read(void* data, size_t size, void* user_data);

SAIL_HIDDEN int heif_private_reader_seek(int64_t position, void* user_data);

SAIL_HIDDEN enum heif_reader_grow_status heif_private_reader_wait_for_file_size(int64_t target_size, void* user_data);

SAIL_HIDDEN struct heif_error heif_private_writer_write(struct heif_context* ctx, const void* data, size_t size, void* user_data);
