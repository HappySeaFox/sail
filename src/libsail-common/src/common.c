/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "common.h"
#include "log.h"
#include "meta_entry_node.h"

/*
 * File functions.
 */

static int sail_alloc_file_private(struct sail_file **file) {

    *file = (struct sail_file *)malloc(sizeof(struct sail_file));

    if (*file == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*file)->fptr = NULL;
    (*file)->pimpl = NULL;

    return 0;
}

int sail_alloc_file(const char *filepath, const char *mode, struct sail_file **file) {

    /* Try to open the file first */
    FILE *fptr;

#ifdef SAIL_WIN32
    fptr = _fsopen(filepath, mode, _SH_DENYWR);
#else
    /* Fallback to a regular fopen() */
    fptr = fopen(filepath, mode);
#endif

    if (fptr == NULL) {
        return SAIL_FILE_OPEN_ERROR;
    }

    SAIL_TRY(sail_alloc_file_private(file));

    (*file)->fptr = fptr;

    return 0;
}

void sail_destroy_file(struct sail_file *file) {

    if (file == NULL) {
        return;
    }

    if (file->fptr != NULL) {
        fclose(file->fptr);
    }

    free(file->pimpl);
    free(file);
}

/*
 * Image functions.
 */

int sail_alloc_image(struct sail_image **image) {

    *image = (struct sail_image *)malloc(sizeof(struct sail_image));

    if (*image == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*image)->width                = 0;
    (*image)->height               = 0;
    (*image)->bytes_per_line       = 0;
    (*image)->pixel_format         = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->passes               = 0;
    (*image)->animated             = false;
    (*image)->delay                = 0;
    (*image)->palette_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->palette              = NULL;
    (*image)->palette_size         = 0;
    (*image)->meta_entry_node      = NULL;
    (*image)->properties           = 0;
    (*image)->source_pixel_format  = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->source_properties    = 0;

    return 0;
}

void sail_destroy_image(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    free(image->palette);

    sail_destroy_meta_entry_node_chain(image->meta_entry_node);

    free(image);
}

int sail_alloc_read_features(struct sail_read_features **read_features) {

    *read_features = (struct sail_read_features *)malloc(sizeof(struct sail_read_features));

    if (*read_features == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*read_features)->input_pixel_formats           = NULL;
    (*read_features)->input_pixel_formats_length    = 0;
    (*read_features)->output_pixel_formats          = NULL;
    (*read_features)->output_pixel_formats_length   = 0;
    (*read_features)->preferred_output_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*read_features)->features                      = 0;

    return 0;
}

void sail_destroy_read_features(struct sail_read_features *read_features) {

    if (read_features == NULL) {
        return;
    }

    free(read_features->input_pixel_formats);
    free(read_features->output_pixel_formats);
    free(read_features);
}

int sail_alloc_read_options(struct sail_read_options **read_options) {

    *read_options = (struct sail_read_options *)malloc(sizeof(struct sail_read_options));

    if (*read_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*read_options)->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*read_options)->io_options   = 0;

    return 0;
}

void sail_destroy_read_options(struct sail_read_options *read_options) {

    if (read_options == NULL) {
        return;
    }

    free(read_options);
}

int sail_alloc_write_features(struct sail_write_features **write_features) {

    *write_features = (struct sail_write_features *)malloc(sizeof(struct sail_write_features));

    if (*write_features == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*write_features)->input_pixel_formats           = NULL;
    (*write_features)->input_pixel_formats_length    = 0;
    (*write_features)->output_pixel_formats          = NULL;
    (*write_features)->output_pixel_formats_length   = 0;
    (*write_features)->preferred_output_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*write_features)->features                      = 0;
    (*write_features)->properties                    = 0;
    (*write_features)->passes                        = 0;
    (*write_features)->compression_types             = NULL;
    (*write_features)->compression_types_length      = 0;
    (*write_features)->compression_min               = 0;
    (*write_features)->compression_max               = 0;
    (*write_features)->compression_default           = 0;

    return 0;
}

void sail_destroy_write_features(struct sail_write_features *write_features) {

    if (write_features == NULL) {
        return;
    }

    free(write_features->input_pixel_formats);
    free(write_features->output_pixel_formats);
    free(write_features->compression_types);
    free(write_features);
}

int sail_alloc_write_options(struct sail_write_options **write_options) {

    *write_options = (struct sail_write_options *)malloc(sizeof(struct sail_write_options));

    if (*write_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*write_options)->pixel_format     = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*write_options)->io_options       = 0;
    (*write_options)->compression_type = 0;
    (*write_options)->compression      = 0;

    return 0;
}

void sail_destroy_write_options(struct sail_write_options *write_options) {

    if (write_options == NULL) {
        return;
    }

    free(write_options);
}
