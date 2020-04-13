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

#ifndef SAIL_ERROR_H
#define SAIL_ERROR_H

/*
 * Common error type to return from all SAIL functions.
 * 0 means success. See the errors below.
 */
typedef int sail_error_t;

/*
 * Common errors.
 */
#define SAIL_MEMORY_ALLOCATION_FAILED     1
#define SAIL_FILE_OPEN_ERROR              2
#define SAIL_DIR_OPEN_ERROR               3
#define SAIL_FILE_PARSE_ERROR             4
#define SAIL_INVALID_ARGUMENT             5

/*
 * Encoding/decoding common errors.
 */
#define SAIL_FILE_NULL_PTR                21
#define SAIL_IMAGE_NULL_PTR               22
#define SAIL_SCAN_LINE_NULL_PTR           23

/*
 * Encoding/decoding specific errors.
 */
#define SAIL_INCORRECT_IMAGE_DIMENSIONS   40
#define SAIL_UNSUPPORTED_PIXEL_FORMAT     41
#define SAIL_UNSUPPORTED_COMPRESSION_TYPE 42
#define SAIL_UNDERLYING_CODEC_ERROR       43
#define SAIL_NO_MORE_FRAMES               44

/*
 * Plugins-specific errors.
 */
#define SAIL_PLUGIN_LOAD_ERROR            60
#define SAIL_PLUGIN_NOT_FOUND             61
#define SAIL_UNSUPPORTED_PLUGIN_LAYOUT    62

/*
 * libsail errors.
 */
#define SAIL_CONTEXT_NULL_PTR             80
#define SAIL_PATH_NULL_PTR                81
#define SAIL_PLUGIN_INFO_NULL_PTR         82

/*
 * Helper macros.
 */
#define SAIL_CHECK_FILE(file)      \
do {                               \
    if (file == NULL) {            \
        return SAIL_FILE_NULL_PTR; \
    }                              \
    if (file->fptr == NULL) {      \
        return SAIL_FILE_NULL_PTR; \
    }                              \
} while(0)

#define SAIL_CHECK_IMAGE(image)                    \
do {                                               \
    if (image == NULL) {                           \
        return SAIL_IMAGE_NULL_PTR;                \
    }                                              \
    if (image->width <= 0 || image->height <= 0) { \
        return SAIL_INCORRECT_IMAGE_DIMENSIONS;    \
    }                                              \
} while(0)

#define SAIL_CHECK_SCAN_LINE(scan)      \
do {                                    \
    if (scan == NULL) {                 \
        return SAIL_SCAN_LINE_NULL_PTR; \
    }                                   \
} while(0)

#define SAIL_CHECK_CONTEXT(context)   \
do {                                  \
    if (context == NULL) {            \
        return SAIL_CONTEXT_NULL_PTR; \
    }                                 \
} while(0)

#define SAIL_CHECK_PATH(path)      \
do {                               \
    if (path == NULL) {            \
        return SAIL_PATH_NULL_PTR; \
    }                              \
} while(0)

#define SAIL_CHECK_PLUGIN_INFO(plugin_info) \
do {                                        \
    if (plugin_info == NULL) {              \
        return SAIL_PLUGIN_INFO_NULL_PTR;   \
    }                                       \
} while(0)

/*
 * Try to execute the specified SAIL function. If it fails, return the error code.
 * Use do/while to require ';' at the end of a SAIL_TRY() expression.
 */
#define SAIL_TRY(sail_func)       \
do {                              \
    int res;                      \
                                  \
    if ((res = sail_func) != 0) { \
        return res;               \
    }                             \
} while(0)

/*
 * Try to execute the specified SAIL function. If it fails, execute the rest of arguments
 * (so called cleanup), and return the error code. Use do/while to require ';' at the end
 * of a SAIL_TRY_OR_CLEANUP() expression.
 */
#define SAIL_TRY_OR_CLEANUP(sail_func, ...) \
do {                                        \
    int res;                                \
                                            \
    if ((res = sail_func) != 0) {           \
        __VA_ARGS__;                        \
        return res;                         \
    }                                       \
} while(0)

#endif
