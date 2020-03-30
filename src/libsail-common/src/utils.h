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
 * a new string in the specified output. If length is -1, the entire string is duplicated.
 *
 * Returns 0 on success or errno on error.
 */
int SAIL_EXPORT sail_strdup_length(char *input, int length, char **output);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
