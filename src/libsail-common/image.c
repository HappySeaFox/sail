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

sail_status_t sail_alloc_image(struct sail_image **image) {

    SAIL_CHECK_PTR(image);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_image), &ptr));
    *image = ptr;

    (*image)->pixels                  = NULL;
    (*image)->width                   = 0;
    (*image)->height                  = 0;
    (*image)->bytes_per_line          = 0;
    (*image)->resolution              = NULL;
    (*image)->pixel_format            = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*image)->gamma                   = 1;
    (*image)->delay                   = -1;
    (*image)->palette                 = NULL;
    (*image)->meta_data_node          = NULL;
    (*image)->iccp                    = NULL;
    (*image)->orientation             = SAIL_ORIENTATION_NORMAL;
    (*image)->source_image            = NULL;

    return SAIL_OK;
}

void sail_destroy_image(struct sail_image *image) {

    if (image == NULL) {
        return;
    }

    sail_free(image->pixels);

    sail_destroy_resolution(image->resolution);
    sail_destroy_palette(image->palette);
    sail_destroy_meta_data_node_chain(image->meta_data_node);
    sail_destroy_iccp(image->iccp);
    sail_destroy_source_image(image->source_image);

    sail_free(image);
}

sail_status_t sail_copy_image(const struct sail_image *source, struct sail_image **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image_skeleton(source, &image_local));

    /* Pixels. */
    if (source->pixels != NULL) {
        const unsigned pixels_size = source->height * source->bytes_per_line;

        SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                            /* cleanup */ sail_destroy_image(image_local));

        memcpy(image_local->pixels, source->pixels, pixels_size);
    }

    /* Palette. */
    if (source->palette != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_palette(source->palette, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));

    }

    *target = image_local;

    return SAIL_OK;
}

sail_status_t sail_copy_image_skeleton(const struct sail_image *source, struct sail_image **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width                = source->width;
    image_local->height               = source->height;
    image_local->bytes_per_line       = source->bytes_per_line;

    if (source->resolution != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_resolution(source->resolution, &image_local->resolution),
                            /* cleanup */ sail_destroy_image(image_local));

    }

    image_local->pixel_format = source->pixel_format;
    image_local->gamma        = source->gamma;
    image_local->delay        = source->delay;

    SAIL_TRY_OR_CLEANUP(sail_copy_meta_data_node_chain(source->meta_data_node, &image_local->meta_data_node),
                        /* cleanup */ sail_destroy_image(image_local));

    if (source->iccp != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_iccp(source->iccp, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    image_local->orientation = source->orientation;

    if (source->source_image != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_copy_source_image(source->source_image, &image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *target = image_local;

    return SAIL_OK;
}

sail_status_t sail_check_image_skeleton_valid(const struct sail_image *image)
{
    SAIL_CHECK_PTR(image);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_PIXEL_FORMAT);
    }
    if (image->width == 0 || image->height == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS);
    }
    if (image->bytes_per_line == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_BYTES_PER_LINE);
    }

    return SAIL_OK;
}

sail_status_t sail_check_image_valid(const struct sail_image *image)
{
    SAIL_CHECK_PTR(image);

    SAIL_TRY(sail_check_image_skeleton_valid(image));

    if (sail_is_indexed(image->pixel_format)) {
        SAIL_CHECK_PTR(image->palette);
    }

    SAIL_CHECK_PTR(image->pixels);

    return SAIL_OK;
}

sail_status_t sail_flip_vertically(struct sail_image *image) {

    SAIL_TRY(sail_check_image_valid(image));

    void *line;
    SAIL_TRY(sail_malloc(image->bytes_per_line, &line));

    for (unsigned row1 = 0, row2 = image->height - 1; row1 < row2; row1++, row2--) {
        memcpy(line,                                                          (unsigned char *)image->pixels + image->bytes_per_line * row1, image->bytes_per_line);
        memcpy((unsigned char *)image->pixels + image->bytes_per_line * row1, (unsigned char *)image->pixels + image->bytes_per_line * row2, image->bytes_per_line);
        memcpy((unsigned char *)image->pixels + image->bytes_per_line * row2, line,                                                          image->bytes_per_line);
    }

    sail_free(line);

    //image->properties ^= SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY;

    return SAIL_OK;
}

sail_status_t sail_flip_horizontally(struct sail_image *image) {

    SAIL_TRY(sail_check_image_valid(image));

    unsigned bytes_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bytes_per_pixel));

    void *pixel;
    SAIL_TRY(sail_malloc(bytes_per_pixel, &pixel));

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *scan = (unsigned char *)image->pixels + image->bytes_per_line * row;

        for (unsigned col1 = 0, col2 = image->width - bytes_per_pixel; col1 < col2; col1 += bytes_per_pixel, col2 -= bytes_per_pixel) {
            memcpy(pixel,       scan + col1, bytes_per_pixel);
            memcpy(scan + col1, scan + col2, bytes_per_pixel);
            memcpy(scan + col2, pixel,       bytes_per_pixel);
        }
    }

    sail_free(pixel);

    //image->properties ^= SAIL_IMAGE_PROPERTY_FLIPPED_HORIZONTALLY;

    return SAIL_OK;
}
