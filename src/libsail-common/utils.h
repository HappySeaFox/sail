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

#ifndef SAIL_UTILS_H
#define SAIL_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_image;

/* Min/max macros. */
#define SAIL_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SAIL_MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Duplicates the specified memory buffer and stores a new buffer in the specified output.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_memdup(const void *input, size_t input_size, void **output);

/*
 * Duplicates the specified string and stores a new string in the specified output.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_strdup(const char *input, char **output);

/*
 * Duplicates the specified number of bytes of the specified input string and stores
 * a new string in the specified output. Length must be greater than 0.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_strdup_length(const char *input, size_t length, char **output);

/*
 * Concatenates 'num' number of strings together and puts the result into the specified output string.
 * The assigned output string MUST be destroyed later with sail_free().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_concat(char **output, int num, ...);

/*
 * Converts the specified string to a lower case.
 */
SAIL_EXPORT void sail_to_lower(char *str);

/*
 * Converts the specified char* string into a wchar_t* string.
 * The assigned output string MUST be destroyed later with sail_free().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_to_wchar(const char *input, wchar_t **output);

/*
 * Computes a unique hash of the specified string. It utilizes the djb2 algorithm proposed by Dan Bernstein.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_string_hash(const char *str, uint64_t *hash);

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

/*
 * Calculates the number of bits per pixel in the specified pixel format.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is assigned.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_bits_per_pixel(enum SailPixelFormat pixel_format, unsigned *result);

/*
 * Sets the result to true if the first pixel format occupies less bits than the second one.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_less_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result);

/*
 * Sets the result to true if the first pixel format occupies less or the same number of bits than the second one.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_less_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result);

/*
 * Sets the result to true if both the pixel formats occupy the same number of bits.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result);

/*
 * Sets the result to true if the first pixel format occupies more or the same number of bits than the second one.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_greater_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result);

/*
 * Sets the result to true if the first pixel format occupies more bits than the second one.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_greater_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result);

/*
 * Calculates the number of bytes per line needed to hold a scan line without padding.
 *
 * For example:
 *   - 12 pixels * 1 bits per pixel / 8 + 1 ==
 *     12 * 1/8 + 1                         ==
 *     12 * 0.125 + 1                       ==
 *     1.5 + 1                              ==
 *     2.5                                  ==
 *     2 bytes per line
 *
 *   - 12 pixels * 16 bits per pixel / 8 + 0 ==
 *     12 * 16/8 + 0                         ==
 *     12 * 2 + 0                            ==
 *     24 + 0                                ==
 *     24 bytes per line
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format, unsigned *result);

/*
 * Returns true if the given pixel format is indexed and assumes having a palette.
 */
SAIL_EXPORT bool sail_is_indexed(enum SailPixelFormat pixel_format);

/*
 * Returns true if the given pixel format is grayscale, with or without alpha.
 */
SAIL_EXPORT bool sail_is_grayscale(enum SailPixelFormat pixel_format);

/*
 * Returns true if the given pixel format is a kind of RGB, packed or not. E.g. RGBA, BGRA, RGB555 etc.
 */
SAIL_EXPORT bool sail_is_rgb_family(enum SailPixelFormat pixel_format);

/*
 * Prints the recent errno value with SAIL_LOG_ERROR(). The specified format must include '%s'.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_print_errno(const char *format);

/*
 * Interface to malloc().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_malloc(size_t size, void **ptr);

/*
 * Interface to realloc().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_realloc(size_t size, void **ptr);

/*
 * Interface to calloc().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_calloc(size_t nmemb, size_t size, void **ptr);

/*
 * Interface to free().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT void sail_free(void *ptr);

/*
 * Assigns the current number of milliseconds since Epoch.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT uint64_t sail_now(void);

/*
 * Returns true if the specified file system path exists.
 */
SAIL_EXPORT bool sail_path_exists(const char *path);

/*
 * Returns true if the specified file system path is a directory.
 */
SAIL_EXPORT bool sail_is_dir(const char *path);

/*
 * Returns true if the specified file system path is a regular file.
 */
SAIL_EXPORT bool sail_is_file(const char *path);

/*
 * Retrieves the file size.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_file_size(const char *path, size_t *size);

/*
 * Reads the specified file into the memory buffer. The buffer must be large enough.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_read_file_contents(const char *path, void *buffer);

/*
 * Allocates a memory buffer and reads the specified file into it.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_buffer_from_file_contents(const char *path, void **buffer, size_t *buffer_length);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
