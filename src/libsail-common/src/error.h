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
    SAIL_ERROR_STATE_NULL_PTR,
    SAIL_ERROR_IMAGE_NULL_PTR,
    SAIL_ERROR_PIXELS_NULL_PTR,
    SAIL_ERROR_READ_FEATURES_NULL_PTR,
    SAIL_ERROR_READ_OPTIONS_NULL_PTR,
    SAIL_ERROR_WRITE_FEATURES_NULL_PTR,
    SAIL_ERROR_WRITE_OPTIONS_NULL_PTR,
    SAIL_ERROR_STRING_NULL_PTR,
    SAIL_ERROR_IO_NULL_PTR,
    SAIL_ERROR_STREAM_NULL_PTR,
    SAIL_ERROR_DATA_NULL_PTR,
    SAIL_ERROR_BUFFER_NULL_PTR,
    SAIL_ERROR_INVALID_IO,
    SAIL_ERROR_RESULT_NULL_PTR,
    SAIL_ERROR_META_DATA_NODE_NULL_PTR,
    SAIL_ERROR_ICCP_NULL_PTR,
    SAIL_ERROR_PALETTE_NULL_PTR,
    SAIL_ERROR_SOURCE_IMAGE_NULL_PTR,
    SAIL_ERROR_PIXEL_FORMATS_MAPPING_NODE_NULL_PTR,
    SAIL_ERROR_STRING_NODE_NULL_PTR,
    SAIL_ERROR_CODEC_INFO_NODE_NULL_PTR,
    SAIL_ERROR_PIXEL_FORMAT_NULL_PTR,
    SAIL_ERROR_RESOLUTION_NULL_PTR,

    /*
     * Encoding/decoding specific errors.
     */
    SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS = 200,
    SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT,
    SAIL_ERROR_UNSUPPORTED_COMPRESSION,
    SAIL_ERROR_UNSUPPORTED_META_DATA,
    SAIL_ERROR_UNDERLYING_CODEC,
    SAIL_ERROR_NO_MORE_FRAMES,
    SAIL_ERROR_INTERLACING_UNSUPPORTED,
    SAIL_ERROR_INCORRECT_BYTES_PER_LINE,
    SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY,
    SAIL_ERROR_UNSUPPORTED_BIT_DEPTH,
    SAIL_ERROR_MISSING_PALETTE,

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
    SAIL_ERROR_CONTEXT_NULL_PTR = 400,
    SAIL_ERROR_PATH_NULL_PTR,
    SAIL_ERROR_EXTENSION_NULL_PTR,
    SAIL_ERROR_CODEC_INFO_NULL_PTR,
    SAIL_ERROR_CODEC_NULL_PTR,
    SAIL_ERROR_ENV_UPDATE,
    SAIL_ERROR_CONTEXT_UNINITIALIZED,
};

typedef enum SailStatus sail_status_t;

/*
 * Helper macros.
 */
#define SAIL_CHECK_IO(io)              \
do {                                   \
    if (io == NULL) {                  \
        return SAIL_ERROR_IO_NULL_PTR; \
    }                                  \
    if (io->read == NULL      ||       \
            io->seek == NULL  ||       \
            io->tell == NULL  ||       \
            io->write == NULL ||       \
            io->flush == NULL ||       \
            io->close == NULL ||       \
            io->eof == NULL) {         \
        return SAIL_ERROR_INVALID_IO;  \
    }                                  \
} while(0)

#define SAIL_CHECK_IMAGE(image)                       \
do {                                                  \
    if (image == NULL) {                              \
        return SAIL_ERROR_IMAGE_NULL_PTR;             \
    }                                                 \
    if (image->width == 0 || image->height == 0) {    \
        return SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS; \
    }                                                 \
    if (image->bytes_per_line == 0) {                 \
        return SAIL_ERROR_INCORRECT_BYTES_PER_LINE;   \
    }                                                 \
} while(0)

#define SAIL_CHECK_PTR(ptr)         \
do {                                \
    if (ptr == NULL) {              \
        return SAIL_ERROR_NULL_PTR; \
    }                               \
} while(0)

#define SAIL_CHECK_PTR2(ptr, ret) \
do {                              \
    if (ptr == NULL) {            \
        return ret;               \
    }                             \
} while(0)

#define SAIL_CHECK_PIXELS_PTR(pixels)                   SAIL_CHECK_PTR2(pixels,          SAIL_ERROR_PIXELS_NULL_PTR)
#define SAIL_CHECK_READ_FEATURES_PTR(read_features)     SAIL_CHECK_PTR2(read_features,   SAIL_ERROR_READ_FEATURES_NULL_PTR)
#define SAIL_CHECK_READ_OPTIONS_PTR(read_options)       SAIL_CHECK_PTR2(read_options,    SAIL_ERROR_READ_OPTIONS_NULL_PTR)
#define SAIL_CHECK_WRITE_FEATURES_PTR(write_features)   SAIL_CHECK_PTR2(write_features,  SAIL_ERROR_WRITE_FEATURES_NULL_PTR)
#define SAIL_CHECK_WRITE_OPTIONS_PTR(write_options)     SAIL_CHECK_PTR2(write_options,   SAIL_ERROR_WRITE_FEATURES_NULL_PTR)
#define SAIL_CHECK_CONTEXT_PTR(context)                 SAIL_CHECK_PTR2(context,         SAIL_ERROR_CONTEXT_NULL_PTR)
#define SAIL_CHECK_PATH_PTR(path)                       SAIL_CHECK_PTR2(path,            SAIL_ERROR_PATH_NULL_PTR)
#define SAIL_CHECK_EXTENSION_PTR(extension)             SAIL_CHECK_PTR2(extension,       SAIL_ERROR_EXTENSION_NULL_PTR)
#define SAIL_CHECK_IMAGE_PTR(image)                     SAIL_CHECK_PTR2(image,           SAIL_ERROR_IMAGE_NULL_PTR)
#define SAIL_CHECK_CODEC_INFO_PTR(codec_info)           SAIL_CHECK_PTR2(codec_info,      SAIL_ERROR_CODEC_INFO_NULL_PTR)
#define SAIL_CHECK_CODEC_PTR(codec)                     SAIL_CHECK_PTR2(codec,           SAIL_ERROR_CODEC_NULL_PTR)
#define SAIL_CHECK_STATE_PTR(state)                     SAIL_CHECK_PTR2(state,           SAIL_ERROR_STATE_NULL_PTR)
#define SAIL_CHECK_STRING_PTR(str)                      SAIL_CHECK_PTR2(str,             SAIL_ERROR_STRING_NULL_PTR)
#define SAIL_CHECK_IO_PTR(io)                           SAIL_CHECK_PTR2(io,              SAIL_ERROR_IO_NULL_PTR)
#define SAIL_CHECK_STREAM_PTR(stream)                   SAIL_CHECK_PTR2(stream,          SAIL_ERROR_STREAM_NULL_PTR)
#define SAIL_CHECK_DATA_PTR(data)                       SAIL_CHECK_PTR2(data,            SAIL_ERROR_DATA_NULL_PTR)
#define SAIL_CHECK_BUFFER_PTR(buffer)                   SAIL_CHECK_PTR2(buffer,          SAIL_ERROR_BUFFER_NULL_PTR)
#define SAIL_CHECK_RESULT_PTR(result)                   SAIL_CHECK_PTR2(result,          SAIL_ERROR_RESULT_NULL_PTR)
#define SAIL_CHECK_META_DATA_NODE_PTR(meta_data_node)   SAIL_CHECK_PTR2(meta_data_node,  SAIL_ERROR_META_DATA_NODE_NULL_PTR)
#define SAIL_CHECK_ICCP_PTR(iccp)                       SAIL_CHECK_PTR2(iccp,            SAIL_ERROR_ICCP_NULL_PTR)
#define SAIL_CHECK_PALETTE_PTR(palette)                 SAIL_CHECK_PTR2(palette,         SAIL_ERROR_PALETTE_NULL_PTR)
#define SAIL_CHECK_SOURCE_IMAGE_PTR(source_image)       SAIL_CHECK_PTR2(source_image,    SAIL_ERROR_SOURCE_IMAGE_NULL_PTR)
#define SAIL_CHECK_PIXEL_FORMATS_MAPPING_NODE_PTR(node) SAIL_CHECK_PTR2(node,            SAIL_ERROR_PIXEL_FORMATS_MAPPING_NODE_NULL_PTR)
#define SAIL_CHECK_STRING_NODE_PTR(node)                SAIL_CHECK_PTR2(node,            SAIL_ERROR_STRING_NODE_NULL_PTR)
#define SAIL_CHECK_CODEC_INFO_NODE_PTR(node)            SAIL_CHECK_PTR2(node,            SAIL_ERROR_CODEC_INFO_NODE_NULL_PTR)
#define SAIL_CHECK_PIXEL_FORMAT_PTR(pixel_format)       SAIL_CHECK_PTR2(pixel_format,    SAIL_ERROR_PIXEL_FORMAT_NULL_PTR)
#define SAIL_CHECK_RESOLUTION_PTR(resolution)           SAIL_CHECK_PTR2(resolution,      SAIL_ERROR_RESOLUTION_NULL_PTR)

/*
 * Try to execute the specified SAIL function. If it fails, execute the rest of arguments.
 * Use do/while to require ';' at the end of a SAIL_TRY_OR_EXECUTE() expression.
 */
#define SAIL_TRY_OR_EXECUTE(sail_func, ...)       \
do {                                              \
    sail_status_t __sail_error_result;            \
                                                  \
    if ((__sail_error_result = sail_func) != 0) { \
        __VA_ARGS__;                              \
    }                                             \
} while(0)

/*
 * Try to execute the specified SAIL function. If it fails, return the error code.
 * Use do/while to require ';' at the end of a SAIL_TRY() expression.
 */
#define SAIL_TRY(sail_func) SAIL_TRY_OR_EXECUTE(sail_func, return __sail_error_result)

/*
 * Try to execute the specified SAIL function. If it fails, ignore the error and continue execution.
 * Use do/while to require ';' at the end of a SAIL_TRY_OR_SUPPRESS() expression.
 */
#define SAIL_TRY_OR_SUPPRESS(sail_func) SAIL_TRY_OR_EXECUTE(sail_func, (void)0)

/*
 * Try to execute the specified SAIL function. If it fails, execute the rest of arguments
 * (so called cleanup), and return the error code. Use do/while to require ';' at the end
 * of a SAIL_TRY_OR_CLEANUP() expression.
 */
#define SAIL_TRY_OR_CLEANUP(sail_func, ...) \
do {                                        \
    sail_status_t res;                      \
                                            \
    if ((res = sail_func) != 0) {           \
        __VA_ARGS__;                        \
        return res;                         \
    }                                       \
} while(0)

#endif
