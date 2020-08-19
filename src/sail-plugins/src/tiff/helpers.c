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

enum SailCompressionType tiff_compression_to_sail_compression(int compression) {

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

sail_status_t sail_compression_to_tiff_compression(enum SailCompression compression, int *tiff_compression) {

    SAIL_CHECK_PTR(tiff_compression);

    switch (compression) {
        case SAIL_COMPRESSION_ADOBE_DEFLATE: *tiff_compression = COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX3:    *tiff_compression = COMPRESSION_CCITTFAX3;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX4:    *tiff_compression = COMPRESSION_CCITTFAX4;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLE:     *tiff_compression = COMPRESSION_CCITTRLE;      return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLEW:    *tiff_compression = COMPRESSION_CCITTRLEW;     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T4:      *tiff_compression = COMPRESSION_CCITT_T4;      return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T6:      *tiff_compression = COMPRESSION_CCITT_T6;      return SAIL_OK;
        case SAIL_COMPRESSION_DCS:           *tiff_compression = COMPRESSION_DCS;           return SAIL_OK;
        case SAIL_COMPRESSION_DEFLATE:       *tiff_compression = COMPRESSION_DEFLATE;       return SAIL_OK;
        case SAIL_COMPRESSION_IT8_BL:        *tiff_compression = COMPRESSION_IT8BL;         return SAIL_OK;
        case SAIL_COMPRESSION_IT8_CTPAD:     *tiff_compression = COMPRESSION_IT8CTPAD;      return SAIL_OK;
        case SAIL_COMPRESSION_IT8_LW:        *tiff_compression = COMPRESSION_IT8LW;         return SAIL_OK;
        case SAIL_COMPRESSION_IT8_MP:        *tiff_compression = COMPRESSION_IT8MP;         return SAIL_OK;
        case SAIL_COMPRESSION_JBIG:          *tiff_compression = COMPRESSION_JBIG;          return SAIL_OK;
        case SAIL_COMPRESSION_JPEG:          *tiff_compression = COMPRESSION_JPEG;          return SAIL_OK;
        case SAIL_COMPRESSION_JPEG2000:      *tiff_compression = COMPRESSION_JP2000;        return SAIL_OK;
#ifdef HAVE_TIFF_41
        case SAIL_COMPRESSION_LERC:          *tiff_compression = COMPRESSION_LERC;          return SAIL_OK;
#endif
        case SAIL_COMPRESSION_LZMA:          *tiff_compression = COMPRESSION_LZMA;          return SAIL_OK;
        case SAIL_COMPRESSION_LZW:           *tiff_compression = COMPRESSION_LZW;           return SAIL_OK;
        case SAIL_COMPRESSION_NEXT:          *tiff_compression = COMPRESSION_NEXT;          return SAIL_OK;
        case SAIL_COMPRESSION_NONE:          *tiff_compression = COMPRESSION_NONE;          return SAIL_OK;
        case SAIL_COMPRESSION_OJPEG:         *tiff_compression = COMPRESSION_OJPEG;         return SAIL_OK;
        case SAIL_COMPRESSION_PACKBITS:      *tiff_compression = COMPRESSION_PACKBITS;      return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_FILM:    *tiff_compression = COMPRESSION_PIXARFILM;     return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_LOG:     *tiff_compression = COMPRESSION_PIXARLOG;      return SAIL_OK;
        case SAIL_COMPRESSION_RLE:           *tiff_compression = COMPRESSION_SGILOG24;      return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG:       *tiff_compression = COMPRESSION_SGILOG;        return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG24:     *tiff_compression = COMPRESSION_T43;           return SAIL_OK;
        case SAIL_COMPRESSION_T43:           *tiff_compression = COMPRESSION_T85;           return SAIL_OK;
        case SAIL_COMPRESSION_THUNDERSCAN:   *tiff_compression = COMPRESSION_THUNDERSCAN;   return SAIL_OK;
#if defined HAVE_TIFF_41 && !defined SAIL_WIN32
        case SAIL_COMPRESSION_WEBP:          *tiff_compression = COMPRESSION_WEBP;          return SAIL_OK;
#endif
#ifdef HAVE_TIFF_41
        case SAIL_COMPRESSION_ZSTD:          *tiff_compression = COMPRESSION_ZSTD;          return SAIL_OK;
#endif

        default: {
            return SAIL_ERROR_UNSUPPORTED_COMPRESSION_TYPE;
        }
    }
}

enum SailPixelFormat bpp_to_pixel_format(int bpp) {

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

void zero_tiff_image(TIFFRGBAImage *img) {

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

sail_status_t fetch_iccp(TIFF *tiff, struct sail_iccp **iccp) {

    unsigned char *data;
    unsigned data_length;

    if (TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &data_length, &data)) {
        SAIL_TRY(sail_alloc_iccp_with_data(iccp, data, data_length));
    }

    return SAIL_OK;
}

static sail_status_t fetch_single_meta_info(TIFF *tiff, int tag, const char *key, struct sail_meta_entry_node ***last_meta_entry_node) {

    char *data;

    if (TIFFGetField(tiff, tag, &data)) {
        struct sail_meta_entry_node *meta_entry_node;

        SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup(key, &meta_entry_node->key),
                            /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup(data, &meta_entry_node->value),
                            /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));

        **last_meta_entry_node = meta_entry_node;
        *last_meta_entry_node = &meta_entry_node->next;
    }

    return SAIL_OK;
}

sail_status_t fetch_meta_info(TIFF *tiff, struct sail_meta_entry_node ***last_meta_entry_node) {

    SAIL_CHECK_META_ENTRY_NODE_PTR(last_meta_entry_node);

    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_DOCUMENTNAME,     "Document Name", last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_IMAGEDESCRIPTION, "Description",   last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_MAKE,             "Make",          last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_MODEL,            "Model",         last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_SOFTWARE,         "Software",      last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_ARTIST,           "Artist",        last_meta_entry_node));
    SAIL_TRY(fetch_single_meta_info(tiff, TIFFTAG_COPYRIGHT,        "Copyright",     last_meta_entry_node));

    return SAIL_OK;
}

sail_status_t write_meta_info(TIFF *tiff, const struct sail_meta_entry_node *meta_entry_node) {

    SAIL_CHECK_PTR(tiff);

    while (meta_entry_node != NULL) {

        if (strcmp(meta_entry_node->key, "Document Name") == 0) {
            TIFFSetField(tiff, TIFFTAG_DOCUMENTNAME, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Description") == 0) {
            TIFFSetField(tiff, TIFFTAG_IMAGEDESCRIPTION, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Make") == 0) {
            TIFFSetField(tiff, TIFFTAG_MAKE, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Model") == 0) {
            TIFFSetField(tiff, TIFFTAG_MODEL, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Software") == 0) {
            TIFFSetField(tiff, TIFFTAG_SOFTWARE, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Artist") == 0) {
            TIFFSetField(tiff, TIFFTAG_ARTIST, meta_entry_node->value);
        } else if (strcmp(meta_entry_node->key, "Copyright") == 0) {
            TIFFSetField(tiff, TIFFTAG_COPYRIGHT, meta_entry_node->value);
        } else {
            SAIL_LOG_WARNING("TIFF: Ignoring unsupported meta entry key '%s'", meta_entry_node->key);
        }

        meta_entry_node = meta_entry_node->next;
    }

    return SAIL_OK;
}

sail_status_t supported_write_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_AUTO:
        case SAIL_PIXEL_FORMAT_SOURCE: {
            return SAIL_OK;
        }

        default: {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }
    }
}
