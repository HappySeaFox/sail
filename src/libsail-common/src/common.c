#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"

/*
 * File functions.
 */

int sail_file_open(const char *filepath, int flags, struct sail_file **file) {

    *file = (struct sail_file *)malloc(sizeof(struct sail_file));

    (*file)->fd = 0;
    (*file)->pimpl = NULL;
    (*file)->pimpl_destroy = NULL;

    return 0;
}

void sail_file_close(struct sail_file *file) {
}

/*
 * Image functions.
 */

void sail_image_destroy(struct sail_image *image) {
}
