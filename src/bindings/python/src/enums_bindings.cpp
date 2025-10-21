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

#include <pybind11/pybind11.h>
#include <sail-c++/sail-c++.h>
#include <sail-common/common.h>

namespace py = pybind11;

void init_enums(py::module_& m)
{
    // ============================================================================
    // SailPixelFormat
    // ============================================================================

    py::enum_<SailPixelFormat>(m, "PixelFormat", py::arithmetic(), "Pixel format enumeration")
        // Unknown
        .value("UNKNOWN", SAIL_PIXEL_FORMAT_UNKNOWN)
        // Unknown representation
        .value("BPP1", SAIL_PIXEL_FORMAT_BPP1)
        .value("BPP2", SAIL_PIXEL_FORMAT_BPP2)
        .value("BPP4", SAIL_PIXEL_FORMAT_BPP4)
        .value("BPP8", SAIL_PIXEL_FORMAT_BPP8)
        .value("BPP16", SAIL_PIXEL_FORMAT_BPP16)
        .value("BPP24", SAIL_PIXEL_FORMAT_BPP24)
        .value("BPP32", SAIL_PIXEL_FORMAT_BPP32)
        .value("BPP48", SAIL_PIXEL_FORMAT_BPP48)
        .value("BPP64", SAIL_PIXEL_FORMAT_BPP64)
        .value("BPP72", SAIL_PIXEL_FORMAT_BPP72)
        .value("BPP96", SAIL_PIXEL_FORMAT_BPP96)
        .value("BPP128", SAIL_PIXEL_FORMAT_BPP128)
        // Indexed
        .value("BPP1_INDEXED", SAIL_PIXEL_FORMAT_BPP1_INDEXED)
        .value("BPP2_INDEXED", SAIL_PIXEL_FORMAT_BPP2_INDEXED)
        .value("BPP4_INDEXED", SAIL_PIXEL_FORMAT_BPP4_INDEXED)
        .value("BPP8_INDEXED", SAIL_PIXEL_FORMAT_BPP8_INDEXED)
        .value("BPP16_INDEXED", SAIL_PIXEL_FORMAT_BPP16_INDEXED)
        // Grayscale
        .value("BPP1_GRAYSCALE", SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE)
        .value("BPP2_GRAYSCALE", SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE)
        .value("BPP4_GRAYSCALE", SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE)
        .value("BPP8_GRAYSCALE", SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE)
        .value("BPP16_GRAYSCALE", SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE)
        .value("BPP4_GRAYSCALE_ALPHA", SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA)
        .value("BPP8_GRAYSCALE_ALPHA", SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA)
        .value("BPP16_GRAYSCALE_ALPHA", SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA)
        .value("BPP32_GRAYSCALE_ALPHA", SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA)
        // Packed
        .value("BPP16_RGB555", SAIL_PIXEL_FORMAT_BPP16_RGB555)
        .value("BPP16_BGR555", SAIL_PIXEL_FORMAT_BPP16_BGR555)
        .value("BPP16_RGB565", SAIL_PIXEL_FORMAT_BPP16_RGB565)
        .value("BPP16_BGR565", SAIL_PIXEL_FORMAT_BPP16_BGR565)
        // RGB
        .value("BPP24_RGB", SAIL_PIXEL_FORMAT_BPP24_RGB)
        .value("BPP24_BGR", SAIL_PIXEL_FORMAT_BPP24_BGR)
        .value("BPP48_RGB", SAIL_PIXEL_FORMAT_BPP48_RGB)
        .value("BPP48_BGR", SAIL_PIXEL_FORMAT_BPP48_BGR)
        // RGBA/X
        .value("BPP16_RGBX", SAIL_PIXEL_FORMAT_BPP16_RGBX)
        .value("BPP16_BGRX", SAIL_PIXEL_FORMAT_BPP16_BGRX)
        .value("BPP16_XRGB", SAIL_PIXEL_FORMAT_BPP16_XRGB)
        .value("BPP16_XBGR", SAIL_PIXEL_FORMAT_BPP16_XBGR)
        .value("BPP16_RGBA", SAIL_PIXEL_FORMAT_BPP16_RGBA)
        .value("BPP16_BGRA", SAIL_PIXEL_FORMAT_BPP16_BGRA)
        .value("BPP16_ARGB", SAIL_PIXEL_FORMAT_BPP16_ARGB)
        .value("BPP16_ABGR", SAIL_PIXEL_FORMAT_BPP16_ABGR)
        .value("BPP32_RGBX", SAIL_PIXEL_FORMAT_BPP32_RGBX)
        .value("BPP32_BGRX", SAIL_PIXEL_FORMAT_BPP32_BGRX)
        .value("BPP32_XRGB", SAIL_PIXEL_FORMAT_BPP32_XRGB)
        .value("BPP32_XBGR", SAIL_PIXEL_FORMAT_BPP32_XBGR)
        .value("BPP32_RGBA", SAIL_PIXEL_FORMAT_BPP32_RGBA)
        .value("BPP32_BGRA", SAIL_PIXEL_FORMAT_BPP32_BGRA)
        .value("BPP32_ARGB", SAIL_PIXEL_FORMAT_BPP32_ARGB)
        .value("BPP32_ABGR", SAIL_PIXEL_FORMAT_BPP32_ABGR)
        .value("BPP64_RGBX", SAIL_PIXEL_FORMAT_BPP64_RGBX)
        .value("BPP64_BGRX", SAIL_PIXEL_FORMAT_BPP64_BGRX)
        .value("BPP64_XRGB", SAIL_PIXEL_FORMAT_BPP64_XRGB)
        .value("BPP64_XBGR", SAIL_PIXEL_FORMAT_BPP64_XBGR)
        .value("BPP64_RGBA", SAIL_PIXEL_FORMAT_BPP64_RGBA)
        .value("BPP64_BGRA", SAIL_PIXEL_FORMAT_BPP64_BGRA)
        .value("BPP64_ARGB", SAIL_PIXEL_FORMAT_BPP64_ARGB)
        .value("BPP64_ABGR", SAIL_PIXEL_FORMAT_BPP64_ABGR)
        // CMYK
        .value("BPP32_CMYK", SAIL_PIXEL_FORMAT_BPP32_CMYK)
        .value("BPP64_CMYK", SAIL_PIXEL_FORMAT_BPP64_CMYK)
        .value("BPP40_CMYKA", SAIL_PIXEL_FORMAT_BPP40_CMYKA)
        .value("BPP80_CMYKA", SAIL_PIXEL_FORMAT_BPP80_CMYKA)
        // YCbCr
        .value("BPP24_YCBCR", SAIL_PIXEL_FORMAT_BPP24_YCBCR)
        // YCCK
        .value("BPP32_YCCK", SAIL_PIXEL_FORMAT_BPP32_YCCK)
        // CIE LAB
        .value("BPP24_CIE_LAB", SAIL_PIXEL_FORMAT_BPP24_CIE_LAB)
        .value("BPP40_CIE_LAB", SAIL_PIXEL_FORMAT_BPP40_CIE_LAB)
        .value("BPP32_CIE_LABA", SAIL_PIXEL_FORMAT_BPP32_CIE_LABA)
        .value("BPP64_CIE_LABA", SAIL_PIXEL_FORMAT_BPP64_CIE_LABA)
        // CIE LUV
        .value("BPP24_CIE_LUV", SAIL_PIXEL_FORMAT_BPP24_CIE_LUV)
        .value("BPP40_CIE_LUV", SAIL_PIXEL_FORMAT_BPP40_CIE_LUV)
        // CIE XYZ
        .value("BPP24_CIE_XYZ", SAIL_PIXEL_FORMAT_BPP24_CIE_XYZ)
        .value("BPP48_CIE_XYZ", SAIL_PIXEL_FORMAT_BPP48_CIE_XYZ)
        .value("BPP32_CIE_XYZA", SAIL_PIXEL_FORMAT_BPP32_CIE_XYZA)
        .value("BPP64_CIE_XYZA", SAIL_PIXEL_FORMAT_BPP64_CIE_XYZA)
        // YUV
        .value("BPP24_YUV", SAIL_PIXEL_FORMAT_BPP24_YUV)
        .value("BPP30_YUV", SAIL_PIXEL_FORMAT_BPP30_YUV)
        .value("BPP36_YUV", SAIL_PIXEL_FORMAT_BPP36_YUV)
        .value("BPP48_YUV", SAIL_PIXEL_FORMAT_BPP48_YUV)
        .value("BPP32_YUVA", SAIL_PIXEL_FORMAT_BPP32_YUVA)
        .value("BPP40_YUVA", SAIL_PIXEL_FORMAT_BPP40_YUVA)
        .value("BPP48_YUVA", SAIL_PIXEL_FORMAT_BPP48_YUVA)
        .value("BPP64_YUVA", SAIL_PIXEL_FORMAT_BPP64_YUVA)
        .value("BPP32_AYUV", SAIL_PIXEL_FORMAT_BPP32_AYUV)
        .value("BPP64_AYUV", SAIL_PIXEL_FORMAT_BPP64_AYUV)
        // HSV/HSL
        .value("BPP24_HSV", SAIL_PIXEL_FORMAT_BPP24_HSV)
        .value("BPP24_HSL", SAIL_PIXEL_FORMAT_BPP24_HSL)
        .value("BPP48_HSV", SAIL_PIXEL_FORMAT_BPP48_HSV)
        .value("BPP48_HSL", SAIL_PIXEL_FORMAT_BPP48_HSL)
        // 10-bit
        .value("BPP30_RGB", SAIL_PIXEL_FORMAT_BPP30_RGB)
        .value("BPP30_BGR", SAIL_PIXEL_FORMAT_BPP30_BGR)
        .value("BPP32_RGBA_1010102", SAIL_PIXEL_FORMAT_BPP32_RGBA_1010102)
        .value("BPP32_BGRA_1010102", SAIL_PIXEL_FORMAT_BPP32_BGRA_1010102)
        // Float/Half
        .value("BPP16_GRAYSCALE_HALF", SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF)
        .value("BPP32_GRAYSCALE_FLOAT", SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT)
        .value("BPP48_RGB_HALF", SAIL_PIXEL_FORMAT_BPP48_RGB_HALF)
        .value("BPP64_RGBA_HALF", SAIL_PIXEL_FORMAT_BPP64_RGBA_HALF)
        .value("BPP96_RGB_FLOAT", SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT)
        .value("BPP128_RGBA_FLOAT", SAIL_PIXEL_FORMAT_BPP128_RGBA_FLOAT)
        .export_values()
        .def_static("from_string", &sail::image::pixel_format_from_string, py::arg("string"),
                    "Parse PixelFormat from SAIL string (e.g., 'BPP24-RGB')");

    // ============================================================================
    // SailCompression
    // ============================================================================

    py::enum_<SailCompression>(m, "Compression", py::arithmetic(), "Compression type enumeration")
        .value("UNKNOWN", SAIL_COMPRESSION_UNKNOWN)
        .value("NONE", SAIL_COMPRESSION_NONE)
        .value("ADOBE_DEFLATE", SAIL_COMPRESSION_ADOBE_DEFLATE)
        .value("AV1", SAIL_COMPRESSION_AV1)
        .value("CCITT_FAX3", SAIL_COMPRESSION_CCITT_FAX3)
        .value("CCITT_FAX4", SAIL_COMPRESSION_CCITT_FAX4)
        .value("CCITT_RLE", SAIL_COMPRESSION_CCITT_RLE)
        .value("CCITT_RLEW", SAIL_COMPRESSION_CCITT_RLEW)
        .value("CCITT_T4", SAIL_COMPRESSION_CCITT_T4)
        .value("CCITT_T6", SAIL_COMPRESSION_CCITT_T6)
        .value("DCS", SAIL_COMPRESSION_DCS)
        .value("DEFLATE", SAIL_COMPRESSION_DEFLATE)
        .value("IT8_BL", SAIL_COMPRESSION_IT8_BL)
        .value("IT8_CTPAD", SAIL_COMPRESSION_IT8_CTPAD)
        .value("IT8_LW", SAIL_COMPRESSION_IT8_LW)
        .value("IT8_MP", SAIL_COMPRESSION_IT8_MP)
        .value("JBIG", SAIL_COMPRESSION_JBIG)
        .value("JPEG", SAIL_COMPRESSION_JPEG)
        .value("JPEG_2000", SAIL_COMPRESSION_JPEG_2000)
        .value("JPEG_XL", SAIL_COMPRESSION_JPEG_XL)
        .value("JPEG_XR", SAIL_COMPRESSION_JPEG_XR)
        .value("LERC", SAIL_COMPRESSION_LERC)
        .value("LZMA", SAIL_COMPRESSION_LZMA)
        .value("LZW", SAIL_COMPRESSION_LZW)
        .value("NEXT", SAIL_COMPRESSION_NEXT)
        .value("OJPEG", SAIL_COMPRESSION_OJPEG)
        .value("PACKBITS", SAIL_COMPRESSION_PACKBITS)
        .value("PIXAR_FILM", SAIL_COMPRESSION_PIXAR_FILM)
        .value("PIXAR_LOG", SAIL_COMPRESSION_PIXAR_LOG)
        .value("QOI", SAIL_COMPRESSION_QOI)
        .value("RLE", SAIL_COMPRESSION_RLE)
        .value("SGI_LOG", SAIL_COMPRESSION_SGI_LOG)
        .value("SGI_LOG24", SAIL_COMPRESSION_SGI_LOG24)
        .value("T43", SAIL_COMPRESSION_T43)
        .value("T85", SAIL_COMPRESSION_T85)
        .value("THUNDERSCAN", SAIL_COMPRESSION_THUNDERSCAN)
        .value("WEBP", SAIL_COMPRESSION_WEBP)
        .value("ZIP", SAIL_COMPRESSION_ZIP)
        .value("ZSTD", SAIL_COMPRESSION_ZSTD)
        // Since 1.0.0
        .value("ASTC", SAIL_COMPRESSION_ASTC)
        .value("ATC", SAIL_COMPRESSION_ATC)
        .value("B44", SAIL_COMPRESSION_B44)
        .value("B44A", SAIL_COMPRESSION_B44A)
        .value("BC4", SAIL_COMPRESSION_BC4)
        .value("BC5", SAIL_COMPRESSION_BC5)
        .value("BC6H", SAIL_COMPRESSION_BC6H)
        .value("BC7", SAIL_COMPRESSION_BC7)
        .value("BPG", SAIL_COMPRESSION_BPG)
        .value("BROTLI", SAIL_COMPRESSION_BROTLI)
        .value("DWAA", SAIL_COMPRESSION_DWAA)
        .value("DWAB", SAIL_COMPRESSION_DWAB)
        .value("DXT1", SAIL_COMPRESSION_DXT1)
        .value("DXT3", SAIL_COMPRESSION_DXT3)
        .value("DXT5", SAIL_COMPRESSION_DXT5)
        .value("EAC", SAIL_COMPRESSION_EAC)
        .value("ETC1", SAIL_COMPRESSION_ETC1)
        .value("ETC2", SAIL_COMPRESSION_ETC2)
        .value("HEVC", SAIL_COMPRESSION_HEVC)
        .value("JPEG_LS", SAIL_COMPRESSION_JPEG_LS)
        .value("LZ4", SAIL_COMPRESSION_LZ4)
        .value("PIZ", SAIL_COMPRESSION_PIZ)
        .value("PVRTC", SAIL_COMPRESSION_PVRTC)
        .value("PVRTC2", SAIL_COMPRESSION_PVRTC2)
        .value("PXR24", SAIL_COMPRESSION_PXR24)
        .value("SNAPPY", SAIL_COMPRESSION_SNAPPY)
        .value("VVC", SAIL_COMPRESSION_VVC)
        .export_values()
        .def_static("from_string", &sail::image::compression_from_string, py::arg("string"),
                    "Parse Compression from SAIL string (e.g., 'JPEG')");

    // ============================================================================
    // SailOrientation
    // ============================================================================

    py::enum_<SailOrientation>(m, "Orientation", py::arithmetic(), "Image orientation")
        .value("NORMAL", SAIL_ORIENTATION_NORMAL)
        .value("ROTATED_90", SAIL_ORIENTATION_ROTATED_90)
        .value("ROTATED_180", SAIL_ORIENTATION_ROTATED_180)
        .value("ROTATED_270", SAIL_ORIENTATION_ROTATED_270)
        .value("MIRRORED_HORIZONTALLY", SAIL_ORIENTATION_MIRRORED_HORIZONTALLY)
        .value("MIRRORED_VERTICALLY", SAIL_ORIENTATION_MIRRORED_VERTICALLY)
        .value("MIRRORED_HORIZONTALLY_ROTATED_90", SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_90)
        .value("MIRRORED_HORIZONTALLY_ROTATED_270", SAIL_ORIENTATION_MIRRORED_HORIZONTALLY_ROTATED_270)
        .export_values()
        .def_static("from_string", &sail::image::orientation_from_string, py::arg("string"),
                    "Parse Orientation from SAIL string (e.g., 'ROTATED-90')");

    // ============================================================================
    // SailChromaSubsampling
    // ============================================================================

    py::enum_<SailChromaSubsampling>(m, "ChromaSubsampling", py::arithmetic(), "Chroma subsampling")
        .value("UNKNOWN", SAIL_CHROMA_SUBSAMPLING_UNKNOWN)
        .value("C311", SAIL_CHROMA_SUBSAMPLING_311)
        .value("C400", SAIL_CHROMA_SUBSAMPLING_400)
        .value("C410", SAIL_CHROMA_SUBSAMPLING_410)
        .value("C411", SAIL_CHROMA_SUBSAMPLING_411)
        .value("C420", SAIL_CHROMA_SUBSAMPLING_420)
        .value("C421", SAIL_CHROMA_SUBSAMPLING_421)
        .value("C422", SAIL_CHROMA_SUBSAMPLING_422)
        .value("C444", SAIL_CHROMA_SUBSAMPLING_444)
        .export_values()
        .def_static("from_string", &sail::image::chroma_subsampling_from_string, py::arg("string"),
                    "Parse ChromaSubsampling from SAIL string (e.g., '420')");

    // ============================================================================
    // SailResolutionUnit
    // ============================================================================

    py::enum_<SailResolutionUnit>(m, "ResolutionUnit", py::arithmetic(), "Resolution units")
        .value("UNKNOWN", SAIL_RESOLUTION_UNIT_UNKNOWN)
        .value("MICROMETER", SAIL_RESOLUTION_UNIT_MICROMETER)
        .value("CENTIMETER", SAIL_RESOLUTION_UNIT_CENTIMETER)
        .value("METER", SAIL_RESOLUTION_UNIT_METER)
        .value("INCH", SAIL_RESOLUTION_UNIT_INCH)
        .export_values();

    // ============================================================================
    // SailMetaData
    // ============================================================================

    py::enum_<SailMetaData>(m, "MetaDataType", py::arithmetic(), "Metadata types")
        .value("UNKNOWN", SAIL_META_DATA_UNKNOWN)
        .value("ARTIST", SAIL_META_DATA_ARTIST)
        .value("AUTHOR", SAIL_META_DATA_AUTHOR)
        .value("COMMENT", SAIL_META_DATA_COMMENT)
        .value("COMPUTER", SAIL_META_DATA_COMPUTER)
        .value("COPYRIGHT", SAIL_META_DATA_COPYRIGHT)
        .value("CREATION_TIME", SAIL_META_DATA_CREATION_TIME)
        .value("DESCRIPTION", SAIL_META_DATA_DESCRIPTION)
        .value("DISCLAIMER", SAIL_META_DATA_DISCLAIMER)
        .value("DOCUMENT", SAIL_META_DATA_DOCUMENT)
        .value("EXIF", SAIL_META_DATA_EXIF)
        .value("ID", SAIL_META_DATA_ID)
        .value("IPTC", SAIL_META_DATA_IPTC)
        .value("JOB", SAIL_META_DATA_JOB)
        .value("JUMBF", SAIL_META_DATA_JUMBF)
        .value("LABEL", SAIL_META_DATA_LABEL)
        .value("MAKE", SAIL_META_DATA_MAKE)
        .value("MODEL", SAIL_META_DATA_MODEL)
        .value("NAME", SAIL_META_DATA_NAME)
        .value("PRINTER", SAIL_META_DATA_PRINTER)
        .value("SOFTWARE", SAIL_META_DATA_SOFTWARE)
        .value("SOFTWARE_VERSION", SAIL_META_DATA_SOFTWARE_VERSION)
        .value("SOURCE", SAIL_META_DATA_SOURCE)
        .value("TIME_CONSUMED", SAIL_META_DATA_TIME_CONSUMED)
        .value("TITLE", SAIL_META_DATA_TITLE)
        .value("URL", SAIL_META_DATA_URL)
        .value("WARNING", SAIL_META_DATA_WARNING)
        .value("XMP", SAIL_META_DATA_XMP)
        .export_values();

    // ============================================================================
    // SailCodecFeature
    // ============================================================================

    py::enum_<SailCodecFeature>(m, "CodecFeature", py::arithmetic(), "Codec feature flags")
        .value("UNKNOWN", SAIL_CODEC_FEATURE_UNKNOWN)
        .value("STATIC", SAIL_CODEC_FEATURE_STATIC)
        .value("ANIMATED", SAIL_CODEC_FEATURE_ANIMATED)
        .value("MULTI_PAGED", SAIL_CODEC_FEATURE_MULTI_PAGED)
        .value("META_DATA", SAIL_CODEC_FEATURE_META_DATA)
        .value("INTERLACED", SAIL_CODEC_FEATURE_INTERLACED)
        .value("ICCP", SAIL_CODEC_FEATURE_ICCP)
        .value("SOURCE_IMAGE", SAIL_CODEC_FEATURE_SOURCE_IMAGE)
        .export_values();

    // ============================================================================
    // SailStatus
    // ============================================================================

    py::enum_<sail_status_t>(m, "Status", py::arithmetic(), "SAIL status codes")
        // Success
        .value("OK", SAIL_OK)
        // Common errors
        .value("ERROR_NULL_PTR", SAIL_ERROR_NULL_PTR)
        .value("ERROR_MEMORY_ALLOCATION", SAIL_ERROR_MEMORY_ALLOCATION)
        .value("ERROR_OPEN_FILE", SAIL_ERROR_OPEN_FILE)
        .value("ERROR_READ_FILE", SAIL_ERROR_READ_FILE)
        .value("ERROR_SEEK_FILE", SAIL_ERROR_SEEK_FILE)
        .value("ERROR_CLOSE_FILE", SAIL_ERROR_CLOSE_FILE)
        .value("ERROR_LIST_DIR", SAIL_ERROR_LIST_DIR)
        .value("ERROR_PARSE_FILE", SAIL_ERROR_PARSE_FILE)
        .value("ERROR_INVALID_ARGUMENT", SAIL_ERROR_INVALID_ARGUMENT)
        .value("ERROR_READ_IO", SAIL_ERROR_READ_IO)
        .value("ERROR_WRITE_IO", SAIL_ERROR_WRITE_IO)
        .value("ERROR_FLUSH_IO", SAIL_ERROR_FLUSH_IO)
        .value("ERROR_SEEK_IO", SAIL_ERROR_SEEK_IO)
        .value("ERROR_TELL_IO", SAIL_ERROR_TELL_IO)
        .value("ERROR_CLOSE_IO", SAIL_ERROR_CLOSE_IO)
        .value("ERROR_EOF", SAIL_ERROR_EOF)
        .value("ERROR_NOT_IMPLEMENTED", SAIL_ERROR_NOT_IMPLEMENTED)
        .value("ERROR_UNSUPPORTED_SEEK_WHENCE", SAIL_ERROR_UNSUPPORTED_SEEK_WHENCE)
        .value("ERROR_EMPTY_STRING", SAIL_ERROR_EMPTY_STRING)
        .value("ERROR_INVALID_VARIANT", SAIL_ERROR_INVALID_VARIANT)
        // Encoding/decoding
        .value("ERROR_INVALID_IO", SAIL_ERROR_INVALID_IO)
        .value("ERROR_INVALID_IMAGE_DIMENSIONS", SAIL_ERROR_INVALID_IMAGE_DIMENSIONS)
        .value("ERROR_UNSUPPORTED_PIXEL_FORMAT", SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT)
        .value("ERROR_INVALID_PIXEL_FORMAT", SAIL_ERROR_INVALID_PIXEL_FORMAT)
        .value("ERROR_UNSUPPORTED_COMPRESSION", SAIL_ERROR_UNSUPPORTED_COMPRESSION)
        .value("ERROR_UNSUPPORTED_META_DATA", SAIL_ERROR_UNSUPPORTED_META_DATA)
        .value("ERROR_UNDERLYING_CODEC", SAIL_ERROR_UNDERLYING_CODEC)
        .value("ERROR_NO_MORE_FRAMES", SAIL_ERROR_NO_MORE_FRAMES)
        .value("ERROR_INTERLACING_UNSUPPORTED", SAIL_ERROR_INTERLACING_UNSUPPORTED)
        .value("ERROR_INVALID_BYTES_PER_LINE", SAIL_ERROR_INVALID_BYTES_PER_LINE)
        .value("ERROR_UNSUPPORTED_IMAGE_PROPERTY", SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY)
        .value("ERROR_UNSUPPORTED_BIT_DEPTH", SAIL_ERROR_UNSUPPORTED_BIT_DEPTH)
        .value("ERROR_MISSING_PALETTE", SAIL_ERROR_MISSING_PALETTE)
        .value("ERROR_UNSUPPORTED_FORMAT", SAIL_ERROR_UNSUPPORTED_FORMAT)
        .value("ERROR_INVALID_IMAGE", SAIL_ERROR_INVALID_IMAGE)
        // Codec errors
        .value("ERROR_CODEC_LOAD", SAIL_ERROR_CODEC_LOAD)
        .value("ERROR_CODEC_NOT_FOUND", SAIL_ERROR_CODEC_NOT_FOUND)
        .value("ERROR_UNSUPPORTED_CODEC_LAYOUT", SAIL_ERROR_UNSUPPORTED_CODEC_LAYOUT)
        .value("ERROR_CODEC_SYMBOL_RESOLVE", SAIL_ERROR_CODEC_SYMBOL_RESOLVE)
        .value("ERROR_INCOMPLETE_CODEC_INFO", SAIL_ERROR_INCOMPLETE_CODEC_INFO)
        .value("ERROR_UNSUPPORTED_CODEC_FEATURE", SAIL_ERROR_UNSUPPORTED_CODEC_FEATURE)
        .value("ERROR_UNSUPPORTED_CODEC_PRIORITY", SAIL_ERROR_UNSUPPORTED_CODEC_PRIORITY)
        // libsail errors
        .value("ERROR_ENV_UPDATE", SAIL_ERROR_ENV_UPDATE)
        .value("ERROR_CONTEXT_UNINITIALIZED", SAIL_ERROR_CONTEXT_UNINITIALIZED)
        .value("ERROR_GET_DLL_PATH", SAIL_ERROR_GET_DLL_PATH)
        .value("ERROR_CONFLICTING_OPERATION", SAIL_ERROR_CONFLICTING_OPERATION)
        .export_values();

    // ============================================================================
    // SailOption - Load/Save options (can be or-ed)
    // ============================================================================

    py::enum_<SailOption>(m, "Option", py::arithmetic(), "Load or save options (can be or-ed)")
        .value("META_DATA", SAIL_OPTION_META_DATA, "Load or save meta data like JPEG comments or EXIF")
        .value("INTERLACED", SAIL_OPTION_INTERLACED, "Save interlaced images")
        .value("ICCP", SAIL_OPTION_ICCP, "Load or save embedded ICC profile")
        .value("SOURCE_IMAGE", SAIL_OPTION_SOURCE_IMAGE, "Preserve source image information in loading")
        .export_values();

    // ============================================================================
    // SailLogLevel
    // ============================================================================

    py::enum_<SailLogLevel>(m, "LogLevel", py::arithmetic(), "SAIL logging levels")
        .value("SILENCE", SAIL_LOG_LEVEL_SILENCE, "Silent mode (no logs)")
        .value("ERROR", SAIL_LOG_LEVEL_ERROR, "Error messages only")
        .value("WARNING", SAIL_LOG_LEVEL_WARNING, "Warnings and errors")
        .value("INFO", SAIL_LOG_LEVEL_INFO, "Informational messages")
        .value("MESSAGE", SAIL_LOG_LEVEL_MESSAGE, "Regular messages")
        .value("DEBUG", SAIL_LOG_LEVEL_DEBUG, "Debug messages (default)")
        .value("TRACE", SAIL_LOG_LEVEL_TRACE, "Verbose trace messages")
        .export_values();
}
