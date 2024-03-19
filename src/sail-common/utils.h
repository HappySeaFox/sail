/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <stddef.h> /* size_t */
#include <stdint.h>
#include <wchar.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

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
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_to_wchar(const char *input, wchar_t **output);

/*
 * Computes a unique hash of the specified string. It utilizes the djb2 algorithm proposed by Dan Bernstein.
 *
 * Returns 0 if the string is NULL or empty.
 */
SAIL_EXPORT uint64_t sail_string_hash(const char *str);

/*
 * Returns the number of bits per pixel of the specified pixel format.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is returned. Returns 0 on unknown pixel format.
 */
SAIL_EXPORT unsigned sail_bits_per_pixel(enum SailPixelFormat pixel_format);

/*
 * Returns true if the first pixel format occupies less bits than the second one.
 */
SAIL_EXPORT bool sail_less_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2);

/*
 * Returns true if the first pixel format occupies less or the same number of bits than the second one.
 */
SAIL_EXPORT bool sail_less_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2);

/*
 * Returns true if both the pixel formats occupy the same number of bits.
 */
SAIL_EXPORT bool sail_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2);

/*
 * Returns true if the first pixel format occupies more or the same number of bits than the second one.
 */
SAIL_EXPORT bool sail_greater_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2);

/*
 * Returns true if the first pixel format occupies more bits than the second one.
 */
SAIL_EXPORT bool sail_greater_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2);

/*
 * Returns the number of bytes per line needed to hold a scan line of the given width
 * without padding.
 *
 * For example:
 *   sail_bytes_per_line(12, SAIL_PIXEL_FORMAT_BPP1) ==
 *     (12 pixels * 1 bits per pixel + 7) / 8        ==
 *     (12 * 1 + 7) / 8                              ==
 *     (12 + 7 ) / 8                                 ==
 *     19 / 8                                        ==
 *     2 bytes per line
 */
SAIL_EXPORT unsigned sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format);

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
 */
SAIL_EXPORT void sail_print_errno(const char *format);

/*
 * Returns the current number of milliseconds since Epoch or 0 on error.
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
SAIL_EXPORT sail_status_t sail_file_contents_into_data(const char *path, void *data);

/*
 * Allocates a memory buffer and reads the specified file into it.
 * The size of the memory buffer is stored in 'data_size'.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_data_from_file_contents(const char *path, void **data, size_t *data_size);

/*
 * Decodes the specified HEX-encoded string into the specified memory buffer.
 * The memory buffer must be large enough. Every encoded byte must be in form of two HEX characters.
 * For example: 04 or 5A. Skips any white spaces.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_hex_string_into_data(const char *str, void *data);

/*
 * Allocates a memory buffer and decodes the specified HEX-encoded string into it.
 * Every encoded byte must be in form of two HEX characters. For example: 04 or 5A.
 * Skips any white spaces. The size of the decoded block is stored in 'data_size'.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_hex_string_to_data(const char *str, void **data, size_t *data_size);

/*
 * Encodes the specified memory buffer into the HEX-encoded string. The string must be large enough.
 * Every encoded byte will be in form of two HEX characters. For example: 04 or 5A.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_data_into_hex_string(const void *data, size_t data_size, char *str);

/*
 * Allocates a string and HEX-encodes the specified memory buffer into it.
 * Every encoded byte will be in form of two HEX characters. For example: 04 or 5A.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_data_to_hex_string(const void *data, size_t data_size, char **str);

/*
 * Reverses the input value byte order.
 *
 * Returns the reversed value.
 */
SAIL_EXPORT uint16_t sail_reverse_uint16(uint16_t v);

/*
 * Reverses the input value byte order.
 *
 * Returns the reversed value.
 */
SAIL_EXPORT uint32_t sail_reverse_uint32(uint32_t v);

/*
 * Reverses the input value byte order.
 *
 * Returns the reversed value.
 */
SAIL_EXPORT uint64_t sail_reverse_uint64(uint64_t v);

/*
 * Converts a character string to a UTF-16 string. Available on Windows only.
 *
 * Returns SAIL_OK on success.
 */
#ifdef SAIL_WINDOWS_UTF8_PATHS
SAIL_EXPORT sail_status_t sail_multibyte_to_wchar(const char *str, wchar_t **wstr);
#endif

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
