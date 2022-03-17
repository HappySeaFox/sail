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

#ifndef SAIL_COMMON_SERIALIZE_H
#define SAIL_COMMON_SERIALIZE_H

#ifdef SAIL_BUILD
    #include "common.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns a string representation of the specified pixel format.
 * For example: "BPP32-RGBA" is returned for SAIL_PIXEL_FORMAT_BPP32_RGBA.
 *
 * Returns NULL if the pixel format is not known.
 */
SAIL_EXPORT const char* sail_pixel_format_to_string(enum SailPixelFormat pixel_format);

/*
 * Returns a pixel format from the string representation.
 * For example: SAIL_PIXEL_FORMAT_BPP32_RGBA is returned for "BPP32-RGBA".
 *
 * Returns SAIL_PIXEL_FORMAT_UNKNOWN if the pixel format is not known.
 */
SAIL_EXPORT enum SailPixelFormat sail_pixel_format_from_string(const char *str);

/*
 * Returns a string representation of the specified image property. See SailImageProperty.
 * For example: "FLIPPED-VERTICALLY" is returned for SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY.
 *
 * Returns NULL if the property is not known.
 */
SAIL_EXPORT const char* sail_image_property_to_string(enum SailImageProperty image_property);

/*
 * Returns an image property from the string representation. See SailImageProperty.
 * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is returned for "FLIPPED-VERTICALLY".
 *
 * Returns SAIL_IMAGE_PROPERTY_UNKNOWN if the property is not known.
 */
SAIL_EXPORT enum SailImageProperty sail_image_property_from_string(const char *str);

/*
 * Returns string representation of the specified compression type. See SailCompression.
 * For example: "RLE" is returned for SAIL_COMPRESSION_RLE.
 *
 * Returns NULL if the compression is not known.
 */
SAIL_EXPORT const char* sail_compression_to_string(enum SailCompression compression);

/*
 * Returns a compression from the string representation. See SailCompression.
 * For example: SAIL_COMPRESSION_RLE is returned for "RLE".
 *
 * Returns SAIL_COMPRESSION_UNKNOWN if the compression is not known.
 */
SAIL_EXPORT enum SailCompression sail_compression_from_string(const char *str);

/*
 * Returns a string representation of the specified meta data key. See SailMetaData.
 * For example: "Author" is returned for SAIL_META_DATA_AUTHOR.
 *
 * Returns NULL if the meta data key is not known.
 */
SAIL_EXPORT const char* sail_meta_data_to_string(enum SailMetaData meta_data);

/*
 * Returns a meta data key from the string representation. See SailMetaData.
 * For example: SAIL_META_DATA_AUTHOR is returned for "Author".
 *
 * Returns SAIL_META_DATA_UNKNOWN if the meta data key is not known.
 */
SAIL_EXPORT enum SailMetaData sail_meta_data_from_string(const char *str);

/*
 * Returns a string representation of the specified codec feature. See SailCodecFeature.
 * For example: "STATIC" is returned for SAIL_CODEC_FEATURE_STATIC.
 *
 * Returns NULL if the codec feature is not known.
 */
SAIL_EXPORT const char* sail_codec_feature_to_string(enum SailCodecFeature codec_feature);

/*
 * Returns a codec feature from the string representation. See SailCodecFeature.
 * For example: SAIL_CODEC_FEATURE_STATIC is returned for "STATIC".
 *
 * Returns SAIL_CODEC_FEATURE_UNKNOWN if the codec feature is not known.
 */
SAIL_EXPORT enum SailCodecFeature sail_codec_feature_from_string(const char *str);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
