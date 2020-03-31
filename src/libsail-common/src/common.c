#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "common.h"
#include "meta_entry_node.h"

/*
 * File functions.
 */

static int sail_file_alloc_private(struct sail_file **file) {

    *file = (struct sail_file *)malloc(sizeof(struct sail_file));

    if (*file == NULL) {
        return ENOMEM;
    }

    (*file)->fptr = NULL;
    (*file)->pimpl = NULL;

    return 0;
}

int sail_file_alloc(const char *filepath, const char *mode, struct sail_file **file) {

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

    int res;

    if ((res = sail_file_alloc_private(file)) != 0) {
        return res;
    }

    (*file)->fptr = fptr;

    return 0;
}

void sail_file_destroy(struct sail_file *file) {

    if (file == NULL) {
        return;
    }

    if (file->fptr != NULL) {
        fclose(file->fptr);
    }

    if (file->pimpl != NULL) {
        free(file->pimpl);
    }

    free(file);
}

/*
 * Image functions.
 */

int sail_image_alloc(struct sail_image **image) {

    *image = (struct sail_image *)malloc(sizeof(struct sail_image));

    if (*image == NULL) {
        return ENOMEM;
    }

    (*image)->width                = 0;
    (*image)->height               = 0;
    (*image)->pixel_format         = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->passes               = 0;
    (*image)->animated             = false;
    (*image)->delay                = 0;
    (*image)->palette_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->palette              = NULL;
    (*image)->palette_size         = 0;
    (*image)->meta_entry_node      = NULL;
    (*image)->source_pixel_format  = SAIL_PIXEL_FORMAT_UNKNOWN;

    return 0;
}

void sail_image_destroy(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    if (image->palette != NULL) {
        free(image->palette);
    }

    sail_destroy_meta_entry_node_chain(image->meta_entry_node);

    free(image);
}

struct sail_read_options sail_default_read_options() {

    struct sail_read_options read_options;

    read_options.pixel_format = SAIL_PIXEL_FORMAT_RGB;
    read_options.meta_info = true;

    return read_options;
}
