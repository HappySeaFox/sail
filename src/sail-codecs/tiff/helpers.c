/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <string.h>

#include <tiff.h>
#include <tiffio.h>

#include <sail-common/sail-common.h>

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

sail_status_t tiff_private_sail_pixel_format_from_tiff(TIFF *tiff, enum SailPixelFormat *result) {

    SAIL_CHECK_PTR(tiff);
    SAIL_CHECK_PTR(result);

    uint16_t photometric = 0;
    uint16_t bits_per_sample = 0;
    uint16_t samples_per_pixel = 0;
    uint16_t extra_samples = 0;
    uint16_t *extra_samples_types = NULL;
    uint16_t planar_config = PLANARCONFIG_CONTIG;

    if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &photometric)) {
        SAIL_LOG_ERROR("TIFF: Failed to get photometric interpretation");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample)) {
        bits_per_sample = 1;
    }

    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel)) {
        samples_per_pixel = 1;
    }

    TIFFGetField(tiff, TIFFTAG_EXTRASAMPLES, &extra_samples, &extra_samples_types);
    TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planar_config);

    /* Only support contiguous (interleaved) data for now. */
    if (planar_config != PLANARCONFIG_CONTIG) {
        SAIL_LOG_ERROR("TIFF: Planar configuration %u is not supported", planar_config);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const unsigned total_bpp = bits_per_sample * samples_per_pixel;

    switch (photometric) {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK: {
            /* Grayscale or bilevel. */
            switch (samples_per_pixel) {
                case 1: {
                    /* Pure grayscale. */
                    switch (bits_per_sample) {
                        case 1:  *result = SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;  return SAIL_OK;
                        case 2:  *result = SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;  return SAIL_OK;
                        case 4:  *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;  return SAIL_OK;
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;  return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE; return SAIL_OK;
                    }
                    break;
                }
                case 2: {
                    /* Grayscale + alpha. */
                    switch (bits_per_sample) {
                        case 4:  *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;  return SAIL_OK;
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        case PHOTOMETRIC_PALETTE: {
            /* Indexed color. */
            if (samples_per_pixel == 1) {
                switch (bits_per_sample) {
                    case 1: *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED; return SAIL_OK;
                    case 2: *result = SAIL_PIXEL_FORMAT_BPP2_INDEXED; return SAIL_OK;
                    case 4: *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
                    case 8: *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED; return SAIL_OK;
                }
            }
            break;
        }
        case PHOTOMETRIC_RGB: {
            /* RGB or RGBA. */
            switch (samples_per_pixel) {
                case 3: {
                    /* RGB without alpha. */
                    switch (bits_per_sample) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP24_RGB; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP48_RGB; return SAIL_OK;
                    }
                    break;
                }
                case 4: {
                    /* RGBA with alpha. */
                    switch (bits_per_sample) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP32_RGBA; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP64_RGBA; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        case PHOTOMETRIC_SEPARATED: {
            /* CMYK for print. */
            switch (samples_per_pixel) {
                case 4: {
                    /* Pure CMYK without alpha. */
                    switch (bits_per_sample) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP32_CMYK; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP64_CMYK; return SAIL_OK;
                    }
                    break;
                }
                case 5: {
                    /* CMYK with alpha. */
                    switch (bits_per_sample) {
                        case 8:  *result = SAIL_PIXEL_FORMAT_BPP40_CMYKA; return SAIL_OK;
                        case 16: *result = SAIL_PIXEL_FORMAT_BPP80_CMYKA; return SAIL_OK;
                    }
                    break;
                }
            }
            break;
        }
        case PHOTOMETRIC_YCBCR: {
            /* YCbCr color space (used in JPEG compression). */
            if (samples_per_pixel == 3 && bits_per_sample == 8) {
                *result = SAIL_PIXEL_FORMAT_BPP24_YCBCR;
                return SAIL_OK;
            }
            break;
        }
        case PHOTOMETRIC_CIELAB: {
            /* CIELab color space for accurate color representation. */
            if (samples_per_pixel == 3) {
                switch (bits_per_sample) {
                    case 8:  *result = SAIL_PIXEL_FORMAT_BPP24_CIE_LAB; return SAIL_OK;
                    /* Note: TIFF spec allows mixed bit depths for CIELab (L=8, a=16, b=16). */
                }
            }
            break;
        }
    }

    /* Unknown or unsupported combination. */
    SAIL_LOG_ERROR("TIFF: Unsupported pixel format: photometric=%u, bits_per_sample=%u, samples_per_pixel=%u, total_bpp=%u",
                   photometric, bits_per_sample, samples_per_pixel, total_bpp);

    *result = tiff_private_bpp_to_pixel_format(total_bpp);

    if (*result == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    return SAIL_OK;
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
    unsigned data_size;

    if (TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &data_size, &data)) {
        SAIL_TRY(sail_alloc_iccp_from_data(data, data_size, iccp));
    }

    return SAIL_OK;
}

static sail_status_t fetch_single_meta_data(TIFF *tiff, int tag, enum SailMetaData key, struct sail_meta_data_node ***last_meta_data_node) {

    char *data;

    if (TIFFGetField(tiff, tag, &data)) {
        struct sail_meta_data_node *meta_data_node;
        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(key, &meta_data_node->meta_data),
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

    /* Basic TIFF tags. */
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_DOCUMENTNAME,     SAIL_META_DATA_DOCUMENT,    last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_IMAGEDESCRIPTION, SAIL_META_DATA_DESCRIPTION, last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_MAKE,             SAIL_META_DATA_MAKE,        last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_MODEL,            SAIL_META_DATA_MODEL,       last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_SOFTWARE,         SAIL_META_DATA_SOFTWARE,    last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_ARTIST,           SAIL_META_DATA_ARTIST,      last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_COPYRIGHT,        SAIL_META_DATA_COPYRIGHT,   last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_DATETIME,         SAIL_META_DATA_CREATION_TIME, last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_HOSTCOMPUTER,     SAIL_META_DATA_COMPUTER,    last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_PAGENAME,         SAIL_META_DATA_NAME,        last_meta_data_node));
    SAIL_TRY(fetch_single_meta_data(tiff, TIFFTAG_TARGETPRINTER,    SAIL_META_DATA_PRINTER,     last_meta_data_node));

    return SAIL_OK;
}

sail_status_t tiff_private_write_meta_data(TIFF *tiff, const struct sail_meta_data_node *meta_data_node) {

    SAIL_CHECK_PTR(tiff);

    for (; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
        const struct sail_meta_data *meta_data = meta_data_node->meta_data;

        if (meta_data->value->type == SAIL_VARIANT_TYPE_STRING) {
            int tiff_tag = -1;

            switch (meta_data->key) {
                case SAIL_META_DATA_DOCUMENT:      tiff_tag = TIFFTAG_DOCUMENTNAME;     break;
                case SAIL_META_DATA_DESCRIPTION:   tiff_tag = TIFFTAG_IMAGEDESCRIPTION; break;
                case SAIL_META_DATA_MAKE:          tiff_tag = TIFFTAG_MAKE;             break;
                case SAIL_META_DATA_MODEL:         tiff_tag = TIFFTAG_MODEL;            break;
                case SAIL_META_DATA_SOFTWARE:      tiff_tag = TIFFTAG_SOFTWARE;         break;
                case SAIL_META_DATA_ARTIST:        tiff_tag = TIFFTAG_ARTIST;           break;
                case SAIL_META_DATA_COPYRIGHT:     tiff_tag = TIFFTAG_COPYRIGHT;        break;
                case SAIL_META_DATA_CREATION_TIME: tiff_tag = TIFFTAG_DATETIME;         break;
                case SAIL_META_DATA_COMPUTER:      tiff_tag = TIFFTAG_HOSTCOMPUTER;     break;
                case SAIL_META_DATA_NAME:          tiff_tag = TIFFTAG_PAGENAME;         break;
                case SAIL_META_DATA_PRINTER:       tiff_tag = TIFFTAG_TARGETPRINTER;    break;

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
            /* Binary metadata will be handled separately (EXIF, XMP, etc). */
            SAIL_LOG_TRACE("TIFF: Binary meta data key '%s' will be processed separately", sail_meta_data_to_string(meta_data->key));
        }
    }

    return SAIL_OK;
}

sail_status_t tiff_private_fetch_xmp(TIFF *tiff, struct sail_meta_data_node ***last_meta_data_node) {

    SAIL_CHECK_PTR(tiff);
    SAIL_CHECK_PTR(last_meta_data_node);

    /* Check if XMP packet exists. */
    void *xmp_data = NULL;
    uint32_t xmp_size = 0;

    if (TIFFGetField(tiff, TIFFTAG_XMLPACKET, &xmp_size, &xmp_data)) {
        if (xmp_data != NULL && xmp_size > 0) {
            /* Allocate metadata node for XMP. */
            struct sail_meta_data_node *meta_data_node;
            SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_XMP, &meta_data_node->meta_data),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

            /* Copy XMP data. */
            void *xmp_copy = NULL;
            SAIL_TRY_OR_CLEANUP(sail_malloc(xmp_size, &xmp_copy),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            memcpy(xmp_copy, xmp_data, xmp_size);

            SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value, xmp_copy, xmp_size),
                                /* cleanup */ sail_free(xmp_copy); sail_destroy_meta_data_node(meta_data_node));

            **last_meta_data_node = meta_data_node;
            *last_meta_data_node = &meta_data_node->next;

            SAIL_LOG_TRACE("TIFF: Loaded XMP metadata (%u bytes)", xmp_size);
        }
    }

    return SAIL_OK;
}

sail_status_t tiff_private_write_xmp(TIFF *tiff, const struct sail_meta_data_node *meta_data_node) {

    SAIL_CHECK_PTR(tiff);

    /* Look for XMP metadata in the list. */
    for (; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
        const struct sail_meta_data *meta_data = meta_data_node->meta_data;

        if (meta_data->key == SAIL_META_DATA_XMP && meta_data->value->type == SAIL_VARIANT_TYPE_DATA) {
            /* We have XMP data to write. */
            const void *xmp_data = sail_variant_to_data(meta_data->value);
            const size_t xmp_size = meta_data->value->size;

            if (xmp_data != NULL && xmp_size > 0) {
                TIFFSetField(tiff, TIFFTAG_XMLPACKET, (uint32_t)xmp_size, xmp_data);
                SAIL_LOG_TRACE("TIFF: Saved XMP metadata (%zu bytes)", xmp_size);
            }
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

bool tiff_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    TIFF *tiff = user_data;

    if (strcmp(key, "tiff-predictor") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_STRING) {
            const char *str_value = sail_variant_to_string(value);
            uint16_t predictor = PREDICTOR_NONE;

            if (strcmp(str_value, "none") == 0) {
                predictor = PREDICTOR_NONE;
                SAIL_LOG_TRACE("TIFF: Setting predictor to NONE");
            } else if (strcmp(str_value, "horizontal") == 0) {
                predictor = PREDICTOR_HORIZONTAL;
                SAIL_LOG_TRACE("TIFF: Setting predictor to HORIZONTAL");
            } else if (strcmp(str_value, "floating-point") == 0) {
                predictor = PREDICTOR_FLOATINGPOINT;
                SAIL_LOG_TRACE("TIFF: Setting predictor to FLOATING-POINT");
            }

            TIFFSetField(tiff, TIFFTAG_PREDICTOR, predictor);
        } else {
            SAIL_LOG_ERROR("TIFF: 'tiff-predictor' must be a string");
        }
    } else if (strcmp(key, "tiff-jpeg-quality") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            int quality = (value->type == SAIL_VARIANT_TYPE_INT)
                          ? sail_variant_to_int(value)
                          : (int)sail_variant_to_unsigned_int(value);

            if (quality >= 1 && quality <= 100) {
                TIFFSetField(tiff, TIFFTAG_JPEGQUALITY, quality);
                SAIL_LOG_TRACE("TIFF: Setting JPEG quality to %d", quality);
            } else {
                SAIL_LOG_WARNING("TIFF: JPEG quality must be 1-100, got %d", quality);
            }
        } else {
            SAIL_LOG_ERROR("TIFF: 'tiff-jpeg-quality' must be an integer");
        }
    } else if (strcmp(key, "tiff-zip-quality") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            int quality = (value->type == SAIL_VARIANT_TYPE_INT)
                          ? sail_variant_to_int(value)
                          : (int)sail_variant_to_unsigned_int(value);

            if (quality >= 1 && quality <= 9) {
                TIFFSetField(tiff, TIFFTAG_ZIPQUALITY, quality);
                SAIL_LOG_TRACE("TIFF: Setting ZIP/DEFLATE quality to %d", quality);
            } else {
                SAIL_LOG_WARNING("TIFF: ZIP quality must be 1-9, got %d", quality);
            }
        } else {
            SAIL_LOG_ERROR("TIFF: 'tiff-zip-quality' must be an integer");
        }
    }

    return true;
}
