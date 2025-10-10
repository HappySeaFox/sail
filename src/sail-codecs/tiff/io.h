/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#pragma once

#include <tiffio.h>

#include <sail-common/export.h>

SAIL_HIDDEN tmsize_t tiff_private_my_read_proc(thandle_t client_data, void* buffer, tmsize_t buffer_size);

SAIL_HIDDEN tmsize_t tiff_private_my_write_proc(thandle_t client_data, void* buffer, tmsize_t buffer_size);

SAIL_HIDDEN toff_t tiff_private_my_seek_proc(thandle_t client_data, toff_t offset, int whence);

SAIL_HIDDEN int tiff_private_my_dummy_close_proc(thandle_t client_data);

SAIL_HIDDEN toff_t tiff_private_my_dummy_size_proc(thandle_t client_data);
