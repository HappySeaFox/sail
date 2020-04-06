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
sail_error_t SAIL_EXPORT sail_strdup(const char *input, char **output);

/*
 * Duplicates the specified number of bytes of the specified input string and stores
 * a new string in the specified output. Length must be greater than 0.
 *
 * Returns 0 on success or sail_error_t on error.
 */
sail_error_t SAIL_EXPORT sail_strdup_length(const char *input, size_t length, char **output);

/*
 * Returns a non-NULL string representation of the specified pixel format.
 * For example: "RGB", "CMYK".
 */
const char* SAIL_EXPORT sail_pixel_format_to_string(int pixel_format);

/*
 * Returns a number of bits a pixel in the specified pixel format occupies.
 * For example, for SAIL_PIXEL_FORMAT_RGB 24 is returned.
 */
int SAIL_EXPORT sail_bits_per_pixel(int pixel_format);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
