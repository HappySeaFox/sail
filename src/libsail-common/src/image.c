/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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
    (*image)->palette_pixel_format    = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->palette                 = NULL;
    (*image)->palette_color_count     = 0;
    (*image)->meta_entry_node         = NULL;
    (*image)->properties              = 0;
    (*image)->source_pixel_format     = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->source_properties       = 0;
    (*image)->source_compression_type = SAIL_COMPRESSION_UNSUPPORTED;
    (*image)->iccp                    = NULL;

    return 0;
}

void sail_destroy_image(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    free(image->palette);

    sail_destroy_meta_entry_node_chain(image->meta_entry_node);
    sail_destroy_iccp(image->iccp);

    free(image);
}

sail_error_t sail_copy_image(const struct sail_image *source_image, struct sail_image **target_image) {

    SAIL_CHECK_IMAGE_PTR(source_image);
    SAIL_CHECK_IMAGE_PTR(target_image);

    SAIL_TRY(sail_alloc_image(target_image));

    (*target_image)->width                = source_image->width;
    (*target_image)->height               = source_image->height;
    (*target_image)->bytes_per_line       = source_image->bytes_per_line;
    (*target_image)->pixel_format         = source_image->pixel_format;
    (*target_image)->interlaced_passes    = source_image->interlaced_passes;
    (*target_image)->animated             = source_image->animated;
    (*target_image)->delay                = source_image->delay;
    (*target_image)->palette_pixel_format = source_image->palette_pixel_format;
    (*target_image)->palette              = NULL;
    (*target_image)->palette_color_count  = source_image->palette_color_count;

    if (source_image->palette != NULL) {
        unsigned bits_per_pixel;
        SAIL_TRY_OR_CLEANUP(sail_bits_per_pixel(source_image->palette_pixel_format, &bits_per_pixel),
                            /* cleanup */ sail_destroy_image(*target_image));

        unsigned palette_size = source_image->palette_color_count * bits_per_pixel / 8;
        (*target_image)->palette = malloc(palette_size);

        if ((*target_image)->palette == NULL) {
            sail_destroy_image(*target_image);
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        memcpy((*target_image)->palette, source_image->palette, palette_size);
    }

    SAIL_TRY_OR_CLEANUP(sail_copy_meta_entry_node_chain(source_image->meta_entry_node, &(*target_image)->meta_entry_node),
                        /* cleanup */ sail_destroy_image(*target_image));

    (*target_image)->properties              = source_image->properties;
    (*target_image)->source_pixel_format     = source_image->source_pixel_format;
    (*target_image)->source_properties       = source_image->source_properties;
    (*target_image)->source_compression_type = source_image->source_compression_type;

    if (source_image->iccp != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_iccp(source_image->iccp, &(*target_image)->iccp),
                            /* cleanup */ sail_destroy_image(*target_image));
    }

    return 0;
}
