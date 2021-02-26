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

#include <string.h>

#include "sail-common.h"

sail_status_t sail_read_sail_pixel4_uint8(struct sail_io *io, struct sail_pixel4_uint8 *pixel) {

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_PIXEL_PTR(pixel);

    SAIL_TRY(io->strict_read(io->stream, &pixel->component1, sizeof(pixel->component1)));
    SAIL_TRY(io->strict_read(io->stream, &pixel->component2, sizeof(pixel->component2)));
    SAIL_TRY(io->strict_read(io->stream, &pixel->component3, sizeof(pixel->component3)));
    SAIL_TRY(io->strict_read(io->stream, &pixel->component4, sizeof(pixel->component4)));

    return SAIL_OK;
}
