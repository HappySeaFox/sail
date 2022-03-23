/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#include "helpers.h"

static const unsigned char reverse_lookup_4bits[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
    0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

unsigned char xbm_private_reverse_byte(unsigned char byte) {

   return (reverse_lookup_4bits[byte & 0xF] << 4) | reverse_lookup_4bits[byte >> 4];
}

sail_status_t xbm_private_skip_comments(struct sail_io *io, char *str, size_t str_size, size_t *str_start_pos) {

/*
    size_t pos;

    do
    {
        SAIL_TRY(io->tell(io->stream, &pos));

        char str[512 + 1];
        unsigned i = 0;

        do {
            SAIL_TRY(io->strict_read(io->stream, str + i++, 1));
        } while(i < sizeof(str) - 1 || str[i - 1] != '\n');

        if (strstr(str, "/*") == NULL) {
            break;
        }
    } while(true);


        SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));
    fsetpos(fp, (fpos_t*)&pos);
*/

    return SAIL_OK;
}
