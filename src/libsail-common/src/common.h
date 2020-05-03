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
    #include <sail/error.h>
    #include <sail/export.h>
#endif

/*
 * Common data structures and functions used across SAIL, both in libsail and in image plugins.
 */

/* Pixel format */
enum SailPixelFormat {

    /* Unknown pixel format that cannot be parsed by SAIL. */
    SAIL_PIXEL_FORMAT_UNKNOWN,

    /*
     * Copy the source pixels as is without converting them to a different pixel format.
     */
    SAIL_PIXEL_FORMAT_SOURCE,

    SAIL_PIXEL_FORMAT_MONO,

    SAIL_PIXEL_FORMAT_GRAYSCALE,
    SAIL_PIXEL_FORMAT_INDEXED,

    SAIL_PIXEL_FORMAT_YCBCR,
    SAIL_PIXEL_FORMAT_CMYK,
    SAIL_PIXEL_FORMAT_YCCK,
    SAIL_PIXEL_FORMAT_RGB,
    SAIL_PIXEL_FORMAT_RGBX,
    SAIL_PIXEL_FORMAT_BGR,
    SAIL_PIXEL_FORMAT_BGRX,
    SAIL_PIXEL_FORMAT_XRGB,
    SAIL_PIXEL_FORMAT_XBGR,
    SAIL_PIXEL_FORMAT_RGBA,
    SAIL_PIXEL_FORMAT_BGRA,
    SAIL_PIXEL_FORMAT_ARGB,
    SAIL_PIXEL_FORMAT_ABGR,
    SAIL_PIXEL_FORMAT_RGB565,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_PIXEL_FORMAT_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Image properties. */
enum SailImageProperties {

    /* Image needs flipping vertically. */
    SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY = 1 << 0,

    /* Image is interlaced. */
    SAIL_IMAGE_PROPERTY_INTERLACED         = 1 << 1,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IMAGE_PROPERTIES_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Pixels compression types. */
enum SailCompressionTypes {

    /* RLE compression. */
    SAIL_COMPRESSION_RLE = 1 << 0,
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
    SAIL_PLUGIN_FEATURES_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Read or write options. */
enum SailIoOptions {

    /* Instruction to read or write simple image meta information like JPEG comments. */
    SAIL_IO_OPTION_META_INFO = 1 << 0,

    /* Instruction to read or write EXIF meta information. */
    SAIL_IO_OPTION_EXIF      = 1 << 1,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IO_OPTIONS_RESIZE_ENUM_TO_INT = INT_MAX
};

#endif
