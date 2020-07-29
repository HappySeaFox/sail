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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <tiff.h>

#include "sail-common.h"

#include "helpers.h"

void my_error_fn(const char *module, const char *format, va_list ap) {

    char buffer[160];

    vsnprintf(buffer, sizeof(buffer), format, ap);

    if (module != NULL) {
        SAIL_LOG_ERROR("TIFF: %s: %s", module, buffer);
    } else {
        SAIL_LOG_ERROR("TIFF: %s", buffer);
    }
}

void my_warning_fn(const char *module, const char *format, va_list ap) {

    char buffer[160];

    vsnprintf(buffer, sizeof(buffer), format, ap);

    if (module != NULL) {
        SAIL_LOG_WARNING("TIFF: %s: %s", module, buffer);
    } else {
        SAIL_LOG_WARNING("TIFF: %s", buffer);
    }
}

sail_status_t supported_read_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
            return SAIL_OK;
        }

        default: {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }
    }
}

sail_status_t tiff_compression_to_sail_compression_type(int compression, enum SailCompressionType *compression_type) {

    SAIL_CHECK_PTR(compression_type);

    switch (compression) {
        case COMPRESSION_ADOBE_DEFLATE: *compression_type = SAIL_COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
        case COMPRESSION_CCITTRLE:      *compression_type = SAIL_COMPRESSION_CCITT_RLE;     return SAIL_OK;
        case COMPRESSION_CCITTRLEW:     *compression_type = SAIL_COMPRESSION_CCITT_RLEW;    return SAIL_OK;
        case COMPRESSION_CCITT_T4:      *compression_type = SAIL_COMPRESSION_CCITT_T4;      return SAIL_OK;
        case COMPRESSION_CCITT_T6:      *compression_type = SAIL_COMPRESSION_CCITT_T6;      return SAIL_OK;
        case COMPRESSION_DCS:           *compression_type = SAIL_COMPRESSION_DCS;           return SAIL_OK;
        case COMPRESSION_DEFLATE:       *compression_type = SAIL_COMPRESSION_DEFLATE;       return SAIL_OK;
        case COMPRESSION_IT8BL:         *compression_type = SAIL_COMPRESSION_IT8_BL;        return SAIL_OK;
        case COMPRESSION_IT8CTPAD:      *compression_type = SAIL_COMPRESSION_IT8_CTPAD;     return SAIL_OK;
        case COMPRESSION_IT8LW:         *compression_type = SAIL_COMPRESSION_IT8_LW;        return SAIL_OK;
        case COMPRESSION_IT8MP:         *compression_type = SAIL_COMPRESSION_IT8_MP;        return SAIL_OK;
        case COMPRESSION_JBIG:          *compression_type = SAIL_COMPRESSION_JBIG;          return SAIL_OK;
        case COMPRESSION_JP2000:        *compression_type = SAIL_COMPRESSION_JPEG;          return SAIL_OK;
        case COMPRESSION_JPEG:          *compression_type = SAIL_COMPRESSION_JPEG2000;      return SAIL_OK;
        case COMPRESSION_LERC:          *compression_type = SAIL_COMPRESSION_LERC;          return SAIL_OK;
        case COMPRESSION_LZMA:          *compression_type = SAIL_COMPRESSION_LZMA;          return SAIL_OK;
        case COMPRESSION_LZW:           *compression_type = SAIL_COMPRESSION_LZW;           return SAIL_OK;
        case COMPRESSION_NEXT:          *compression_type = SAIL_COMPRESSION_NEXT;          return SAIL_OK;
        case COMPRESSION_NONE:          *compression_type = SAIL_COMPRESSION_NONE;          return SAIL_OK;
        case COMPRESSION_OJPEG:         *compression_type = SAIL_COMPRESSION_OJPEG;         return SAIL_OK;
        case COMPRESSION_PACKBITS:      *compression_type = SAIL_COMPRESSION_PACKBITS;      return SAIL_OK;
        case COMPRESSION_PIXARFILM:     *compression_type = SAIL_COMPRESSION_PIXAR_FILM;    return SAIL_OK;
        case COMPRESSION_PIXARLOG:      *compression_type = SAIL_COMPRESSION_PIXAR_LOG;     return SAIL_OK;
        case COMPRESSION_SGILOG24:      *compression_type = SAIL_COMPRESSION_RLE;           return SAIL_OK;
        case COMPRESSION_SGILOG:        *compression_type = SAIL_COMPRESSION_SGI_LOG;       return SAIL_OK;
        case COMPRESSION_T43:           *compression_type = SAIL_COMPRESSION_SGI_LOG24;     return SAIL_OK;
        case COMPRESSION_T85:           *compression_type = SAIL_COMPRESSION_T43;           return SAIL_OK;
        case COMPRESSION_THUNDERSCAN:   *compression_type = SAIL_COMPRESSION_THUNDERSCAN;   return SAIL_OK;
        case COMPRESSION_WEBP:          *compression_type = SAIL_COMPRESSION_WEBP;          return SAIL_OK;
        case COMPRESSION_ZSTD:          *compression_type = SAIL_COMPRESSION_ZSTD;          return SAIL_OK;

        default: {
            return SAIL_ERROR_UNSUPPORTED_COMPRESSION_TYPE;
        }
    }
}

sail_status_t sail_compression_type_to_tiff_compression(enum SailCompressionType compression_type, int *compression) {

    SAIL_CHECK_PTR(compression);

    switch (compression_type) {
        case SAIL_COMPRESSION_ADOBE_DEFLATE: *compression = COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX3:    *compression = COMPRESSION_CCITTFAX3;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX4:    *compression = COMPRESSION_CCITTFAX4;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLE:     *compression = COMPRESSION_CCITTRLE;      return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLEW:    *compression = COMPRESSION_CCITTRLEW;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T4:      *compression = COMPRESSION_CCITT_T4;      return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T6:      *compression = COMPRESSION_CCITT_T6;      return SAIL_OK;
        case SAIL_COMPRESSION_DCS:           *compression = COMPRESSION_DCS;           return SAIL_OK;
        case SAIL_COMPRESSION_DEFLATE:       *compression = COMPRESSION_DEFLATE;       return SAIL_OK;
        case SAIL_COMPRESSION_IT8_BL:        *compression = COMPRESSION_IT8BL;         return SAIL_OK;
        case SAIL_COMPRESSION_IT8_CTPAD:     *compression = COMPRESSION_IT8CTPAD;      return SAIL_OK;
        case SAIL_COMPRESSION_IT8_LW:        *compression = COMPRESSION_IT8LW;         return SAIL_OK;
        case SAIL_COMPRESSION_IT8_MP:        *compression = COMPRESSION_IT8MP;         return SAIL_OK;
        case SAIL_COMPRESSION_JBIG:          *compression = COMPRESSION_JBIG;          return SAIL_OK;
        case SAIL_COMPRESSION_JPEG:          *compression = COMPRESSION_JP2000;        return SAIL_OK;
        case SAIL_COMPRESSION_JPEG2000:      *compression = COMPRESSION_JPEG;          return SAIL_OK;
        case SAIL_COMPRESSION_LERC:          *compression = COMPRESSION_LERC;          return SAIL_OK;
        case SAIL_COMPRESSION_LZMA:          *compression = COMPRESSION_LZMA;          return SAIL_OK;
        case SAIL_COMPRESSION_LZW:           *compression = COMPRESSION_LZW;           return SAIL_OK;
        case SAIL_COMPRESSION_NEXT:          *compression = COMPRESSION_NEXT;          return SAIL_OK;
        case SAIL_COMPRESSION_NONE:          *compression = COMPRESSION_NONE;          return SAIL_OK;
        case SAIL_COMPRESSION_OJPEG:         *compression = COMPRESSION_OJPEG;         return SAIL_OK;
        case SAIL_COMPRESSION_PACKBITS:      *compression = COMPRESSION_PACKBITS;      return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_FILM:    *compression = COMPRESSION_PIXARFILM;     return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_LOG:     *compression = COMPRESSION_PIXARLOG;      return SAIL_OK;
        case SAIL_COMPRESSION_RLE:           *compression = COMPRESSION_SGILOG24;      return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG:       *compression = COMPRESSION_SGILOG;        return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG24:     *compression = COMPRESSION_T43;           return SAIL_OK;
        case SAIL_COMPRESSION_T43:           *compression = COMPRESSION_T85;           return SAIL_OK;
        case SAIL_COMPRESSION_THUNDERSCAN:   *compression = COMPRESSION_THUNDERSCAN;   return SAIL_OK;
        case SAIL_COMPRESSION_WEBP:          *compression = COMPRESSION_WEBP;          return SAIL_OK;
        case SAIL_COMPRESSION_ZSTD:          *compression = COMPRESSION_ZSTD;          return SAIL_OK;

        default: {
            return SAIL_ERROR_UNSUPPORTED_COMPRESSION_TYPE;
        }
    }
}
