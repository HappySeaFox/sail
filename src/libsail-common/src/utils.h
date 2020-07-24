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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Duplicates the specified string and stores a new string in the specified output.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_strdup(const char *input, char **output);

/*
 * Duplicates the specified number of bytes of the specified input string and stores
 * a new string in the specified output. Length must be greater than 0.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_strdup_length(const char *input, size_t length, char **output);

/*
 * Concatenates 'num' number of strings together and puts the result into the specified output string.
 * The assigned output string MUST be destroyed later with sail_free().
 *
 * Returns 0 on success or sail_status_t on error.
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
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_to_wchar(const char *input, wchar_t **output);

/*
 * Computes a unique hash of the specified string. It utilizes the djb2 algorithm proposed by Dan Bernstein.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_string_hash(const char *str, uint64_t *hash);

/*
 * Assigns a non-NULL string representation of the specified pixel format.
 * The assigned string MUST NOT be destroyed. For example: "RGB".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_pixel_format_to_string(enum SailPixelFormat pixel_format, const char **result);

/*
 * Assigns pixel format from a string representation.
 * For example: SAIL_PIXEL_FORMAT_SOURCE is assigned for "SOURCE".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_pixel_format_from_string(const char *str, enum SailPixelFormat *result);

/*
 * Assigns a non-NULL string representation of the specified image property. See SailImageProperty.
 * The assigned string MUST NOT be destroyed. For example: "FLIPPED-VERTICALLY".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_image_property_to_string(enum SailImageProperty image_property, const char **result);

/*
 * Assigns image property from a string representation or 0. See SailImageProperty.
 * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is assigned for "FLIPPED-VERTICALLY".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_image_property_from_string(const char *str, enum SailImageProperty *result);

/*
 * Assigns a non-NULL string representation of the specified compression type. See SailCompressionType.
 * The assigned string MUST NOT be destroyed. For example: "RLE".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_compression_type_to_string(enum SailCompressionType compression, const char **result);

/*
 * Assigns compression from a string representation or 0. See SailCompressionType.
 * For example: SAIL_COMPRESSION_RLE is assigned for "RLE".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_compression_type_from_string(const char *str, enum SailCompressionType *result);

/*
 * Assigns a non-NULL string representation of the specified plugin feature. See SailPluginFeature.
 * The assigned string MUST NOT be destroyed. For example: "STATIC".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_plugin_feature_to_string(enum SailPluginFeature plugin_feature, const char **result);

/*
 * Assigns plugin feature from a string representation or 0. See SailPluginFeature.
 * For example: SAIL_PLUGIN_FEATURE_STATIC is assigned for "STATIC".
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_plugin_feature_from_string(const char *str, enum SailPluginFeature *result);

/*
 * Calculates the number of bits per pixel in the specified pixel format.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is assigned.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_bits_per_pixel(enum SailPixelFormat pixel_format, unsigned *result);

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
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format, unsigned *result);

/*
 * Calculates the number of bytes needed to hold an entire image in memory without padding.
 * It is effectively bytes per line * image height.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_bytes_per_image(const struct sail_image *image, unsigned *result);

/*
 * Prints the recent errno value with SAIL_LOG_ERROR(). The specified format must include '%s'.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_print_errno(const char *format);

/*
 * Interface to malloc().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_malloc(void **ptr, size_t size);

/*
 * Interface to realloc().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_realloc(void **ptr, size_t size);

/*
 * Interface to calloc().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_calloc(void **ptr, size_t nmemb, size_t size);

/*
 * Interface to free().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT void sail_free(void *ptr);

/*
 * Assigns the current number of milliseconds since Epoch.
 *
 * Returns 0 on success or sail_status_t on error.
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

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
