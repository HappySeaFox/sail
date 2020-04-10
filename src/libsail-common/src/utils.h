#ifndef SAIL_UTILS_H
#define SAIL_UTILS_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

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
 * Returns a non-NULL string representation of the specified pixel format.
 * For example: "RGB", "CMYK".
 */
SAIL_EXPORT const char* sail_pixel_format_to_string(int pixel_format);

/*
 * Returns a number of bits a pixel in the specified pixel format occupies.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is returned.
 */
SAIL_EXPORT int sail_bits_per_pixel(int pixel_format);

/*
 * Converts the specified string to a lower case.
 */
SAIL_EXPORT void sail_to_lower(char *str);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
