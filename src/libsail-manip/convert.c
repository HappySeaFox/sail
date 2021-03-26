/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include <stdint.h>
#include <stdlib.h>

#include "sail-common.h"

#include "sail-manip.h"

/*
 * Private functions.
 */

static sail_status_t get_palette_rgba32_color(const sail_rgba8_t *palette, unsigned palette_count, unsigned index, sail_rgba8_t *color) {

    if (index >= palette_count) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    *color = palette[index];

    return SAIL_OK;
}

static void fill_scan_from_values(uint16_t *scan, int r, int g, int b, int a, uint16_t rv, uint16_t gv, uint16_t bv, uint16_t av) {

    *(scan+r) = rv;
    *(scan+g) = gv;
    *(scan+b) = bv;
    if (a >= 0) {
        *(scan+a) = av;
    }
}
/*
 * Public functions.
 */

sail_status_t sail_convert_image_to_bpp64_rgba_kind(const struct sail_image *image_input, enum SailPixelFormat output_pixel_format, struct sail_image **image_output) {

    SAIL_CHECK_IMAGE(image_input);
    SAIL_CHECK_IMAGE_PTR(image_output);

    int r, g, b, a;

    switch (output_pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP64_RGBX: { r = 0; g = 1; b = 2; a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_BGRX: { r = 2; g = 1; b = 0; a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_XRGB: { r = 1; g = 2; b = 3; a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_XBGR: { r = 3; g = 2; b = 1; a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: { r = 0; g = 1; b = 2; a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_BGRA: { r = 2; g = 1; b = 0; a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_ARGB: { r = 1; g = 2; b = 3; a = 0;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: { r = 3; g = 2; b = 1; a = 0;  break; }
        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image_skeleton(image_input, &image_local));

    /* Setup new properties. */
    image_local->pixel_format = output_pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    const unsigned pixels_size = image_local->height * image_local->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Convert image. */
    for (unsigned i = 0; i < image_input->height; i++) {
        uint16_t *scan_output = (uint16_t *)((uint8_t *)image_local->pixels + image_local->bytes_per_line * i);

        switch (image_input->pixel_format) {
            case SAIL_PIXEL_FORMAT_BPP24_RGB: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * i;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    fill_scan_from_values(scan_output, r, g, b, a, *(scan_input+0) * 255, *(scan_input+1) * 255, *(scan_input+2) * 255, 255 * 255);
                    scan_input += 3;
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP24_BGR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * i;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    fill_scan_from_values(scan_output, r, g, b, a, *(scan_input+2) * 255, *(scan_input+1) * 255, *(scan_input+0) * 255, 255 * 255);
                    scan_input += 3;
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP48_RGB: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * i);

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    fill_scan_from_values(scan_output, r, g, b, a, *(scan_input+0), *(scan_input+1), *(scan_input+2), 255 * 255);
                    scan_input += 3;
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP48_BGR: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * i);

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    fill_scan_from_values(scan_output, r, g, b, a, *(scan_input+2) * 255, *(scan_input+1) * 255, *(scan_input+0) * 255, 255 * 255);
                    scan_input += 3;
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_CMYK: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * i;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint8_t C =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t M =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t Y =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t K =  (uint8_t)(*scan_input++ / 100.0);

                    fill_scan_from_values(scan_output, r, g, b, a,
                                            (uint16_t)((1-C) * (1-K) * 255 * 255), (uint16_t)((1-M) * (1-K) * 255 * 255),
                                            (uint16_t)((1-Y) * (1-K) * 255 * 255), 255 * 255);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP24_YCBCR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * i;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const double Y  = *scan_input++;
                    const double Cb = *scan_input++;
                    const double Cr = *scan_input++;

                    const int rv = (int)(Y                        + 1.402   * (Cr - 128));
                    const int gv = (int)(Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128));
                    const int bv = (int)(Y + 1.772   * (Cb - 128));

                    fill_scan_from_values(scan_output, r, g, b, a,
                                            (uint16_t)(max(0, min(255, rv)) * 255), (uint16_t)(max(0, min(255, gv)) * 255),
                                            (uint16_t)(max(0, min(255, bv)) * 255), 255 * 255);
                    scan_output += 4;
                }
                break;
            }
            default: {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }
    }

    *image_output = image_local;

    return SAIL_OK;
}
