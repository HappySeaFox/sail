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

sail_status_t sail_alloc_palette(struct sail_palette **palette) {

    SAIL_CHECK_PTR(palette);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_palette), &ptr));
    *palette = ptr;

    (*palette)->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*palette)->data         = NULL;
    (*palette)->color_count  = 0;

    return SAIL_OK;
}

void sail_destroy_palette(struct sail_palette *palette) {

    if (palette == NULL) {
        return;
    }

    sail_free(palette->data);
    sail_free(palette);
}

sail_status_t sail_copy_palette(const struct sail_palette *source_palette, struct sail_palette **target_palette) {

    SAIL_CHECK_PTR(source_palette);
    SAIL_CHECK_PTR(target_palette);

    struct sail_palette *palette_local;
    SAIL_TRY(sail_alloc_palette(&palette_local));

    unsigned bits_per_pixel;
    SAIL_TRY_OR_CLEANUP(sail_bits_per_pixel(source_palette->pixel_format, &bits_per_pixel),
                        /* cleanup */ sail_destroy_palette(palette_local));

    unsigned palette_size = source_palette->color_count * bits_per_pixel / 8;

    SAIL_TRY_OR_CLEANUP(sail_malloc(palette_size, &palette_local->data),
                        /* cleanup */ sail_destroy_palette(palette_local));

    palette_local->pixel_format = source_palette->pixel_format;
    palette_local->color_count  = source_palette->color_count;

    memcpy(palette_local->data, source_palette->data, palette_size);

    *target_palette = palette_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_palette_for_data(enum SailPixelFormat pixel_format, unsigned color_count, struct sail_palette **palette) {

    SAIL_CHECK_PTR(palette);

    struct sail_palette *palette_local;
    SAIL_TRY(sail_alloc_palette(&palette_local));

    palette_local->pixel_format = pixel_format;
    palette_local->color_count = color_count;

    unsigned palette_size;
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(color_count, pixel_format, &palette_size),
                        /* cleanup */ sail_destroy_palette(palette_local));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(palette_size, &ptr),
                        /* cleanup */ sail_destroy_palette(palette_local));
    palette_local->data = ptr;

    *palette = palette_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_palette_from_data(enum SailPixelFormat pixel_format, const void *data, unsigned color_count, struct sail_palette **palette) {

    SAIL_CHECK_PTR(palette);

    struct sail_palette *palette_local;
    SAIL_TRY(sail_alloc_palette_for_data(pixel_format, color_count, &palette_local));

    unsigned palette_size;
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(color_count, pixel_format, &palette_size),
                        /* cleanup */ sail_destroy_palette(palette_local));

    memcpy(palette_local->data, data, palette_size);

    *palette = palette_local;

    return SAIL_OK;
}
