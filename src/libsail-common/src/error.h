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
#define SAIL_IO_READ_ERROR                6
#define SAIL_IO_WRITE_ERROR               7
#define SAIL_IO_FLUSH_ERROR               8
#define SAIL_IO_SEEK_ERROR                9
#define SAIL_IO_TELL_ERROR                10
#define SAIL_IO_CLOSE_ERROR               11
#define SAIL_IO_EOF                       12
#define SAIL_NOT_IMPLEMENTED              13

/*
 * Encoding/decoding common errors.
 */
#define SAIL_NULL_PTR                     20
#define SAIL_STATE_NULL_PTR               21
#define SAIL_IMAGE_NULL_PTR               22
#define SAIL_SCAN_LINE_NULL_PTR           23
#define SAIL_READ_FEATURES_NULL_PTR       24
#define SAIL_READ_OPTIONS_NULL_PTR        25
#define SAIL_WRITE_FEATURES_NULL_PTR      26
#define SAIL_WRITE_OPTIONS_NULL_PTR       27
#define SAIL_STRING_NULL_PTR              28
#define SAIL_IO_NULL_PTR                  29
#define SAIL_STREAM_NULL_PTR              30
#define SAIL_BUFFER_NULL_PTR              31
#define SAIL_INVALID_IO                   32
#define SAIL_RESULT_NULL_PTR              33
#define SAIL_META_ENTRY_NODE_NULL_PTR     34
#define SAIL_ICCP_NULL_PTR                35
#define SAIL_PALETTE_NULL_PTR             36

/*
 * Encoding/decoding specific errors.
 */
#define SAIL_INCORRECT_IMAGE_DIMENSIONS   40
#define SAIL_UNSUPPORTED_PIXEL_FORMAT     41
#define SAIL_UNSUPPORTED_COMPRESSION_TYPE 42
#define SAIL_UNDERLYING_CODEC_ERROR       43
#define SAIL_NO_MORE_FRAMES               44
#define SAIL_INTERLACED_UNSUPPORTED       45
#define SAIL_INCORRECT_BYTES_PER_LINE     46
#define SAIL_UNSUPPORTED_IMAGE_PROPERTY   47
#define SAIL_UNSUPPORTED_BIT_DEPTH        48

/*
 * Plugins-specific errors.
 */
#define SAIL_PLUGIN_LOAD_ERROR            60
#define SAIL_PLUGIN_NOT_FOUND             61
#define SAIL_UNSUPPORTED_PLUGIN_LAYOUT    62
#define SAIL_PLUGIN_SYMBOL_RESOLVE_FAILED 63
#define SAIL_INCOMPLETE_PLUGIN_INFO       64
#define SAIL_UNSUPPORTED_PLUGIN_FEATURE   65

/*
 * libsail errors.
 */
#define SAIL_CONTEXT_NULL_PTR             80
#define SAIL_PATH_NULL_PTR                81
#define SAIL_EXTENSION_NULL_PTR           82
#define SAIL_PLUGIN_INFO_NULL_PTR         83
#define SAIL_PLUGIN_NULL_PTR              84
#define SAIL_ENV_UPDATE_FAILED            85
#define SAIL_CONTEXT_UNINITIALIZED        86

/*
 * Helper macros.
 */
#define SAIL_CHECK_IO(io)        \
do {                             \
    if (io == NULL) {            \
        return SAIL_IO_NULL_PTR; \
    }                            \
    if (io->read == NULL      || \
            io->seek == NULL  || \
            io->tell == NULL  || \
            io->write == NULL || \
            io->flush == NULL || \
            io->close == NULL || \
            io->eof == NULL) {   \
        return SAIL_INVALID_IO;  \
    }                            \
} while(0)

#define SAIL_CHECK_IMAGE(image)                    \
do {                                               \
    if (image == NULL) {                           \
        return SAIL_IMAGE_NULL_PTR;                \
    }                                              \
    if (image->width <= 0 || image->height <= 0) { \
        return SAIL_INCORRECT_IMAGE_DIMENSIONS;    \
    }                                              \
    if (image->bytes_per_line <= 0) {              \
        return SAIL_INCORRECT_BYTES_PER_LINE;      \
    }                                              \
} while(0)

#define SAIL_CHECK_PTR(ptr)   \
do {                          \
    if (ptr == NULL) {        \
        return SAIL_NULL_PTR; \
    }                         \
} while(0)

#define SAIL_CHECK_PTR2(ptr, ret) \
do {                              \
    if (ptr == NULL) {            \
        return ret;               \
    }                             \
} while(0)

#define SAIL_CHECK_SCAN_LINE_PTR(scan)                  SAIL_CHECK_PTR2(scan,            SAIL_SCAN_LINE_NULL_PTR)
#define SAIL_CHECK_READ_FEATURES_PTR(read_features)     SAIL_CHECK_PTR2(read_features,   SAIL_READ_FEATURES_NULL_PTR)
#define SAIL_CHECK_READ_OPTIONS_PTR(read_options)       SAIL_CHECK_PTR2(read_options,    SAIL_READ_OPTIONS_NULL_PTR)
#define SAIL_CHECK_WRITE_FEATURES_PTR(write_features)   SAIL_CHECK_PTR2(write_features,  SAIL_WRITE_FEATURES_NULL_PTR)
#define SAIL_CHECK_WRITE_OPTIONS_PTR(write_options)     SAIL_CHECK_PTR2(write_options,   SAIL_WRITE_FEATURES_NULL_PTR)
#define SAIL_CHECK_CONTEXT_PTR(context)                 SAIL_CHECK_PTR2(context,         SAIL_CONTEXT_NULL_PTR)
#define SAIL_CHECK_PATH_PTR(path)                       SAIL_CHECK_PTR2(path,            SAIL_PATH_NULL_PTR)
#define SAIL_CHECK_EXTENSION_PTR(extension)             SAIL_CHECK_PTR2(extension,       SAIL_EXTENSION_NULL_PTR)
#define SAIL_CHECK_IMAGE_PTR(image)                     SAIL_CHECK_PTR2(image,           SAIL_IMAGE_NULL_PTR)
#define SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info)         SAIL_CHECK_PTR2(plugin_info,     SAIL_PLUGIN_INFO_NULL_PTR)
#define SAIL_CHECK_PLUGIN_PTR(plugin)                   SAIL_CHECK_PTR2(plugin,          SAIL_PLUGIN_NULL_PTR)
#define SAIL_CHECK_STATE_PTR(state)                     SAIL_CHECK_PTR2(state,           SAIL_STATE_NULL_PTR)
#define SAIL_CHECK_STRING_PTR(str)                      SAIL_CHECK_PTR2(str,             SAIL_STRING_NULL_PTR)
#define SAIL_CHECK_IO_PTR(io)                           SAIL_CHECK_PTR2(io,              SAIL_IO_NULL_PTR)
#define SAIL_CHECK_STREAM_PTR(stream)                   SAIL_CHECK_PTR2(stream,          SAIL_STREAM_NULL_PTR)
#define SAIL_CHECK_BUFFER_PTR(buffer)                   SAIL_CHECK_PTR2(buffer,          SAIL_BUFFER_NULL_PTR)
#define SAIL_CHECK_RESULT_PTR(result)                   SAIL_CHECK_PTR2(result,          SAIL_RESULT_NULL_PTR)
#define SAIL_CHECK_META_ENTRY_NODE_PTR(meta_entry_node) SAIL_CHECK_PTR2(meta_entry_node, SAIL_META_ENTRY_NODE_NULL_PTR)
#define SAIL_CHECK_ICCP_PTR(iccp)                       SAIL_CHECK_PTR2(iccp,            SAIL_ICCP_NULL_PTR)
#define SAIL_CHECK_PALETTE_PTR(palette)                 SAIL_CHECK_PTR2(palette,         SAIL_PALETTE_NULL_PTR)

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
 * Try to execute the specified SAIL function. If it fails, ignore the error and continue execution.
 * Use do/while to require ';' at the end of a SAIL_TRY_OR_SUPPRESS() expression.
 */
#define SAIL_TRY_OR_SUPPRESS(sail_func) \
do {                                    \
    int res;                            \
                                        \
    if ((res = sail_func) != 0) {       \
        (void)0;                        \
    }                                   \
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
