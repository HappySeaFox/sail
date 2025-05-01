/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#ifndef SAIL_COMMON_CPP_H
#define SAIL_COMMON_CPP_H

#include <sail-common/common.h>

#include <sail-c++/flags.h>

/*
 * Common data types used across SAIL, adapted for C++.
 * See the common data types documentation in sail-common.
 */

namespace sail
{

/* Pixel format */
enum class PixelFormat {

    /*
     * Unknown or unsupported pixel format that cannot be parsed by SAIL.
     */
    Unknown = SAIL_PIXEL_FORMAT_UNKNOWN,

    /*
     * Formats with unknown pixel representation/model.
     */
    BPP1   = SAIL_PIXEL_FORMAT_BPP1,
    BPP2   = SAIL_PIXEL_FORMAT_BPP2,
    BPP4   = SAIL_PIXEL_FORMAT_BPP4,
    BPP8   = SAIL_PIXEL_FORMAT_BPP8,
    BPP16  = SAIL_PIXEL_FORMAT_BPP16,
    BPP24  = SAIL_PIXEL_FORMAT_BPP24,
    BPP32  = SAIL_PIXEL_FORMAT_BPP32,
    BPP48  = SAIL_PIXEL_FORMAT_BPP48,
    BPP64  = SAIL_PIXEL_FORMAT_BPP64,
    BPP72  = SAIL_PIXEL_FORMAT_BPP72,
    BPP96  = SAIL_PIXEL_FORMAT_BPP96,
    BPP128 = SAIL_PIXEL_FORMAT_BPP128,

    /*
     * Indexed formats with palette.
     */
    BPP1_Indexed  = SAIL_PIXEL_FORMAT_BPP1_INDEXED,
    BPP2_Indexed  = SAIL_PIXEL_FORMAT_BPP2_INDEXED,
    BPP4_Indexed  = SAIL_PIXEL_FORMAT_BPP4_INDEXED,
    BPP8_Indexed  = SAIL_PIXEL_FORMAT_BPP8_INDEXED,
    BPP16_Indexed = SAIL_PIXEL_FORMAT_BPP16_INDEXED,

    /*
     * Grayscale formats.
     */
    BPP1_Grayscale  = SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE,
    BPP2_Grayscale  = SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE,
    BPP4_Grayscale  = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE,
    BPP8_Grayscale  = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE,
    BPP16_Grayscale = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE,

    BPP4_Grayscale_Alpha  = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA,
    BPP8_Grayscale_Alpha  = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA,
    BPP16_Grayscale_Alpha = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA,
    BPP32_Grayscale_Alpha = SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA,

    /*
     * Packed formats.
     */
    BPP16_RGB555 = SAIL_PIXEL_FORMAT_BPP16_RGB555,
    BPP16_BGR555 = SAIL_PIXEL_FORMAT_BPP16_BGR555,
    BPP16_RGB565 = SAIL_PIXEL_FORMAT_BPP16_RGB565,
    BPP16_BGR565 = SAIL_PIXEL_FORMAT_BPP16_BGR565,

    /*
     * RGB formats.
     */
    BPP24_RGB = SAIL_PIXEL_FORMAT_BPP24_RGB,
    BPP24_BGR = SAIL_PIXEL_FORMAT_BPP24_BGR,

    BPP48_RGB = SAIL_PIXEL_FORMAT_BPP48_RGB,
    BPP48_BGR = SAIL_PIXEL_FORMAT_BPP48_BGR,

    /*
     * RGBA/X formats. X = unused color channel.
     */
    BPP16_RGBX = SAIL_PIXEL_FORMAT_BPP16_RGBX,
    BPP16_BGRX = SAIL_PIXEL_FORMAT_BPP16_BGRX,
    BPP16_XRGB = SAIL_PIXEL_FORMAT_BPP16_XRGB,
    BPP16_XBGR = SAIL_PIXEL_FORMAT_BPP16_XBGR,
    BPP16_RGBA = SAIL_PIXEL_FORMAT_BPP16_RGBA,
    BPP16_BGRA = SAIL_PIXEL_FORMAT_BPP16_BGRA,
    BPP16_ARGB = SAIL_PIXEL_FORMAT_BPP16_ARGB,
    BPP16_ABGR = SAIL_PIXEL_FORMAT_BPP16_ABGR,

    BPP32_RGBX = SAIL_PIXEL_FORMAT_BPP32_RGBX,
    BPP32_BGRX = SAIL_PIXEL_FORMAT_BPP32_BGRX,
    BPP32_XRGB = SAIL_PIXEL_FORMAT_BPP32_XRGB,
    BPP32_XBGR = SAIL_PIXEL_FORMAT_BPP32_XBGR,
    BPP32_RGBA = SAIL_PIXEL_FORMAT_BPP32_RGBA,
    BPP32_BGRA = SAIL_PIXEL_FORMAT_BPP32_BGRA,
    BPP32_ARGB = SAIL_PIXEL_FORMAT_BPP32_ARGB,
    BPP32_ABGR = SAIL_PIXEL_FORMAT_BPP32_ABGR,

    BPP64_RGBX = SAIL_PIXEL_FORMAT_BPP64_RGBX,
    BPP64_BGRX = SAIL_PIXEL_FORMAT_BPP64_BGRX,
    BPP64_XRGB = SAIL_PIXEL_FORMAT_BPP64_XRGB,
    BPP64_XBGR = SAIL_PIXEL_FORMAT_BPP64_XBGR,
    BPP64_RGBA = SAIL_PIXEL_FORMAT_BPP64_RGBA,
    BPP64_BGRA = SAIL_PIXEL_FORMAT_BPP64_BGRA,
    BPP64_ARGB = SAIL_PIXEL_FORMAT_BPP64_ARGB,
    BPP64_ABGR = SAIL_PIXEL_FORMAT_BPP64_ABGR,

    /*
     * CMYK formats.
     */
    BPP32_CMYK = SAIL_PIXEL_FORMAT_BPP32_CMYK,
    BPP64_CMYK = SAIL_PIXEL_FORMAT_BPP64_CMYK,

    BPP40_CMYKA = SAIL_PIXEL_FORMAT_BPP40_CMYKA,
    BPP80_CMYKA = SAIL_PIXEL_FORMAT_BPP80_CMYKA,

    /*
     * YCbCr formats.
     */
    BPP24_YCbCr = SAIL_PIXEL_FORMAT_BPP24_YCBCR,

    /*
     * YCCK formats.
     */
    BPP32_YCCK = SAIL_PIXEL_FORMAT_BPP32_YCCK,

    /*
     * CIE LAB formats.
     */
    BPP24_CIE_LAB = SAIL_PIXEL_FORMAT_BPP24_CIE_LAB, /* 8/8/8   */
    BPP40_CIE_LAB = SAIL_PIXEL_FORMAT_BPP40_CIE_LAB, /* 8/16/16 */

    /*
     * CIE LUV formats.
     */
    BPP24_CIE_LUV = SAIL_PIXEL_FORMAT_BPP24_CIE_LUV, /* 8/8/8   */
    BPP40_CIE_LUV = SAIL_PIXEL_FORMAT_BPP40_CIE_LUV, /* 8/16/16 */

    /*
     * YUV formats.
     */
    BPP24_YUV = SAIL_PIXEL_FORMAT_BPP24_YUV, /* 8-bit  */
    BPP30_YUV = SAIL_PIXEL_FORMAT_BPP30_YUV, /* 10-bit */
    BPP36_YUV = SAIL_PIXEL_FORMAT_BPP36_YUV, /* 12-bit */
    BPP48_YUV = SAIL_PIXEL_FORMAT_BPP48_YUV, /* 16-bit */

    BPP32_YUVA = SAIL_PIXEL_FORMAT_BPP32_YUVA,
    BPP40_YUVA = SAIL_PIXEL_FORMAT_BPP40_YUVA,
    BPP48_YUVA = SAIL_PIXEL_FORMAT_BPP48_YUVA,
    BPP64_YUVA = SAIL_PIXEL_FORMAT_BPP64_YUVA,
};

/* Chroma subsampling. See https://en.wikipedia.org/wiki/Chroma_subsampling */
enum class ChromaSubsampling {

    Unknown   = SAIL_CHROMA_SUBSAMPLING_UNKNOWN,

    Format311 = SAIL_CHROMA_SUBSAMPLING_311,
    Format400 = SAIL_CHROMA_SUBSAMPLING_400,
    Format410 = SAIL_CHROMA_SUBSAMPLING_410,
    Format411 = SAIL_CHROMA_SUBSAMPLING_411,
    Format420 = SAIL_CHROMA_SUBSAMPLING_420,
    Format421 = SAIL_CHROMA_SUBSAMPLING_421,
    Format422 = SAIL_CHROMA_SUBSAMPLING_422,
    Format444 = SAIL_CHROMA_SUBSAMPLING_444,
};

/* Orientation. */
enum class Orientation {

    Normal                         = SAIL_ORIENTATION_NORMAL,
    Rotated90                      = SAIL_ORIENTATION_ROTATED_90,
    Rotated180                     = SAIL_ORIENTATION_ROTATED_180,
    Rotated270                     = SAIL_ORIENTATION_ROTATED_270,
    MirroredHorizontally           = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY,
    MirroredVertically             = SAIL_ORIENTATION_MIRRORED_VERTICALLY,
    MirroredHorizontallyRotated90  = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_90,
    MirroredHorizontallyRotated270 = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_270,
};

/* Pixels compression types. */
enum class Compression {

    /* Unknown compression. */
    Unknown = SAIL_COMPRESSION_UNKNOWN,

    /* No compression at all. */
    None = SAIL_COMPRESSION_NONE,

    Adobe_Deflate = SAIL_COMPRESSION_ADOBE_DEFLATE, /* Deflate compression, as recognized by Adobe. */
    AV1           = SAIL_COMPRESSION_AV1,           /* AOMedia Video 1. */
    CCITT_Fax3    = SAIL_COMPRESSION_CCITT_FAX3,    /* CCITT Group 3 fax encoding. */
    CCITT_Fax4    = SAIL_COMPRESSION_CCITT_FAX4,    /* CCITT Group 4 fax encoding. */
    CCITT_RLE     = SAIL_COMPRESSION_CCITT_RLE,     /* CCITT modified Huffman RLE. */
    CCITT_RLEW    = SAIL_COMPRESSION_CCITT_RLEW,    /* #1 w/ word alignment. */
    CCITT_T4      = SAIL_COMPRESSION_CCITT_T4,      /* CCITT T.4 (TIFF 6 name). */
    CCITT_T6      = SAIL_COMPRESSION_CCITT_T6,      /* CCITT T.6 (TIFF 6 name). */
    DCS           = SAIL_COMPRESSION_DCS,           /* Kodak DCS encoding. */
    Deflate       = SAIL_COMPRESSION_DEFLATE,       /* Deflate compression. */
    IT8_BL        = SAIL_COMPRESSION_IT8_BL,        /* IT8 Binary line art. */
    IT8_CTPad     = SAIL_COMPRESSION_IT8_CTPAD,     /* IT8 CT w/padding. */
    IT8_LW        = SAIL_COMPRESSION_IT8_LW,        /* IT8 Linework RLE. */
    IT8_MP        = SAIL_COMPRESSION_IT8_MP,        /* IT8 Monochrome picture. */
    JBIG          = SAIL_COMPRESSION_JBIG,          /* ISO JBIG. */
    JPEG          = SAIL_COMPRESSION_JPEG,          /* JPEG DCT compression. */
    JPEG_2000     = SAIL_COMPRESSION_JPEG_2000,     /* Leadtools JPEG 2000. */
    JPEG_XL       = SAIL_COMPRESSION_JPEG_XL,       /* JPEG XL. */
    JPEG_XR       = SAIL_COMPRESSION_JPEG_XR,       /* JPEG XR. */
    LERC          = SAIL_COMPRESSION_LERC,          /* ESRI Lerc codec. */
    LZMA          = SAIL_COMPRESSION_LZMA,          /* LZMA2. */
    LZW           = SAIL_COMPRESSION_LZW,           /* Lempel-Ziv  & Welch. */
    Next          = SAIL_COMPRESSION_NEXT,          /* NeXT 2-bit RLE. */
    OJPEG         = SAIL_COMPRESSION_OJPEG,         /* !6.0 JPEG. */
    PackBits      = SAIL_COMPRESSION_PACKBITS,      /* Macintosh RLE. */
    Pixar_Film    = SAIL_COMPRESSION_PIXAR_FILM,    /* Pixar companded 10bit LZW. */
    Pixar_Log     = SAIL_COMPRESSION_PIXAR_LOG,     /* Pixar companded 11bit ZIP. */
    QOI           = SAIL_COMPRESSION_QOI,           /* Quite OK Image. */
    RLE           = SAIL_COMPRESSION_RLE,           /* RLE compression. */
    SGI_LOG       = SAIL_COMPRESSION_SGI_LOG,       /* SGI Log Luminance RLE. */
    SGI_LOG24     = SAIL_COMPRESSION_SGI_LOG24,     /* SGI Log 24-bit packed. */
    T43           = SAIL_COMPRESSION_T43,           /* !TIFF/FX T.43 colour by layered JBIG compression. */
    T85           = SAIL_COMPRESSION_T85,           /* !TIFF/FX T.85 JBIG compression. */
    ThunderScan   = SAIL_COMPRESSION_THUNDERSCAN,   /* ThunderScan RLE. */
    WebP          = SAIL_COMPRESSION_WEBP,          /* WEBP. */
    ZIP           = SAIL_COMPRESSION_ZIP,           /* ZIP. */
    ZStd          = SAIL_COMPRESSION_ZSTD,          /* ZSTD. */
};

/* Meta data. */
enum class MetaData {

    /* Unknown meta data type. */
    Unknown = SAIL_META_DATA_UNKNOWN,

    Artist          = SAIL_META_DATA_ARTIST,
    Author          = SAIL_META_DATA_AUTHOR,
    Comment         = SAIL_META_DATA_COMMENT,
    Computer        = SAIL_META_DATA_COMPUTER,
    Copyright       = SAIL_META_DATA_COPYRIGHT,
    CreationTime    = SAIL_META_DATA_CREATION_TIME,
    Description     = SAIL_META_DATA_DESCRIPTION,
    Disclaimer      = SAIL_META_DATA_DISCLAIMER,
    Document        = SAIL_META_DATA_DOCUMENT,
    Exif            = SAIL_META_DATA_EXIF, /* This one may or may not start with "Exif\0\0". */
    Id              = SAIL_META_DATA_ID,
    IPTC            = SAIL_META_DATA_IPTC,
    Job             = SAIL_META_DATA_JOB,
    JUMBF           = SAIL_META_DATA_JUMBF,
    Label           = SAIL_META_DATA_LABEL,
    Make            = SAIL_META_DATA_MAKE,
    Model           = SAIL_META_DATA_MODEL,
    Name            = SAIL_META_DATA_NAME,
    Printer         = SAIL_META_DATA_PRINTER,
    Software        = SAIL_META_DATA_SOFTWARE,
    SoftwareVersion = SAIL_META_DATA_SOFTWARE_VERSION,
    Source          = SAIL_META_DATA_SOURCE,
    TimeConsumed    = SAIL_META_DATA_TIME_CONSUMED,
    Title           = SAIL_META_DATA_TITLE,
    URL             = SAIL_META_DATA_URL,
    Warning         = SAIL_META_DATA_WARNING,
    XMP             = SAIL_META_DATA_XMP,
};

/* Resolution units. */
enum class ResolutionUnit {

    Unknown    = SAIL_RESOLUTION_UNIT_UNKNOWN,
    Micrometer = SAIL_RESOLUTION_UNIT_MICROMETER,
    Centimeter = SAIL_RESOLUTION_UNIT_CENTIMETER,
    Meter      = SAIL_RESOLUTION_UNIT_METER,
    Inch       = SAIL_RESOLUTION_UNIT_INCH,
};

/* Codec features. */
enum class CodecFeature {

    /* Unknown codec feature used to indicate an error in parsing functions. */
    Unknown = SAIL_CODEC_FEATURE_UNKNOWN,

    /* Can load or save static images. */
    Static = SAIL_CODEC_FEATURE_STATIC,

    /* Can load or save animated images. */
    Animated = SAIL_CODEC_FEATURE_ANIMATED,

    /* Can load or save multi-paged (but not animated) images. */
    MultiPaged = SAIL_CODEC_FEATURE_MULTI_PAGED,

    /* Can load or save image meta data like JPEG comments or EXIF. */
    MetaData = SAIL_CODEC_FEATURE_META_DATA,

    /* Can load or save interlaced images. */
    Interlaced = SAIL_CODEC_FEATURE_INTERLACED,

    /* Can load or save embedded ICC profiles. */
    ICCP = SAIL_CODEC_FEATURE_ICCP,

    /* Can preserve the source image information. */
    SourceImage = SAIL_CODEC_FEATURE_SOURCE_IMAGE,
};

/* Load or save options. */
enum class CodecOption {

    /*
     * Instruction to load or save image meta data like JPEG comments or EXIF.
     * Loading special properties in source images is also affected by this option.
     */
    MetaData = SAIL_OPTION_META_DATA,

    /* Instruction to save interlaced images. Specifying this option for loading operations has no effect. */
    Interlaced = SAIL_OPTION_INTERLACED,

    /* Instruction to load or save embedded ICC profile. */
    ICCP = SAIL_OPTION_ICCP,

    /*
     * Instruction to preserve the source image information in loading operations.
     * Specifying this option for saving operations has no effect.
     */
    SourceImage = SAIL_OPTION_SOURCE_IMAGE,
};

}

#endif
