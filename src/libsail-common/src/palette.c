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

sail_error_t sail_alloc_palette(struct sail_palette **palette) {

    *palette = (struct sail_palette *)malloc(sizeof(struct sail_palette));

    if (*palette == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*palette)->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*palette)->data         = NULL;
    (*palette)->color_count  = 0;

    return 0;
}

void sail_destroy_palette(struct sail_palette *palette) {

    if (palette == NULL) {
        return;
    }

    free(palette->data);
    free(palette);
}

sail_error_t sail_copy_palette(const struct sail_palette *source_palette, struct sail_palette **target_palette) {

    SAIL_CHECK_ICCP_PTR(source_palette);
    SAIL_CHECK_ICCP_PTR(target_palette);

    SAIL_TRY(sail_alloc_palette(target_palette));

    unsigned bits_per_pixel;
    SAIL_TRY_OR_CLEANUP(sail_bits_per_pixel(source_palette->pixel_format, &bits_per_pixel),
                        /* cleanup */ sail_destroy_palette(*target_palette));

    unsigned palette_size = source_palette->color_count * bits_per_pixel / 8;
    (*target_palette)->data = malloc(palette_size);

    if ((*target_palette)->data == NULL) {
        sail_destroy_palette(*target_palette);
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*target_palette)->pixel_format = source_palette->pixel_format;
    (*target_palette)->color_count  = source_palette->color_count;

    memcpy((*target_palette)->data, source_palette->data, palette_size);

    return 0;
}
