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

void tiff_private_my_error_fn(const char *module, const char *format, va_list ap) {

    char buffer[160];

    vsnprintf(buffer, sizeof(buffer), format, ap);

    if (module != NULL) {
        SAIL_LOG_ERROR("TIFF: %s: %s", module, buffer);
    } else {
        SAIL_LOG_ERROR("TIFF: %s", buffer);
    }
}

void tiff_private_my_warning_fn(const char *module, const char *format, va_list ap) {

    char buffer[160];

    vsnprintf(buffer, sizeof(buffer), format, ap);

    if (module != NULL) {
        SAIL_LOG_WARNING("TIFF: %s: %s", module, buffer);
    } else {
        SAIL_LOG_WARNING("TIFF: %s", buffer);
    }
}

sail_status_t tiff_private_supported_read_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_SOURCE: {
            return SAIL_OK;
        }

        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }
}

enum SailCompression tiff_private_compression_to_sail_compression(int compression) {

    switch (compression) {
        case COMPRESSION_ADOBE_DEFLATE: return SAIL_COMPRESSION_ADOBE_DEFLATE;
        case COMPRESSION_CCITTRLE:      return SAIL_COMPRESSION_CCITT_RLE;
        case COMPRESSION_CCITTRLEW:     return SAIL_COMPRESSION_CCITT_RLEW;
        case COMPRESSION_CCITT_T4:      return SAIL_COMPRESSION_CCITT_T4;
        case COMPRESSION_CCITT_T6:      return SAIL_COMPRESSION_CCITT_T6;
        case COMPRESSION_DCS:           return SAIL_COMPRESSION_DCS;
        case COMPRESSION_DEFLATE:       return SAIL_COMPRESSION_DEFLATE;
        case COMPRESSION_IT8BL:         return SAIL_COMPRESSION_IT8_BL;
        case COMPRESSION_IT8CTPAD:      return SAIL_COMPRESSION_IT8_CTPAD;
        case COMPRESSION_IT8LW:         return SAIL_COMPRESSION_IT8_LW;
        case COMPRESSION_IT8MP:         return SAIL_COMPRESSION_IT8_MP;
        case COMPRESSION_JBIG:          return SAIL_COMPRESSION_JBIG;
        case COMPRESSION_JPEG:          return SAIL_COMPRESSION_JPEG;
        case COMPRESSION_JP2000:        return SAIL_COMPRESSION_JPEG2000;
#ifdef HAVE_TIFF_41
        case COMPRESSION_LERC:          return SAIL_COMPRESSION_LERC;
#endif
        case COMPRESSION_LZMA:          return SAIL_COMPRESSION_LZMA;
        case COMPRESSION_LZW:           return SAIL_COMPRESSION_LZW;
        case COMPRESSION_NEXT:          return SAIL_COMPRESSION_NEXT;
        case COMPRESSION_NONE:          return SAIL_COMPRESSION_NONE;
        case COMPRESSION_OJPEG:         return SAIL_COMPRESSION_OJPEG;
        case COMPRESSION_PACKBITS:      return SAIL_COMPRESSION_PACKBITS;
        case COMPRESSION_PIXARFILM:     return SAIL_COMPRESSION_PIXAR_FILM;
        case COMPRESSION_PIXARLOG:      return SAIL_COMPRESSION_PIXAR_LOG;
        case COMPRESSION_SGILOG24:      return SAIL_COMPRESSION_RLE;
        case COMPRESSION_SGILOG:        return SAIL_COMPRESSION_SGI_LOG;
        case COMPRESSION_T43:           return SAIL_COMPRESSION_SGI_LOG24;
        case COMPRESSION_T85:           return SAIL_COMPRESSION_T43;
        case COMPRESSION_THUNDERSCAN:   return SAIL_COMPRESSION_THUNDERSCAN;
#if defined HAVE_TIFF_41 && !defined SAIL_WIN32
        case COMPRESSION_WEBP:          return SAIL_COMPRESSION_WEBP;
#endif
#ifdef HAVE_TIFF_41
        case COMPRESSION_ZSTD:          return SAIL_COMPRESSION_ZSTD;
#endif

        default: {
            return SAIL_COMPRESSION_UNKNOWN;
        }
    }
}

sail_status_t tiff_private_sail_compression_to_compression(enum SailCompression compression, int *tiff_compression) {

    SAIL_CHECK_PTR(tiff_compression);

    switch (compression) {
        case SAIL_COMPRESSION_ADOBE_DEFLATE: *tiff_compression = COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
        case SAIL_COMPRESSION_DEFLATE:       *tiff_compression = COMPRESSION_DEFLATE;       return SAIL_OK;
        case SAIL_COMPRESSION_JPEG:          *tiff_compression = COMPRESSION_JPEG;          return SAIL_OK;
        case SAIL_COMPRESSION_LZW:           *tiff_compression = COMPRESSION_LZW;           return SAIL_OK;
        case SAIL_COMPRESSION_NONE:          *tiff_compression = COMPRESSION_NONE;          return SAIL_OK;
        case SAIL_COMPRESSION_PACKBITS:      *tiff_compression = COMPRESSION_PACKBITS;      return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_LOG:     *tiff_compression = COMPRESSION_PIXARLOG;      return SAIL_OK;
#ifdef HAVE_TIFF_41
#if !defined SAIL_WIN32
        case SAIL_COMPRESSION_WEBP:          *tiff_compression = COMPRESSION_WEBP;          return SAIL_OK;
#endif
        case SAIL_COMPRESSION_ZSTD:          *tiff_compression = COMPRESSION_ZSTD;          return SAIL_OK;
#endif

        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
        }
    }
}

enum SailPixelFormat tiff_private_bpp_to_pixel_format(int bpp) {

    switch (bpp) {
        case 1:   return SAIL_PIXEL_FORMAT_BPP1;
        case 2:   return SAIL_PIXEL_FORMAT_BPP2;
        case 4:   return SAIL_PIXEL_FORMAT_BPP4;
        case 8:   return SAIL_PIXEL_FORMAT_BPP8;
        case 16:  return SAIL_PIXEL_FORMAT_BPP16;
        case 24:  return SAIL_PIXEL_FORMAT_BPP24;
        case 32:  return SAIL_PIXEL_FORMAT_BPP32;
        case 48:  return SAIL_PIXEL_FORMAT_BPP48;
        case 64:  return SAIL_PIXEL_FORMAT_BPP64;
        case 72:  return SAIL_PIXEL_FORMAT_BPP72;
        case 96:  return SAIL_PIXEL_FORMAT_BPP96;
        case 128: return SAIL_PIXEL_FORMAT_BPP128;

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

void tiff_private_zero_tiff_image(TIFFRGBAImage *img) {

    if (img == NULL) {
        return;
    }

    img->Map           = NULL;
    img->BWmap         = NULL;
    img->PALmap        = NULL;
    img->ycbcr         = NULL;
    img->cielab        = NULL;
    img->UaToAa        = NULL;
    img->Bitdepth16To8 = NULL;
    img->redcmap       = NULL;
    img->greencmap     = NULL;
    img->bluecmap      = NULL;
}

sail_status_t tiff_private_fetch_iccp(TIFF *tiff, struct sail_iccp **iccp) {

    unsigned char *data;
    unsigned data_length;

    if (TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &data_length, &data)) {
        SAIL_TRY(sail_alloc_iccp_from_data(iccp, data, data_length));
    }

    return SAIL_OK;
}

static sail_status_t fetch_single_meta_data(TIFF *tiff, int tag, enum SailMetaData key, struct sail_meta_data_node ***last_meta_data_node) {

    char *data;

    if (TIFFGetField(tiff, tag, &data)) {
        struct sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node_from_known_string(key, data, &meta_data_node));

        **last_meta_data_node = meta_data_node;
        *last_meta_data_node = &meta_data_node->next;
    }

    return SAIL_OK;
}

sail_status_t tiff_private_fetch_meta_data(TIFF *tiff, struct sail_meta_data_node ***last_meta_data_node) {

    SAIL_CHECK_META_DATA_NODE_PTR(last_meta_data_node);

    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_DOCUMENTNAME,     SAIL_META_DATA_DOCUMENT,    last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_IMAGEDESCRIPTION, SAIL_META_DATA_DESCRIPTION, last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_MAKE,             SAIL_META_DATA_MAKE,        last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_MODEL,            SAIL_META_DATA_MODEL,       last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_SOFTWARE,         SAIL_META_DATA_SOFTWARE,    last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_ARTIST,           SAIL_META_DATA_ARTIST,      last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_COPYRIGHT,        SAIL_META_DATA_COPYRIGHT,   last_meta_data_node));

    return SAIL_OK;
}

sail_status_t tiff_private_write_meta_data(TIFF *tiff, const struct sail_meta_data_node *meta_data_node) {

    SAIL_CHECK_PTR(tiff);

    while (meta_data_node != NULL) {
        const char *meta_data_str = NULL;

        if (meta_data_node->value_type == SAIL_META_DATA_TYPE_STRING) {
            int tiff_tag = -1;

            switch (meta_data_node->key) {
                case SAIL_META_DATA_DOCUMENT:    tiff_tag = TIFFTAG_DOCUMENTNAME;     break;
                case SAIL_META_DATA_DESCRIPTION: tiff_tag = TIFFTAG_IMAGEDESCRIPTION; break;
                case SAIL_META_DATA_MAKE:        tiff_tag = TIFFTAG_MAKE;             break;
                case SAIL_META_DATA_MODEL:       tiff_tag = TIFFTAG_MODEL;            break;
                case SAIL_META_DATA_SOFTWARE:    tiff_tag = TIFFTAG_SOFTWARE;         break;
                case SAIL_META_DATA_ARTIST:      tiff_tag = TIFFTAG_ARTIST;           break;
                case SAIL_META_DATA_COPYRIGHT:   tiff_tag = TIFFTAG_COPYRIGHT;        break;

                case SAIL_META_DATA_UNKNOWN: {
                    SAIL_LOG_WARNING("TIFF: Ignoring unsupported unknown meta data keys like '%s'", meta_data_node->key_unknown);
                    break;
                }

                default: {
                    SAIL_TRY_OR_SUPPRESS(sail_meta_data_to_string(meta_data_node->key, &meta_data_str));
                    SAIL_LOG_WARNING("TIFF: Ignoring unsupported meta data key '%s'", meta_data_str);
                }
            }

            if (tiff_tag < 0) {
                continue;
            }

            TIFFSetField(tiff, tiff_tag, meta_data_node->value_string);
        } else {
            SAIL_TRY_OR_SUPPRESS(sail_meta_data_to_string(meta_data_node->key, &meta_data_str));
            SAIL_LOG_WARNING("TIFF: Ignoring unsupported binary key '%s'", meta_data_str);
        }

        meta_data_node = meta_data_node->next;
    }

    return SAIL_OK;
}

sail_status_t tiff_private_supported_write_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_AUTO:
        case SAIL_PIXEL_FORMAT_SOURCE: {
            return SAIL_OK;
        }

        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }
}

sail_status_t tiff_private_fetch_resolution(TIFF *tiff, struct sail_resolution **resolution) {

    SAIL_CHECK_RESOLUTION_PTR(resolution);

    int unit = RESUNIT_NONE;
    float x = 0, y = 0;

    TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &unit);
    TIFFGetField(tiff, TIFFTAG_XRESOLUTION,    &x);
    TIFFGetField(tiff, TIFFTAG_YRESOLUTION,    &y);

    /* Resolution information is not valid. */
    if (x == 0 && y == 0) {
        return SAIL_OK;
    }

    SAIL_TRY(sail_alloc_resolution(resolution));

    switch (unit) {
        case RESUNIT_INCH: {
            (*resolution)->unit = SAIL_RESOLUTION_UNIT_INCH;
            break;
        }
        case RESUNIT_CENTIMETER: {
            (*resolution)->unit = SAIL_RESOLUTION_UNIT_CENTIMETER;
            break;
        }
    }

    (*resolution)->x = x;
    (*resolution)->y = y;

    return SAIL_OK;
}

sail_status_t tiff_private_write_resolution(TIFF *tiff, const struct sail_resolution *resolution) {

    /* Not an error. */
    if (resolution == NULL) {
        return SAIL_OK;
    }

    uint16_t unit;

    switch (resolution->unit) {
        case SAIL_RESOLUTION_UNIT_INCH: {
            unit = RESUNIT_INCH;
            break;
        }
        case SAIL_RESOLUTION_UNIT_CENTIMETER: {
            unit = RESUNIT_CENTIMETER;
            break;
        }
        default: {
            unit = RESUNIT_NONE;
            break;
        }
    }

    TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, unit);
    TIFFSetField(tiff, TIFFTAG_XRESOLUTION,    resolution->x);
    TIFFSetField(tiff, TIFFTAG_YRESOLUTION,    resolution->y);

    return SAIL_OK;
}
