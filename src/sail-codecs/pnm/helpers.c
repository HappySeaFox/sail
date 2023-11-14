/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#include <sail-common/sail-common.h>

#include "helpers.h"

sail_status_t pnm_private_skip_to_data(struct sail_io *io, char *first_char) {

    char c;

    do {
        SAIL_TRY(io->strict_read(io->stream, &c, 1));

        if (c == '#') {
            do {
                SAIL_TRY(io->strict_read(io->stream, &c, 1));
            } while(c != '\n');
        }
    } while(c < '0' || c > '9');

    *first_char = c;

    return SAIL_OK;
}

enum SailPixelFormat pnm_private_rgb_sail_pixel_format(enum SailPnmVersion pnm_version, unsigned bpp) {

    switch (pnm_version) {
        case SAIL_PNM_VERSION_P1:
        case SAIL_PNM_VERSION_P4: return SAIL_PIXEL_FORMAT_BPP1_INDEXED;

        case SAIL_PNM_VERSION_P2:
        case SAIL_PNM_VERSION_P5: {
            switch (bpp) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        case SAIL_PNM_VERSION_P3:
        case SAIL_PNM_VERSION_P6: {
            switch (bpp) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

