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

sail_status_t sail_read_pixel3_uint8(struct sail_io *io, struct sail_pixel3_uint8 *pixel) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(pixel);

    uint8_t a[3];

    SAIL_TRY(io->strict_read(io->stream, a, sizeof(a)));

    pixel->component1 = a[0];
    pixel->component2 = a[1];
    pixel->component3 = a[2];

    return SAIL_OK;
}

sail_status_t sail_read_pixel4_uint8(struct sail_io *io, struct sail_pixel4_uint8 *pixel) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(pixel);

    uint8_t a[4];

    SAIL_TRY(io->strict_read(io->stream, a, sizeof(a)));

    pixel->component1 = a[0];
    pixel->component2 = a[1];
    pixel->component3 = a[2];
    pixel->component4 = a[3];

    return SAIL_OK;
}

sail_status_t sail_read_pixel3_uint16(struct sail_io *io, struct sail_pixel3_uint16 *pixel) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(pixel);

    uint16_t a[3];

    SAIL_TRY(io->strict_read(io->stream, a, sizeof(a)));

    pixel->component1 = a[0];
    pixel->component2 = a[1];
    pixel->component3 = a[2];

    return SAIL_OK;
}

sail_status_t sail_read_pixel4_uint16(struct sail_io *io, struct sail_pixel4_uint16 *pixel) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(pixel);

    uint16_t a[4];

    SAIL_TRY(io->strict_read(io->stream, a, sizeof(a)));

    pixel->component1 = a[0];
    pixel->component2 = a[1];
    pixel->component3 = a[2];
    pixel->component4 = a[3];

    return SAIL_OK;
}
