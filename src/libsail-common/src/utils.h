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

#ifndef SAIL_UTILS_H
#define SAIL_UTILS_H

#include <stdint.h>
#include <wchar.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

struct sail_image;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Duplicates the specified string and stores a new string in the specified output.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_strdup(const char *input, char **output);

/*
 * Duplicates the specified number of bytes of the specified input string and stores
 * a new string in the specified output. Length must be greater than 0.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_strdup_length(const char *input, size_t length, char **output);

/*
 * Concatenates 'num' number of strings together and puts the result into the specified output string.
 * The assigned output string MUST be destroyed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_concat(char **output, int num, ...);

/*
 * Converts the specified string to a lower case.
 */
SAIL_EXPORT void sail_to_lower(char *str);

/*
 * Converts the specified char* string into a wchar_t* string.
 * The assigned output string MUST be destroyed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_to_wchar(const char *input, wchar_t **output);

/*
 * Computes an unique hash of the specified string. It utilizes the djb2 algorithm
 * proposed by Dan Bernstein.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_string_hash(const char *str, uint64_t *hash);

/*
 * Assigns a non-NULL string representation of the specified pixel format. The assigned string
 * MUST NOT be destroyed. For example: "RGB".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_pixel_format_to_string(int pixel_format, const char **result);

/*
 * Assigns pixel format from a string representation.
 * For example: SAIL_PIXEL_FORMAT_SOURCE is assigned for "SOURCE".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_pixel_format_from_string(const char *str, int *result);

/*
 * Assigns a non-NULL string representation of the specified image property. See SailImageProperties.
 * The assigned string MUST NOT be destroyed. For example: "FLIPPED-VERTICALLY".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_image_property_to_string(int image_property, const char **result);

/*
 * Assigns image property from a string representation or 0. See SailImageProperties.
 * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is assigned for "FLIPPED-VERTICALLY".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_image_property_from_string(const char *str, int *result);

/*
 * Assigns a non-NULL string representation of the specified compression type. See SailCompressionTypes.
 * The assigned string MUST NOT be destroyed. For example: "RLE".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_compression_type_to_string(int compression, const char **result);

/*
 * Assigns compression from a string representation or 0. See SailCompressionTypes.
 * For example: SAIL_COMPRESSION_RLE is assigned for "RLE".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_compression_type_from_string(const char *str, int *result);

/*
 * Assigns a non-NULL string representation of the specified plugin feature. See SailPluginFeatures.
 * The assigned string MUST NOT be destroyed. For example: "STATIC".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_feature_to_string(int plugin_feature, const char **result);

/*
 * Assigns plugin feature from a string representation or 0. See SailPluginFeatures.
 * For example: SAIL_PLUGIN_FEATURE_STATIC is assigned for "STATIC".
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_feature_from_string(const char *str, int *result);

/*
 * Calculates a number of bits per pixel in the specified pixel format.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is assigned.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_bits_per_pixel(int pixel_format, int *result);

/*
 * Calculates a number of bytes per line needed to hold a scan line without padding.
 * 'width' and 'pixel_format' fields are used to calculate a result.
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
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_bytes_per_line(const struct sail_image *image, int *result);

/*
 * Calculates a number of bytes needed to hold an entire image in memory without padding.
 * It's effectively bytes per line * image height.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_bytes_per_image(const struct sail_image *image, int *result);

/*
 * Prints the recent errno value with SAIL_LOG_ERROR(). The specified format must include '%s'.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_print_errno(const char *format);

/*
 * Assigns the current number of milliseconds since Epoch.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_now(uint64_t *result);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
