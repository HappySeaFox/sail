/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

sail_status_t psd_private_get_big_endian_uint16_t(struct sail_io *io, uint16_t *v)
{
    SAIL_TRY(io->strict_read(io->stream, v, sizeof(uint16_t)));

    unsigned char *view = (unsigned char *)v;

    *v = (view[0] << 8) + view[1];

    return SAIL_OK;
}

sail_status_t psd_private_get_big_endian_uint32_t(struct sail_io *io, uint32_t *v)
{
    SAIL_TRY(io->strict_read(io->stream, v, sizeof(uint32_t)));

    unsigned char *view = (unsigned char *)v;

    *v = (view[0] << 24) + (view[1] << 16) + (view[2] << 8) + view[3];

    return SAIL_OK;
}

sail_status_t psd_private_sail_pixel_format(enum SailPsdMode mode, uint16_t channels, enum SailPixelFormat *result) {

    switch (mode) {
        case SAIL_PSD_MODE_INDEXED: {
            switch (channels) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED; return SAIL_OK;
            }
            break;
        }
        case SAIL_PSD_MODE_GRAYSCALE: {
            switch (channels) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE; return SAIL_OK;
            }
            break;
        }
        case SAIL_PSD_MODE_RGB: {
            switch (channels) {
                case 4: *result = SAIL_PIXEL_FORMAT_BPP24_RGB;  return SAIL_OK;
                case 5: *result = SAIL_PIXEL_FORMAT_BPP32_RGBA; return SAIL_OK;
            }
            break;
        }
        case SAIL_PSD_MODE_CMYK: {
            switch (channels) {
                /* TODO: 5 channels? */
                case 4: *result = SAIL_PIXEL_FORMAT_BPP32_CMYK; return SAIL_OK;
            }
            break;
        }
    }

    SAIL_LOG_ERROR("PSD: Unsuppored combination of mode(%u) and channels(%u)", mode, channels);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}
