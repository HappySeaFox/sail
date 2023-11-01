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

#include <sail-common/sail-common.h>

#include "helpers.h"

sail_status_t psd_private_get_big_endian_uint16_t(struct sail_io *io, uint16_t *v)
{
    SAIL_TRY(io->strict_read(io->stream, v, sizeof(*v)));

    *v = sail_reverse_uint16(*v);

    return SAIL_OK;
}

sail_status_t psd_private_get_big_endian_uint32_t(struct sail_io *io, uint32_t *v)
{
    SAIL_TRY(io->strict_read(io->stream, v, sizeof(*v)));

    *v = sail_reverse_uint32(*v);

    return SAIL_OK;
}

sail_status_t psd_private_sail_pixel_format(enum SailPsdMode mode, uint16_t channels, uint16_t depth, enum SailPixelFormat *result) {

    switch (mode) {
        case SAIL_PSD_MODE_BITMAP: {
            switch (channels) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED; return SAIL_OK;
            }
            break;
        }
        case SAIL_PSD_MODE_INDEXED: {
            switch (channels) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED; return SAIL_OK;
            }
            break;
        }
        case SAIL_PSD_MODE_GRAYSCALE: {
            switch (channels) {
                case 1: {
                    switch (depth) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;  return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        case SAIL_PSD_MODE_RGB: {
            switch (channels) {
                case 3: {
                    switch(depth) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP24_RGB; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP48_RGB; return SAIL_OK;
                    }
                    break;
                }
                case 4: {
                    switch (depth) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP32_RGBA; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP64_RGBA; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        case SAIL_PSD_MODE_CMYK: {
            switch (channels) {
                /* TODO: 5 channels? */
                case 4: {
                    switch (depth) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP32_CMYK; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP64_CMYK; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    SAIL_LOG_ERROR("PSD: Unsuppored combination of mode(%u) and channels(%u)", mode, channels);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

enum SailCompression psd_private_sail_compression(enum SailPsdCompression compression)
{
    switch (compression) {
        case SAIL_PSD_COMPRESSION_NONE:                   return SAIL_COMPRESSION_NONE;
        case SAIL_PSD_COMPRESSION_RLE:                    return SAIL_COMPRESSION_RLE;
        case SAIL_PSD_COMPRESSION_ZIP_WITHOUT_PREDICTION: return SAIL_COMPRESSION_ZIP;
        case SAIL_PSD_COMPRESSION_ZIP_WITH_PREDICTION:    return SAIL_COMPRESSION_ZIP;
        default: return SAIL_COMPRESSION_UNKNOWN;
    }
}
