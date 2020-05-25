/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_COMMON_H
#define SAIL_COMMON_H

#include <limits.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

/*
 * Common data structures and functions used across SAIL, both in libsail and in image plugins.
 */

/* Pixel format */
enum SailPixelFormat {

    /*
     * Unknown or unsupported pixel format that cannot be parsed by SAIL.
     */
    SAIL_PIXEL_FORMAT_UNKNOWN,

    /*
     * Copy the source pixels as is without converting them to a different pixel format.
     * This pixel format can be used in reading and writing operations.
     * If pixels are compressed with some compression algorithm (e.g. RLE),
     * reading operations still unpack them.
     */
    SAIL_PIXEL_FORMAT_SOURCE,

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

    /* Not to be used. Resize the enum for future elements. */
    _SAIL_PIXEL_FORMAT_RESIZE_ENUM_TO_INT = UINT_MAX
};

/* Image properties. */
enum SailImageProperties {

    /* Image needs flipping vertically. */
    SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY = 1 << 0,

    /*
     * Image is interlaced. Only sail_image.source_properties can have this property.
     * Reading operations never output interlaced images, that's why sail_image.properties
     * never has it.
     */
    SAIL_IMAGE_PROPERTY_INTERLACED         = 1 << 1,

    /* Not to be used. Resize the enum for future elements. */
    _SAIL_IMAGE_PROPERTIES_RESIZE_ENUM_TO_INT = UINT_MAX
};

/* Pixels compression types. */
enum SailCompressionTypes {

    /* RLE compression. */
    SAIL_COMPRESSION_RLE = 1 << 0,

    /* Not to be used. Resize the enum for future elements. */
    _SAIL_COMPRESSION_TYPES_RESIZE_ENUM_TO_INT = UINT_MAX
};

/* Plugin features. */
enum SailPluginFeatures {

    /* Ability to read or write static images. */
    SAIL_PLUGIN_FEATURE_STATIC     = 1 << 0,

    /* Ability to read or write animated images. */
    SAIL_PLUGIN_FEATURE_ANIMATED   = 1 << 1,

    /* Ability to read or write multipaged (not animated) images. */
    SAIL_PLUGIN_FEATURE_MULTIPAGED = 1 << 2,

    /* Ability to read or write simple image meta information like JPEG comments. */
    SAIL_PLUGIN_FEATURE_META_INFO  = 1 << 3,

    /* Ability to read or write EXIF meta information. */
    SAIL_PLUGIN_FEATURE_EXIF       = 1 << 4,

    /* Ability to read or write interlaced images. */
    SAIL_PLUGIN_FEATURE_INTERLACED = 1 << 5,

    /* Not to be used. Resize the enum for future elements. */
    _SAIL_PLUGIN_FEATURES_RESIZE_ENUM_TO_INT = UINT_MAX
};

/* Read or write options. */
enum SailIoOptions {

    /* Instruction to read or write simple image meta information like JPEG comments. */
    SAIL_IO_OPTION_META_INFO  = 1 << 0,

    /* Instruction to read or write EXIF meta information. */
    SAIL_IO_OPTION_EXIF       = 1 << 1,

    /* Instruction to write interlaced images. Specifying this option for reading operations has no effect. */
    SAIL_IO_OPTION_INTERLACED = 1 << 2,

    /* Not to be used. Resize the enum for future elements. */
    _SAIL_IO_OPTIONS_RESIZE_ENUM_TO_INT = UINT_MAX
};

#endif
