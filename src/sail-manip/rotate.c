/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "rotate.h"

/*
 * Private functions.
 */

static sail_status_t rotate_90_clockwise(const struct sail_image* image, struct sail_image* output)
{
    SAIL_TRY(sail_check_image_valid(image));
    SAIL_CHECK_PTR(output);

    const unsigned bits_per_pixel = sail_bits_per_pixel(image->pixel_format);

    if (bits_per_pixel % 8 != 0)
    {
        SAIL_LOG_ERROR("Only byte-aligned pixels are supported for rotation");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const unsigned bytes_per_pixel = bits_per_pixel / 8;
    const unsigned src_width = image->width;
    const unsigned src_height = image->height;

    /* For 90° CW rotation: new[x][y] = old[height-1-y][x] */
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < src_height; row++)
    {
        const uint8_t* src_scan = (const uint8_t*)sail_scan_line(image, row);
        const unsigned dst_col = src_height - 1 - row;

        for (unsigned col = 0; col < src_width; col++)
        {
            const unsigned dst_row = col;
            uint8_t* dst_pixel = (uint8_t*)sail_scan_line(output, dst_row) + dst_col * bytes_per_pixel;
            const uint8_t* src_pixel = src_scan + col * bytes_per_pixel;

            memcpy(dst_pixel, src_pixel, bytes_per_pixel);
        }
    }

    return SAIL_OK;
}

static sail_status_t rotate_180(const struct sail_image* image, struct sail_image* output)
{
    SAIL_TRY(sail_check_image_valid(image));
    SAIL_CHECK_PTR(output);

    const unsigned bits_per_pixel = sail_bits_per_pixel(image->pixel_format);

    if (bits_per_pixel % 8 != 0)
    {
        SAIL_LOG_ERROR("Only byte-aligned pixels are supported for rotation");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const unsigned bytes_per_pixel = bits_per_pixel / 8;
    const unsigned width = image->width;
    const unsigned height = image->height;

    /* For 180° rotation: new[x][y] = old[width-1-x][height-1-y] */
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < height; row++)
    {
        const uint8_t* src_scan = (const uint8_t*)sail_scan_line(image, row);
        uint8_t* dst_scan = (uint8_t*)sail_scan_line(output, height - 1 - row);

        /* Copy pixels in reverse order */
        for (unsigned col = 0; col < width; col++)
        {
            const uint8_t* src_pixel = src_scan + col * bytes_per_pixel;
            uint8_t* dst_pixel = dst_scan + (width - 1 - col) * bytes_per_pixel;

            memcpy(dst_pixel, src_pixel, bytes_per_pixel);
        }
    }

    return SAIL_OK;
}

static sail_status_t rotate_270_clockwise(const struct sail_image* image, struct sail_image* output)
{
    SAIL_TRY(sail_check_image_valid(image));
    SAIL_CHECK_PTR(output);

    const unsigned bits_per_pixel = sail_bits_per_pixel(image->pixel_format);

    if (bits_per_pixel % 8 != 0)
    {
        SAIL_LOG_ERROR("Only byte-aligned pixels are supported for rotation");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const unsigned bytes_per_pixel = bits_per_pixel / 8;
    const unsigned src_width = image->width;
    const unsigned src_height = image->height;

    /* For 270° CW rotation: new[x][y] = old[y][width-1-x] */
    unsigned row;

#pragma omp parallel for schedule(SAIL_OPENMP_SCHEDULE)
    for (row = 0; row < src_height; row++)
    {
        const uint8_t* src_scan = (const uint8_t*)sail_scan_line(image, row);
        const unsigned dst_col = row;

        for (unsigned col = 0; col < src_width; col++)
        {
            const unsigned dst_row = src_width - 1 - col;
            uint8_t* dst_pixel = (uint8_t*)sail_scan_line(output, dst_row) + dst_col * bytes_per_pixel;
            const uint8_t* src_pixel = src_scan + col * bytes_per_pixel;

            memcpy(dst_pixel, src_pixel, bytes_per_pixel);
        }
    }

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_rotate_image(const struct sail_image* image,
                                enum SailOrientation angle,
                                struct sail_image** image_output)
{
    SAIL_TRY(sail_check_image_valid(image));
    SAIL_CHECK_PTR(image_output);

    struct sail_image* output = NULL;
    unsigned new_width, new_height;

    /* Determine output dimensions */
    switch (angle)
    {
    case SAIL_ORIENTATION_ROTATED_90:
    case SAIL_ORIENTATION_ROTATED_270:
        /* Swap dimensions for 90° and 270° */
        new_width = image->height;
        new_height = image->width;
        break;

    case SAIL_ORIENTATION_ROTATED_180:
        /* Keep dimensions for 180° */
        new_width = image->width;
        new_height = image->height;
        break;

    default:
        SAIL_LOG_ERROR("Unsupported rotation angle. Use SAIL_ORIENTATION_ROTATED_90, "
                      "SAIL_ORIENTATION_ROTATED_180, or SAIL_ORIENTATION_ROTATED_270");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* Create output image with appropriate dimensions */
    SAIL_TRY(sail_alloc_image(&output));

    output->width = new_width;
    output->height = new_height;
    output->pixel_format = image->pixel_format;
    output->bytes_per_line = sail_bytes_per_line(new_width, image->pixel_format);

    /* Allocate pixels */
    const size_t pixels_size = (size_t)output->height * output->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &output->pixels),
                       /* cleanup */ sail_destroy_image(output));

    /* Copy metadata */
    if (image->palette != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_palette(image->palette, &output->palette),
                           /* cleanup */ sail_destroy_image(output));
    }

    if (image->resolution != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_resolution(image->resolution, &output->resolution),
                           /* cleanup */ sail_destroy_image(output));
    }

    if (image->iccp != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_iccp(image->iccp, &output->iccp),
                           /* cleanup */ sail_destroy_image(output));
    }

    if (image->meta_data_node != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_meta_data_node(image->meta_data_node, &output->meta_data_node),
                           /* cleanup */ sail_destroy_image(output));
    }

    /* Perform rotation */
    sail_status_t status;

    switch (angle)
    {
    case SAIL_ORIENTATION_ROTATED_90:
        status = rotate_90_clockwise(image, output);
        break;

    case SAIL_ORIENTATION_ROTATED_180:
        status = rotate_180(image, output);
        break;

    case SAIL_ORIENTATION_ROTATED_270:
        status = rotate_270_clockwise(image, output);
        break;

    default:
        status = SAIL_ERROR_INVALID_ARGUMENT;
        break;
    }

    if (status != SAIL_OK)
    {
        sail_destroy_image(output);
        return status;
    }

    *image_output = output;

    return SAIL_OK;
}

sail_status_t sail_rotate_image_180_inplace(struct sail_image* image)
{
    SAIL_TRY(sail_check_image_valid(image));

    const unsigned bits_per_pixel = sail_bits_per_pixel(image->pixel_format);

    if (bits_per_pixel % 8 != 0)
    {
        SAIL_LOG_ERROR("Only byte-aligned pixels are supported for rotation");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const unsigned bytes_per_pixel = bits_per_pixel / 8;
    const unsigned width = image->width;
    const unsigned height = image->height;
    const unsigned total_pixels = width * height;
    const unsigned half_pixels = total_pixels / 2;

    uint8_t* pixels = (uint8_t*)image->pixels;

    /* Allocate temporary buffer for pixel swap */
    void* temp_pixel;
    SAIL_TRY(sail_malloc(bytes_per_pixel, &temp_pixel));

    /* Swap pixels from opposite ends moving toward center */
    for (unsigned i = 0; i < half_pixels; i++)
    {
        const unsigned opposite_i = total_pixels - 1 - i;

        uint8_t* pixel1 = pixels + i * bytes_per_pixel;
        uint8_t* pixel2 = pixels + opposite_i * bytes_per_pixel;

        /* Swap pixels */
        memcpy(temp_pixel, pixel1, bytes_per_pixel);
        memcpy(pixel1, pixel2, bytes_per_pixel);
        memcpy(pixel2, temp_pixel, bytes_per_pixel);
    }

    sail_free(temp_pixel);

    return SAIL_OK;
}

