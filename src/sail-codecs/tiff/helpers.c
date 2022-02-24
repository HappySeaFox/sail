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

enum SailCompression tiff_private_compression_to_sail_compression(int compression) {

    switch (compression) {
#ifdef SAIL_HAVE_TIFF_ADOBE_DEFLATE
        case COMPRESSION_ADOBE_DEFLATE: return SAIL_COMPRESSION_ADOBE_DEFLATE;
#endif
#ifdef SAIL_HAVE_TIFF_CCITTRLE
        case COMPRESSION_CCITTRLE:      return SAIL_COMPRESSION_CCITT_RLE;
#endif
#ifdef SAIL_HAVE_TIFF_CCITTRLEW
        case COMPRESSION_CCITTRLEW:     return SAIL_COMPRESSION_CCITT_RLEW;
#endif
#ifdef SAIL_HAVE_TIFF_CCITT_T4
        case COMPRESSION_CCITT_T4:      return SAIL_COMPRESSION_CCITT_T4;
#endif
#ifdef SAIL_HAVE_TIFF_CCITT_T6
        case COMPRESSION_CCITT_T6:      return SAIL_COMPRESSION_CCITT_T6;
#endif
#ifdef SAIL_HAVE_TIFF_DCS
        case COMPRESSION_DCS:           return SAIL_COMPRESSION_DCS;
#endif
#ifdef SAIL_HAVE_TIFF_DEFLATE
        case COMPRESSION_DEFLATE:       return SAIL_COMPRESSION_DEFLATE;
#endif
#ifdef SAIL_HAVE_TIFF_IT8BL
        case COMPRESSION_IT8BL:         return SAIL_COMPRESSION_IT8_BL;
#endif
#ifdef SAIL_HAVE_TIFF_IT8CTPAD
        case COMPRESSION_IT8CTPAD:      return SAIL_COMPRESSION_IT8_CTPAD;
#endif
#ifdef SAIL_HAVE_TIFF_IT8LW
        case COMPRESSION_IT8LW:         return SAIL_COMPRESSION_IT8_LW;
#endif
#ifdef SAIL_HAVE_TIFF_IT8MP
        case COMPRESSION_IT8MP:         return SAIL_COMPRESSION_IT8_MP;
#endif
#ifdef SAIL_HAVE_TIFF_JBIG
        case COMPRESSION_JBIG:          return SAIL_COMPRESSION_JBIG;
#endif
#ifdef SAIL_HAVE_TIFF_JPEG
        case COMPRESSION_JPEG:          return SAIL_COMPRESSION_JPEG;
#endif
#ifdef SAIL_HAVE_TIFF_JP2000
        case COMPRESSION_JP2000:        return SAIL_COMPRESSION_JPEG_2000;
#endif
#ifdef SAIL_HAVE_TIFF_JXL
        case COMPRESSION_JXL:           return SAIL_COMPRESSION_JPEG_XL;
#endif
#ifdef SAIL_HAVE_TIFF_LERC
        case COMPRESSION_LERC:          return SAIL_COMPRESSION_LERC;
#endif
#ifdef SAIL_HAVE_TIFF_LZMA
        case COMPRESSION_LZMA:          return SAIL_COMPRESSION_LZMA;
#endif
#ifdef SAIL_HAVE_TIFF_LZW
        case COMPRESSION_LZW:           return SAIL_COMPRESSION_LZW;
#endif
#ifdef SAIL_HAVE_TIFF_NEXT
        case COMPRESSION_NEXT:          return SAIL_COMPRESSION_NEXT;
#endif
#ifdef SAIL_HAVE_TIFF_NONE
        case COMPRESSION_NONE:          return SAIL_COMPRESSION_NONE;
#endif
#ifdef SAIL_HAVE_TIFF_OJPEG
        case COMPRESSION_OJPEG:         return SAIL_COMPRESSION_OJPEG;
#endif
#ifdef SAIL_HAVE_TIFF_PACKBITS
        case COMPRESSION_PACKBITS:      return SAIL_COMPRESSION_PACKBITS;
#endif
#ifdef SAIL_HAVE_TIFF_PIXARFILM
        case COMPRESSION_PIXARFILM:     return SAIL_COMPRESSION_PIXAR_FILM;
#endif
#ifdef SAIL_HAVE_TIFF_PIXARLOG
        case COMPRESSION_PIXARLOG:      return SAIL_COMPRESSION_PIXAR_LOG;
#endif
#ifdef SAIL_HAVE_TIFF_SGILOG24
        case COMPRESSION_SGILOG24:      return SAIL_COMPRESSION_RLE;
#endif
#ifdef SAIL_HAVE_TIFF_SGILOG
        case COMPRESSION_SGILOG:        return SAIL_COMPRESSION_SGI_LOG;
#endif
#ifdef SAIL_HAVE_TIFF_T43
        case COMPRESSION_T43:           return SAIL_COMPRESSION_SGI_LOG24;
#endif
#ifdef SAIL_HAVE_TIFF_T85
        case COMPRESSION_T85:           return SAIL_COMPRESSION_T43;
#endif
#ifdef SAIL_HAVE_TIFF_THUNDERSCAN
        case COMPRESSION_THUNDERSCAN:   return SAIL_COMPRESSION_THUNDERSCAN;
#endif
#ifdef SAIL_HAVE_TIFF_WEBP
        case COMPRESSION_WEBP:          return SAIL_COMPRESSION_WEBP;
#endif
#ifdef SAIL_HAVE_TIFF_ZSTD
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
#ifdef SAIL_HAVE_TIFF_WRITE_ADOBE_DEFLATE
        case SAIL_COMPRESSION_ADOBE_DEFLATE: *tiff_compression = COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_CCITTRLE
        case SAIL_COMPRESSION_CCITT_RLE:     *tiff_compression = COMPRESSION_CCITTRLE;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_CCITTRLEW
        case SAIL_COMPRESSION_CCITT_RLEW:    *tiff_compression = COMPRESSION_CCITTRLEW;    return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_CCITT_T4
        case SAIL_COMPRESSION_CCITT_T4:      *tiff_compression = COMPRESSION_CCITT_T4;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_CCITT_T6
        case SAIL_COMPRESSION_CCITT_T6:      *tiff_compression = COMPRESSION_CCITT_T6;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_DCS
        case SAIL_COMPRESSION_DCS:           *tiff_compression = COMPRESSION_DCS;          return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_DEFLATE
        case SAIL_COMPRESSION_DEFLATE:       *tiff_compression = COMPRESSION_DEFLATE;      return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_IT8BL
        case SAIL_COMPRESSION_IT8_BL:        *tiff_compression = COMPRESSION_IT8BL;        return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_IT8CTPAD
        case SAIL_COMPRESSION_IT8_CTPAD:     *tiff_compression = COMPRESSION_IT8CTPAD;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_IT8LW
        case SAIL_COMPRESSION_IT8_LW:        *tiff_compression = COMPRESSION_IT8LW;        return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_IT8MP
        case SAIL_COMPRESSION_IT8_MP:        *tiff_compression = COMPRESSION_IT8MP;        return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_JBIG
        case SAIL_COMPRESSION_JBIG:          *tiff_compression = COMPRESSION_JBIG;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_JPEG
        case SAIL_COMPRESSION_JPEG:          *tiff_compression = COMPRESSION_JPEG;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_JP2000
        case SAIL_COMPRESSION_JPEG_2000:     *tiff_compression = COMPRESSION_JP2000;       return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_JXL
        case SAIL_COMPRESSION_JPEG_XL:       *tiff_compression = COMPRESSION_JXL;          return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_LERC
        case SAIL_COMPRESSION_LERC:          *tiff_compression = COMPRESSION_LERC;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_LZMA
        case SAIL_COMPRESSION_LZMA:          *tiff_compression = COMPRESSION_LZMA;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_LZW
        case SAIL_COMPRESSION_LZW:           *tiff_compression = COMPRESSION_LZW;          return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_NEXT
        case SAIL_COMPRESSION_NEXT:          *tiff_compression = COMPRESSION_NEXT;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_NONE
        case SAIL_COMPRESSION_NONE:          *tiff_compression = COMPRESSION_NONE;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_OJPEG
        case SAIL_COMPRESSION_OJPEG:         *tiff_compression = COMPRESSION_OJPEG;        return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_PACKBITS
        case SAIL_COMPRESSION_PACKBITS:      *tiff_compression = COMPRESSION_PACKBITS;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_PIXARFILM
        case SAIL_COMPRESSION_PIXAR_FILM:    *tiff_compression = COMPRESSION_PIXARFILM;    return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_PIXARLOG
        case SAIL_COMPRESSION_PIXAR_LOG:     *tiff_compression = COMPRESSION_PIXARLOG;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_SGILOG24
        case SAIL_COMPRESSION_RLE:           *tiff_compression = COMPRESSION_SGILOG24;     return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_SGILOG
        case SAIL_COMPRESSION_SGI_LOG:       *tiff_compression = COMPRESSION_SGILOG;       return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_T43
        case SAIL_COMPRESSION_SGI_LOG24:     *tiff_compression = COMPRESSION_T43;          return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_T85
        case SAIL_COMPRESSION_T43:           *tiff_compression = COMPRESSION_T85;          return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_THUNDERSCAN
        case SAIL_COMPRESSION_THUNDERSCAN:   *tiff_compression = COMPRESSION_THUNDERSCAN;  return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_WEBP
        case SAIL_COMPRESSION_WEBP:          *tiff_compression = COMPRESSION_WEBP;         return SAIL_OK;
#endif
#ifdef SAIL_HAVE_TIFF_WRITE_ZSTD
        case SAIL_COMPRESSION_ZSTD:          *tiff_compression = COMPRESSION_ZSTD;         return SAIL_OK;
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
        SAIL_TRY(sail_alloc_iccp_from_data(data, data_length, iccp));
    }

    return SAIL_OK;
}

static sail_status_t fetch_single_meta_data(TIFF *tiff, int tag, enum SailMetaData key, struct sail_meta_data_node ***last_meta_data_node) {

    char *data;

    if (TIFFGetField(tiff, tag, &data)) {
        struct sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(key, &meta_data_node->meta_data),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
        SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, data),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

        **last_meta_data_node = meta_data_node;
        *last_meta_data_node = &meta_data_node->next;
    }

    return SAIL_OK;
}

sail_status_t tiff_private_fetch_meta_data(TIFF *tiff, struct sail_meta_data_node ***last_meta_data_node) {

    SAIL_CHECK_PTR(last_meta_data_node);

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

    for (; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
        const struct sail_meta_data *meta_data = meta_data_node->meta_data;

        if (meta_data->value->type == SAIL_VARIANT_TYPE_STRING) {
            int tiff_tag = -1;

            switch (meta_data->key) {
                case SAIL_META_DATA_DOCUMENT:    tiff_tag = TIFFTAG_DOCUMENTNAME;     break;
                case SAIL_META_DATA_DESCRIPTION: tiff_tag = TIFFTAG_IMAGEDESCRIPTION; break;
                case SAIL_META_DATA_MAKE:        tiff_tag = TIFFTAG_MAKE;             break;
                case SAIL_META_DATA_MODEL:       tiff_tag = TIFFTAG_MODEL;            break;
                case SAIL_META_DATA_SOFTWARE:    tiff_tag = TIFFTAG_SOFTWARE;         break;
                case SAIL_META_DATA_ARTIST:      tiff_tag = TIFFTAG_ARTIST;           break;
                case SAIL_META_DATA_COPYRIGHT:   tiff_tag = TIFFTAG_COPYRIGHT;        break;

                case SAIL_META_DATA_UNKNOWN: {
                    SAIL_LOG_WARNING("TIFF: Ignoring unsupported unknown meta data keys like '%s'", meta_data->key_unknown);
                    break;
                }

                default: {
                    SAIL_LOG_WARNING("TIFF: Ignoring unsupported meta data key '%s'", sail_meta_data_to_string(meta_data->key));
                }
            }

            if (tiff_tag < 0) {
                continue;
            }

            TIFFSetField(tiff, tiff_tag, sail_variant_to_string(meta_data->value));
        } else {
            SAIL_LOG_WARNING("TIFF: Ignoring unsupported binary key '%s'", sail_meta_data_to_string(meta_data->key));
        }
    }

    return SAIL_OK;
}

sail_status_t tiff_private_fetch_resolution(TIFF *tiff, struct sail_resolution **resolution) {

    SAIL_CHECK_PTR(resolution);

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
