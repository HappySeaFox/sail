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

#ifndef SAIL_ERROR_H
#define SAIL_ERROR_H

#ifdef SAIL_BUILD
    #include "compiler_specifics.h"
#else
    #include <sail-common/compiler_specifics.h>
#endif

/*
 * Common status type to return from all SAIL functions.
 */
enum SailStatus {

    /*
     * Success.
     */
    SAIL_OK = 0,

    /*
     * Common errors.
     */
    SAIL_ERROR_MEMORY_ALLOCATION = 1,
    SAIL_ERROR_OPEN_FILE,
    SAIL_ERROR_READ_FILE,
    SAIL_ERROR_SEEK_FILE,
    SAIL_ERROR_CLOSE_FILE,
    SAIL_ERROR_LIST_DIR,
    SAIL_ERROR_PARSE_FILE,
    SAIL_ERROR_INVALID_ARGUMENT,
    SAIL_ERROR_READ_IO,
    SAIL_ERROR_WRITE_IO,
    SAIL_ERROR_FLUSH_IO,
    SAIL_ERROR_SEEK_IO,
    SAIL_ERROR_TELL_IO,
    SAIL_ERROR_CLOSE_IO,
    SAIL_ERROR_EOF,
    SAIL_ERROR_NOT_IMPLEMENTED,
    SAIL_ERROR_UNSUPPORTED_SEEK_WHENCE,
    SAIL_ERROR_EMPTY_STRING,

    /*
     * Encoding/decoding common errors.
     */
    SAIL_ERROR_NULL_PTR = 100,
    SAIL_ERROR_INVALID_IO,

    /*
     * Encoding/decoding specific errors.
     */
    SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS = 200,
    SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT,
    SAIL_ERROR_INVALID_PIXEL_FORMAT,
    SAIL_ERROR_UNSUPPORTED_COMPRESSION,
    SAIL_ERROR_UNSUPPORTED_META_DATA,
    SAIL_ERROR_UNDERLYING_CODEC,
    SAIL_ERROR_NO_MORE_FRAMES,
    SAIL_ERROR_INTERLACING_UNSUPPORTED,
    SAIL_ERROR_INCORRECT_BYTES_PER_LINE,
    SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY,
    SAIL_ERROR_UNSUPPORTED_BIT_DEPTH,
    SAIL_ERROR_MISSING_PALETTE,
    SAIL_ERROR_UNSUPPORTED_FORMAT,
    SAIL_ERROR_BROKEN_IMAGE,

    /*
     * Codecs-specific errors.
     */
    SAIL_ERROR_CODEC_LOAD = 300,
    SAIL_ERROR_CODEC_NOT_FOUND,
    SAIL_ERROR_UNSUPPORTED_CODEC_LAYOUT,
    SAIL_ERROR_CODEC_SYMBOL_RESOLVE,
    SAIL_ERROR_INCOMPLETE_CODEC_INFO,
    SAIL_ERROR_UNSUPPORTED_CODEC_FEATURE,

    /*
     * libsail errors.
     */
    SAIL_ERROR_ENV_UPDATE = 400,
    SAIL_ERROR_CONTEXT_UNINITIALIZED,
    SAIL_ERROR_GET_DLL_PATH,
    SAIL_ERROR_CONFLICTING_OPERATION,
};

typedef enum SailStatus sail_status_t;

/*
 * Log failure and return.
 */
#define SAIL_LOG_AND_RETURN(code) \
do {                              \
    SAIL_LOG_ERROR("%s", #code);  \
    return code;                  \
} while(0)

/*
 * Helper macros.
 */
#define SAIL_CHECK_PTR(ptr)                            \
do {                                                   \
    if (SAIL_UNLIKELY(ptr == NULL)) {                  \
        SAIL_LOG_ERROR("'%s' argument is NULL", #ptr); \
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NULL_PTR);      \
    }                                                  \
} while(0)

/*
 * Tries to execute the specified SAIL function. If it fails, executes the rest of the arguments which
 * can be separated by commas or by semicolons.
 * Use do/while to require ';' at the end of a SAIL_TRY_OR_EXECUTE() expression.
 */
#define SAIL_TRY_OR_EXECUTE(sail_func, ...)              \
{                                                        \
    const sail_status_t __sail_error_result = sail_func; \
                                                         \
    if (SAIL_UNLIKELY(__sail_error_result != SAIL_OK)) { \
        __VA_ARGS__;                                     \
    }                                                    \
} do{} while(0)

/*
 * Tries to execute the specified SAIL function. If it fails, returns the error code.
 */
#define SAIL_TRY(sail_func) SAIL_TRY_OR_EXECUTE(sail_func, return __sail_error_result)

/*
 * Tries to execute the specified SAIL function. If it fails, ignores the error and continues execution.
 */
#define SAIL_TRY_OR_SUPPRESS(sail_func) SAIL_TRY_OR_EXECUTE(sail_func, (void)0)

/*
 * Tries to execute the specified SAIL function. If it fails, executes the rest of the arguments which
 * can be separated by commas or by semicolons (so called cleanup), and returns the error code.
 */
#define SAIL_TRY_OR_CLEANUP(sail_func, ...) SAIL_TRY_OR_EXECUTE(sail_func, __VA_ARGS__; return __sail_error_result)

#endif
