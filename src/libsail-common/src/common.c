#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "common.h"

/*
 * File functions.
 */

int sail_file_open(const char *filepath, const char *mode, struct sail_file **file) {

    /* Try to open the file first */
    FILE *fptr;

#ifdef SAIL_WIN32
    fptr = _fsopen(filepath, mode, _SH_DENYWR);
#else
    /* Fallback to a regular fopen() */
    fptr = fopen(filepath, mode);
#endif

    if (fptr == NULL) {
        return errno;
    }

    *file = (struct sail_file *)malloc(sizeof(struct sail_file));

    (*file)->fptr = fptr;
    (*file)->pimpl = NULL;

    return 0;
}

void sail_file_close(struct sail_file *file) {

    if (file == NULL) {
        return;
    }

    if (file->fptr != NULL) {
        fclose(file->fptr);
    }

    free(file);
}

/*
 * Image functions.
 */

void sail_image_destroy(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    if (image->palette != NULL) {
        free(image->palette);
    }

    free(image);
}
