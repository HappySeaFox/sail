/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_error_t sail_alloc_image(struct sail_image **image) {

    SAIL_CHECK_IMAGE_PTR(image);

    *image = (struct sail_image *)malloc(sizeof(struct sail_image));

    if (*image == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*image)->width                   = 0;
    (*image)->height                  = 0;
    (*image)->bytes_per_line          = 0;
    (*image)->pixel_format            = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->interlaced_passes       = 0;
    (*image)->animated                = false;
    (*image)->delay                   = 0;
    (*image)->palette                 = NULL;
    (*image)->meta_entry_node         = NULL;
    (*image)->iccp                    = NULL;
    (*image)->properties              = 0;

    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&(*image)->source_image),
                        /* cleanup */ sail_destroy_image(*image));

    return 0;
}

void sail_destroy_image(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    sail_destroy_palette(image->palette);
    sail_destroy_meta_entry_node_chain(image->meta_entry_node);
    sail_destroy_iccp(image->iccp);
    sail_destroy_source_image(image->source_image);

    free(image);
}

sail_error_t sail_copy_image(const struct sail_image *source, struct sail_image **target) {

    SAIL_CHECK_IMAGE_PTR(source);
    SAIL_CHECK_IMAGE_PTR(target);

    SAIL_TRY(sail_alloc_image(target));

    (*target)->width                = source->width;
    (*target)->height               = source->height;
    (*target)->bytes_per_line       = source->bytes_per_line;
    (*target)->pixel_format         = source->pixel_format;
    (*target)->interlaced_passes    = source->interlaced_passes;
    (*target)->animated             = source->animated;
    (*target)->delay                = source->delay;

    if (source->palette != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_palette(source->palette, &(*target)->palette),
                            /* cleanup */ sail_destroy_image(*target));

    }

    SAIL_TRY_OR_CLEANUP(sail_copy_meta_entry_node_chain(source->meta_entry_node, &(*target)->meta_entry_node),
                        /* cleanup */ sail_destroy_image(*target));

    if (source->iccp != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_iccp(source->iccp, &(*target)->iccp),
                            /* cleanup */ sail_destroy_image(*target));
    }

    (*target)->properties = source->properties;

    SAIL_TRY_OR_CLEANUP(sail_copy_source_image(source->source_image, &(*target)->source_image),
                        /* cleanup */ sail_destroy_image(*target));

    return 0;
}
