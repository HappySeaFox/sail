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
#define SAIL_INVALID_ARGUMENT             3

/*
 * Plugins-related errors.
 */
#define SAIL_ERROR_READ_PLUGIN_INFO       20

/*
 * Encoding/decoding common errors.
 */
#define SAIL_FILE_NULL_PTR                41
#define SAIL_IMAGE_NULL_PTR               42
#define SAIL_SCAN_LINE_NULL_PTR           43

/*
 * Encoding/decoding specific errors.
 */
#define SAIL_INCORRECT_IMAGE_DIMENSIONS   60
#define SAIL_UNSUPPORTED_PIXEL_FORMAT     61
#define SAIL_UNSUPPORTED_COMPRESSION_TYPE 62
#define SAIL_UNDERLYING_CODEC_ERROR       63

/*
 * Helper macros.
 */
#define SAIL_CHECK_FILE(file)      \
    if (file == NULL) {            \
        return SAIL_FILE_NULL_PTR; \
    }                              \
    if (file->fptr == NULL) {      \
        return SAIL_FILE_NULL_PTR; \
    }

#define SAIL_CHECK_IMAGE(image)                    \
    if (image == NULL) {                           \
        return SAIL_IMAGE_NULL_PTR;                \
    }                                              \
    if (image->width <= 0 || image->height <= 0) { \
        return SAIL_INCORRECT_IMAGE_DIMENSIONS;    \
    }

#define SAIL_CHECK_SCAN_LINE(scan)      \
    if (scan == NULL) {                 \
        return SAIL_SCAN_LINE_NULL_PTR; \
    }

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
