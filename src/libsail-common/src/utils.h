/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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
 * Returns a non-NULL string representation of the specified pixel format.
 * For example: "RGB".
 */
SAIL_EXPORT const char* sail_pixel_format_to_string(int pixel_format);

/*
 * Returns pixel format from string representation.
 * For example: SAIL_PIXEL_FORMAT_SOURCE is returned for "SOURCE".
 */
SAIL_EXPORT int sail_pixel_format_from_string(const char *str);

/*
 * Returns a non-NULL string representation of the specified image property. See SailImageProperties.
 * For example: "FLIPPED-VERTICALLY".
 */
SAIL_EXPORT const char* sail_image_property_to_string(int image_property);

/*
 * Returns image property from string representation or 0. See SailImageProperties.
 * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is returned for "FLIPPED-VERTICALLY".
 */
SAIL_EXPORT int sail_image_property_from_string(const char *str);

/*
 * Returns a non-NULL string representation of the specified compression type. See SailCompressionTypes.
 * For example: "RLE".
 */
SAIL_EXPORT const char* sail_compression_type_to_string(int compression);

/*
 * Returns compression from string representation or 0. See SailCompressionTypes.
 * For example: SAIL_COMPRESSION_RLE is returned for "RLE".
 */
SAIL_EXPORT int sail_compression_type_from_string(const char *str);

/*
 * Returns a non-NULL string representation of the specified plugin feature. See SailPluginFeatures.
 * For example: "STATIC".
 */
SAIL_EXPORT const char* sail_plugin_feature_to_string(int plugin_feature);

/*
 * Returns plugin feature from string representation or 0. See SailPluginFeatures.
 * For example: SAIL_PLUGIN_FEATURE_STATIC is returned for "STATIC".
 */
SAIL_EXPORT int sail_plugin_feature_from_string(const char *str);

/*
 * Returns a number of bits a pixel in the specified pixel format occupies.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is returned.
 */
SAIL_EXPORT int sail_bits_per_pixel(int pixel_format);

/*
 * Calculates bytes per line.
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
 */
SAIL_EXPORT int sail_bytes_per_line(int width, int pixel_format);


/*
 * Calculates bytes per image. It's effectively bytes per line * image height.
 */
SAIL_EXPORT int sail_bytes_per_image(const struct sail_image *image);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
