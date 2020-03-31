#ifndef SAIL_UTILS_H
#define SAIL_UTILS_H

#ifdef SAIL_BUILD
    #include "export.h"
#else
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Duplicates the specified string and stores a new string in the specified output.
 *
 * Returns 0 on success or errno on error.
 */
int SAIL_EXPORT sail_strdup(char *input, char **output);

/*
 * Duplicates the specified number of bytes of the specified input string and stores
 * a new string in the specified output. Length must be greater than 0.
 *
 * Returns 0 on success or errno on error.
 */
int SAIL_EXPORT sail_strdup_length(char *input, size_t length, char **output);

/*
 * Returns a non-NULL string representation of the specified pixel format.
 * For example: "RGB", "CMYK".
 */
const char* SAIL_EXPORT sail_pixel_format_to_string(int pixel_format);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
