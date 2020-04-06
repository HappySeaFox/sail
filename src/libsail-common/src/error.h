#ifndef SAIL_ERROR_H
#define SAIL_ERROR_H

/*
 * Error codes before this threshold are reserved for errno codes.
 */
#define SAIL_ERROR_THRESHOLD 1024

typedef int sail_error_t;

/*
 * SAIL error codes. TODO.
 */
#define SAIL_FILE_PTR_NULL                (SAIL_ERROR_THRESHOLD+1)
#define SAIL_IMAGE_PTR_NULL               (SAIL_ERROR_THRESHOLD+2)
#define SAIL_SCAN_LINE_PTR_NULL           (SAIL_ERROR_THRESHOLD+3)
#define SAIL_INCORRECT_IMAGE_DIMENSIONS   (SAIL_ERROR_THRESHOLD+4)
#define SAIL_ERROR_READ_PLUGIN_INFO       (SAIL_ERROR_THRESHOLD+5)
#define SAIL_UNSUPPORTED_PIXEL_FORMAT     (SAIL_ERROR_THRESHOLD+6)
#define SAIL_UNSUPPORTED_COMPRESSION_TYPE (SAIL_ERROR_THRESHOLD+7)
#define SAIL_MEMORY_ALLOCATION_FAILED     (SAIL_ERROR_THRESHOLD+8)
#define SAIL_INVALID_ARGUMENT             (SAIL_ERROR_THRESHOLD+9)
#define SAIL_UNDERLYING_CODEC_ERROR       (SAIL_ERROR_THRESHOLD+10)
#define SAIL_FILE_OPEN_ERROR              (SAIL_ERROR_THRESHOLD+11)

/*
 * Helper macros.
 */
#define SAIL_CHECK_FILE(file)      \
    if (file == NULL) {            \
        return SAIL_FILE_PTR_NULL; \
    }                              \
    if (file->fptr == NULL) {      \
        return SAIL_FILE_PTR_NULL; \
    }

#define SAIL_CHECK_IMAGE(image)                    \
    if (image == NULL) {                           \
        return SAIL_IMAGE_PTR_NULL;                \
    }                                              \
    if (image->width <= 0 || image->height <= 0) { \
        return SAIL_INCORRECT_IMAGE_DIMENSIONS;    \
    }

#define SAIL_CHECK_SCAN_LINE(scan)      \
    if (scan == NULL) {                 \
        return SAIL_SCAN_LINE_PTR_NULL; \
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
