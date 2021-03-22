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

#ifndef SAIL_COMMON_H
#define SAIL_COMMON_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

/*
 * Common data structures and functions used across SAIL, both in libsail and in image codecs.
 */

/* Pixel format */
enum SailPixelFormat {

    /*
     * Unknown or unsupported pixel format that cannot be parsed by SAIL.
     */
    SAIL_PIXEL_FORMAT_UNKNOWN,

    /*
     * Formats with unknown pixel representation/model.
     */
    SAIL_PIXEL_FORMAT_BPP1,
    SAIL_PIXEL_FORMAT_BPP2,
    SAIL_PIXEL_FORMAT_BPP4,
    SAIL_PIXEL_FORMAT_BPP8,
    SAIL_PIXEL_FORMAT_BPP16,
    SAIL_PIXEL_FORMAT_BPP24,
    SAIL_PIXEL_FORMAT_BPP32,
    SAIL_PIXEL_FORMAT_BPP48,
    SAIL_PIXEL_FORMAT_BPP64,
    SAIL_PIXEL_FORMAT_BPP72,
    SAIL_PIXEL_FORMAT_BPP96,
    SAIL_PIXEL_FORMAT_BPP128,

    /*
     * Indexed formats with palette.
     */
    SAIL_PIXEL_FORMAT_BPP1_INDEXED,
    SAIL_PIXEL_FORMAT_BPP2_INDEXED,
    SAIL_PIXEL_FORMAT_BPP4_INDEXED,
    SAIL_PIXEL_FORMAT_BPP8_INDEXED,
    SAIL_PIXEL_FORMAT_BPP16_INDEXED,

    /*
     * Grayscale formats.
     */
    SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE,
    SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE,
    SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE,
    SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE,
    SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE,

    SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA,
    SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA,
    SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA,
    SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA,

    /*
     * Packed formats.
     */
    SAIL_PIXEL_FORMAT_BPP16_RGB555,
    SAIL_PIXEL_FORMAT_BPP16_BGR555,
    SAIL_PIXEL_FORMAT_BPP16_RGB565,
    SAIL_PIXEL_FORMAT_BPP16_BGR565,

    /*
     * RGB formats.
     */
    SAIL_PIXEL_FORMAT_BPP24_RGB,
    SAIL_PIXEL_FORMAT_BPP24_BGR,

    SAIL_PIXEL_FORMAT_BPP48_RGB,
    SAIL_PIXEL_FORMAT_BPP48_BGR,

    /*
     * RGBA/X formats. X = unused color channel with a random value.
     */
    SAIL_PIXEL_FORMAT_BPP32_RGBX,
    SAIL_PIXEL_FORMAT_BPP32_BGRX,
    SAIL_PIXEL_FORMAT_BPP32_XRGB,
    SAIL_PIXEL_FORMAT_BPP32_XBGR,
    SAIL_PIXEL_FORMAT_BPP32_RGBA,
    SAIL_PIXEL_FORMAT_BPP32_BGRA,
    SAIL_PIXEL_FORMAT_BPP32_ARGB,
    SAIL_PIXEL_FORMAT_BPP32_ABGR,

    SAIL_PIXEL_FORMAT_BPP64_RGBX,
    SAIL_PIXEL_FORMAT_BPP64_BGRX,
    SAIL_PIXEL_FORMAT_BPP64_XRGB,
    SAIL_PIXEL_FORMAT_BPP64_XBGR,
    SAIL_PIXEL_FORMAT_BPP64_RGBA,
    SAIL_PIXEL_FORMAT_BPP64_BGRA,
    SAIL_PIXEL_FORMAT_BPP64_ARGB,
    SAIL_PIXEL_FORMAT_BPP64_ABGR,

    /*
     * CMYK formats.
     */
    SAIL_PIXEL_FORMAT_BPP32_CMYK,
    SAIL_PIXEL_FORMAT_BPP64_CMYK,

    /*
     * YCbCr formats.
     */
    SAIL_PIXEL_FORMAT_BPP24_YCBCR,

    /*
     * YCCK formats.
     */
    SAIL_PIXEL_FORMAT_BPP32_YCCK,

    /*
     * LAB formats.
     */
    SAIL_PIXEL_FORMAT_BPP24_CIE_LAB,
    SAIL_PIXEL_FORMAT_BPP48_CIE_LAB,
};

/* Image properties. */
enum SailImageProperty {

    /* Image needs flipping vertically. */
    SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY = 1 << 0,

    /*
     * Image is interlaced. Only sail_image.source_properties can have this property.
     * Reading operations never output interlaced images, that's why sail_image.properties
     * never has it.
     */
    SAIL_IMAGE_PROPERTY_INTERLACED         = 1 << 1,
};

/* Pixels compression types. */
enum SailCompression {

    /* Compression is unsupported. */
    SAIL_COMPRESSION_UNSUPPORTED,

    /* Unknown compression. */
    SAIL_COMPRESSION_UNKNOWN,

    /* No compression at all. */
    SAIL_COMPRESSION_NONE,

    SAIL_COMPRESSION_ADOBE_DEFLATE, /* Deflate compression, as recognized by Adobe. */
    SAIL_COMPRESSION_CCITT_FAX3,    /* CCITT Group 3 fax encoding. */
    SAIL_COMPRESSION_CCITT_FAX4,    /* CCITT Group 4 fax encoding. */
    SAIL_COMPRESSION_CCITT_RLE,     /* CCITT modified Huffman RLE. */
    SAIL_COMPRESSION_CCITT_RLEW,    /* #1 w/ word alignment. */
    SAIL_COMPRESSION_CCITT_T4,      /* CCITT T.4 (TIFF 6 name). */
    SAIL_COMPRESSION_CCITT_T6,      /* CCITT T.6 (TIFF 6 name). */
    SAIL_COMPRESSION_DCS,           /* Kodak DCS encoding. */
    SAIL_COMPRESSION_DEFLATE,       /* Deflate compression. */
    SAIL_COMPRESSION_IT8_BL,        /* IT8 Binary line art. */
    SAIL_COMPRESSION_IT8_CTPAD,     /* IT8 CT w/padding. */
    SAIL_COMPRESSION_IT8_LW,        /* IT8 Linework RLE. */
    SAIL_COMPRESSION_IT8_MP,        /* IT8 Monochrome picture. */
    SAIL_COMPRESSION_JBIG,          /* ISO JBIG. */
    SAIL_COMPRESSION_JPEG,          /* %JPEG DCT compression. */
    SAIL_COMPRESSION_JPEG2000,      /* Leadtools JPEG2000. */
    SAIL_COMPRESSION_LERC,          /* ESRI Lerc codec. */
    SAIL_COMPRESSION_LZMA,          /* LZMA2. */
    SAIL_COMPRESSION_LZW,           /* Lempel-Ziv  & Welch. */
    SAIL_COMPRESSION_NEXT,          /* NeXT 2-bit RLE. */
    SAIL_COMPRESSION_OJPEG,         /* !6.0 JPEG. */
    SAIL_COMPRESSION_PACKBITS,      /* Macintosh RLE. */
    SAIL_COMPRESSION_PIXAR_FILM,    /* Pixar companded 10bit LZW. */
    SAIL_COMPRESSION_PIXAR_LOG,     /* Pixar companded 11bit ZIP. */
    SAIL_COMPRESSION_RLE,           /* RLE compression. */
    SAIL_COMPRESSION_SGI_LOG,       /* SGI Log Luminance RLE. */
    SAIL_COMPRESSION_SGI_LOG24,     /* SGI Log 24-bit packed. */
    SAIL_COMPRESSION_T43,           /* !TIFF/FX T.43 colour by layered JBIG compression. */
    SAIL_COMPRESSION_T85,           /* !TIFF/FX T.85 JBIG compression. */
    SAIL_COMPRESSION_THUNDERSCAN,   /* ThunderScan RLE. */
    SAIL_COMPRESSION_WEBP,          /* WEBP. */
    SAIL_COMPRESSION_ZSTD,          /* ZSTD. */
};

/* Meta data. */
enum SailMetaData {

    /* Some unknown meta_data type. */
    SAIL_META_DATA_UNKNOWN,

    /*
     * Keys pointing to zero-terminated strings. Meta data nodes holding strings MUST be allocated with
     * sail_alloc_meta_data_node_from_string(). Failing to do so may lead to corrupted
     * meta data entries in the resulting file.
     */
    SAIL_META_DATA_ARTIST,
    SAIL_META_DATA_AUTHOR,
    SAIL_META_DATA_COMMENT,
    SAIL_META_DATA_COMPUTER,
    SAIL_META_DATA_COPYRIGHT,
    SAIL_META_DATA_CREATION_TIME,
    SAIL_META_DATA_DESCRIPTION,
    SAIL_META_DATA_DISCLAIMER,
    SAIL_META_DATA_DOCUMENT,
    SAIL_META_DATA_HEX_EXIF,
    SAIL_META_DATA_HEX_IPTC,
    SAIL_META_DATA_HEX_XMP,
    SAIL_META_DATA_LABEL,
    SAIL_META_DATA_MAKE,
    SAIL_META_DATA_MODEL,
    SAIL_META_DATA_NAME,
    SAIL_META_DATA_PRINTER,
    SAIL_META_DATA_SOFTWARE,
    SAIL_META_DATA_SOURCE,
    SAIL_META_DATA_TITLE,
    SAIL_META_DATA_URL,
    SAIL_META_DATA_WARNING,
    SAIL_META_DATA_XMP,

    /*
     * Keys pointing to binary data. Meta data nodes holding binary data MUST be allocated with
     * sail_alloc_meta_data_node_from_data(). Failing to do so may lead to corrupted
     * meta data entries in the resulting file.
     */
    SAIL_META_DATA_EXIF,
};

/* Meta data type. */
enum SailMetaDataType {

    /* Meta data string value like a JPEG comment. */
    SAIL_META_DATA_TYPE_STRING,
    /* Meta data binary value like a binary EXIF profile. */
    SAIL_META_DATA_TYPE_DATA,
};

/* Resolution units. */
enum SailResolutionUnit {

    SAIL_RESOLUTION_UNIT_UNKNOWN,
    SAIL_RESOLUTION_UNIT_MICROMETER,
    SAIL_RESOLUTION_UNIT_CENTIMETER,
    SAIL_RESOLUTION_UNIT_METER,
    SAIL_RESOLUTION_UNIT_INCH,
};

/* Codec features. */
enum SailCodecFeature {

    /* Ability to read or write static images. */
    SAIL_CODEC_FEATURE_STATIC      = 1 << 0,

    /* Ability to read or write animated images. */
    SAIL_CODEC_FEATURE_ANIMATED    = 1 << 1,

    /* Ability to read or write multi-frame (but not animated) images. */
    SAIL_CODEC_FEATURE_MULTI_FRAME = 1 << 2,

    /* Ability to read or write simple image meta data like JPEG comments. */
    SAIL_CODEC_FEATURE_META_DATA   = 1 << 3,

    /* Ability to read or write EXIF meta data. */
    SAIL_CODEC_FEATURE_EXIF        = 1 << 4,

    /* Ability to read or write interlaced images. */
    SAIL_CODEC_FEATURE_INTERLACED  = 1 << 5,

    /* Ability to read or write embedded ICC profiles. */
    SAIL_CODEC_FEATURE_ICCP        = 1 << 6,
};

/* Read or write options. */
enum SailIoOption {

    /* Instruction to read or write simple image meta data like JPEG comments. */
    SAIL_IO_OPTION_META_DATA  = 1 << 0,

    /* Instruction to read or write EXIF meta data. */
    SAIL_IO_OPTION_EXIF       = 1 << 1,

    /* Instruction to write interlaced images. Specifying this option for reading operations has no effect. */
    SAIL_IO_OPTION_INTERLACED = 1 << 2,

    /* Instruction to read or write embedded ICC profile. */
    SAIL_IO_OPTION_ICCP       = 1 << 3,
};

#endif
