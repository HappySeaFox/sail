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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "sail-common.h"

#include "sail-manip.h"

/*
 * Private functions.
 */

static sail_status_t verify_and_construct_rgba32_indexes(enum SailPixelFormat output_pixel_format, int *r, int *g, int *b, int *a) {

    switch (output_pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP32_RGBX: { *r = 0; *g = 1; *b = 2; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_BGRX: { *r = 2; *g = 1; *b = 0; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_XRGB: { *r = 1; *g = 2; *b = 3; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_XBGR: { *r = 3; *g = 2; *b = 1; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: { *r = 0; *g = 1; *b = 2; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: { *r = 2; *g = 1; *b = 0; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB: { *r = 1; *g = 2; *b = 3; *a = 0;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: { *r = 3; *g = 2; *b = 1; *a = 0;  break; }
        default: {
            const char *pixel_format_str = NULL;
            SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(output_pixel_format, &pixel_format_str));
            SAIL_LOG_ERROR("Conversion to %s is not supported by this function, use BPP32-RGBA-like output pixel formats instead", pixel_format_str);

            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    return SAIL_OK;
}

static sail_status_t get_palette_rgba32(const struct sail_palette *palette, unsigned index, sail_rgba32_t *color) {

    if (index >= palette->color_count) {
        SAIL_LOG_ERROR("Palette index %u is out of range [0; %u)", index, palette->color_count);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    switch (palette->pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            const uint8_t *entry = (uint8_t *)palette->data + index * 3;

            color->component1 = *(entry+0);
            color->component2 = *(entry+1);
            color->component3 = *(entry+2);
            color->component4 = 255;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            const uint8_t *entry = (uint8_t *)palette->data + index * 4;

            color->component1 = *(entry+0);
            color->component2 = *(entry+1);
            color->component3 = *(entry+2);
            color->component4 = *(entry+3);
            break;
        }
        default: {
            const char *pixel_format_str = NULL;
            SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(palette->pixel_format, &pixel_format_str));
            SAIL_LOG_ERROR("Palette pixel format %s is not currently supported", pixel_format_str);
        }
    }

    return SAIL_OK;
}

static void spread_gray8_to_rgba32(uint8_t value, sail_rgba32_t *color) {

    color->component1 = color->component2 = color->component3 = value;
    color->component4 = 255;
}

static void spread_gray16_to_rgba32(uint16_t value, sail_rgba32_t *color) {

    color->component1 = color->component2 = color->component3 = (uint8_t)(value / 257.0);
    color->component4 = 255;
}

static void fill_rgba32_pixel(uint8_t *scan, int r, int g, int b, int a, uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av) {

    *(scan+r) = rv;
    *(scan+g) = gv;
    *(scan+b) = bv;
    if (a >= 0) {
        *(scan+a) = av;
    }
}

static void fill_rgba32_scan_from_rgb24_kind(const uint8_t **scan_input, unsigned width, int ri, int gi, int bi, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, *(*scan_input+ri), *(*scan_input+gi), *(*scan_input+bi), 255);
        *scan_input += 3;
        *scan_output += 4;
    }
}

static void fill_rgba32_scan_from_rgba32_kind(const uint8_t **scan_input, unsigned width, int ri, int gi, int bi, int ai, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, *(*scan_input+ri), *(*scan_input+gi), *(*scan_input+bi), *(*scan_input+ai));
        *scan_input += 4;
        *scan_output += 4;
    }
}

static void fill_rgba32_scan_from_rgbx32_kind(const uint8_t **scan_input, unsigned width, int ri, int gi, int bi, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, *(*scan_input+ri), *(*scan_input+gi), *(*scan_input+bi), 255);
        *scan_input += 4;
        *scan_output += 4;
    }
}

static void fill_rgba32_scan_from_rgb48_kind(const uint16_t **scan_input, unsigned width, int ri, int gi, int bi, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, (uint8_t)(*(*scan_input+ri) / 257.0), (uint8_t)(*(*scan_input+gi) / 257.0), (uint8_t)(*(*scan_input+bi) / 257.0), 255);
        *scan_input += 3;
        *scan_output += 4;
    }
}

static void fill_rgba32_scan_from_rgba64_kind(const uint16_t **scan_input, unsigned width, int ri, int gi, int bi, int ai, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, (uint8_t)(*(*scan_input+ri) / 257.0), (uint8_t)(*(*scan_input+gi) / 257.0), (uint8_t)(*(*scan_input+bi) / 257.0), (uint8_t)(*(*scan_input+ai) / 257.0));
        *scan_input += 4;
        *scan_output += 4;
    }
}

static void fill_rgba32_scan_from_rgbx64_kind(const uint16_t **scan_input, unsigned width, int ri, int gi, int bi, uint8_t **scan_output, int r, int g, int b, int a) {

    for (unsigned pixel_index = 0; pixel_index < width; pixel_index++) {
        fill_rgba32_pixel(*scan_output, r, g, b, a, (uint8_t)(*(*scan_input+ri) / 257.0), (uint8_t)(*(*scan_input+gi) / 257.0), (uint8_t)(*(*scan_input+bi) / 257.0), 255);
        *scan_input += 4;
        *scan_output += 4;
    }
}

static sail_status_t to_bpp32_rgba_kind(const struct sail_image *image_input, int r, int g, int b, int a, struct sail_image *image_output) {

    sail_rgba32_t rgba;

    /* Convert image. */
    for (unsigned row = 0; row < image_input->height; row++) {
        uint8_t *scan_output = (uint8_t *)image_output->pixels + image_output->bytes_per_line * row;

        switch (image_input->pixel_format) {
            case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
            case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width;) {
                    unsigned bit_shift = 7;
                    unsigned bit_mask = 1 << 7;
                    const uint8_t byte = *scan_input++;

                    while (bit_mask > 0 && pixel_index < image_input->width) {
                        const uint8_t index = (byte & bit_mask) >> bit_shift;

                        if (image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED) {
                            SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba));
                        } else {
                            spread_gray8_to_rgba32(index == 0 ? 0 : 255, &rgba);
                        }

                        fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                        scan_output += 4;

                        bit_shift--;
                        bit_mask >>= 1;
                        pixel_index++;
                    }
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
            case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width;) {
                    unsigned bit_shift = 6;
                    unsigned bit_mask = 3 << 6; /* 11000000 */
                    const uint8_t byte = *scan_input++;

                    while (bit_mask > 0 && pixel_index < image_input->width) {
                        const uint8_t index = (byte & bit_mask) >> bit_shift;

                        if (image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED) {
                            SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba));
                        } else {
                            spread_gray8_to_rgba32(index * 85, &rgba);
                        }

                        fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                        scan_output += 4;

                        bit_shift -= 2;
                        bit_mask >>= 2;
                        pixel_index++;
                    }
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
            case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width;) {
                    unsigned bit_shift = 4;
                    unsigned bit_mask = 15 << 4; /* 11110000 */
                    const uint8_t byte = *scan_input++;

                    while (bit_mask > 0 && pixel_index < image_input->width) {
                        const uint8_t index = (byte & bit_mask) >> bit_shift;

                        if (image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED) {
                            SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba));
                        } else {
                            spread_gray8_to_rgba32(index * 17, &rgba);
                        }

                        fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                        scan_output += 4;

                        bit_shift -= 4;
                        bit_mask >>= 4;
                        pixel_index++;
                    }
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
            case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint8_t index = *scan_input++;

                    if (image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED) {
                        SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba));
                    } else {
                        spread_gray8_to_rgba32(index, &rgba);
                    }

                    fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    spread_gray16_to_rgba32(*scan_input++, &rgba);
                    fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint8_t value = *scan_input++;
                    const uint8_t alpha = *scan_input++;

                    spread_gray8_to_rgba32(value, &rgba);
                    rgba.component4 = alpha;

                    fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint16_t value = *scan_input++;
                    const uint16_t alpha = *scan_input++;

                    spread_gray16_to_rgba32(value, &rgba);
                    rgba.component4 = (uint8_t)(alpha / 257.0);

                    fill_rgba32_pixel(scan_output, r, g, b, a, rgba.component1, rgba.component2, rgba.component3, rgba.component4);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP24_RGB: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgb24_kind(&scan_input, image_input->width, 0, 1, 2, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP24_BGR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgb24_kind(&scan_input, image_input->width, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP48_RGB: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgb48_kind(&scan_input, image_input->width, 0, 1, 2, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP48_BGR: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgb48_kind(&scan_input, image_input->width, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_RGBX: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgbx32_kind(&scan_input, image_input->width, 0, 1, 2, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_BGRX: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgbx32_kind(&scan_input, image_input->width, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_XRGB: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgbx32_kind(&scan_input, image_input->width, 1, 2, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_XBGR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgbx32_kind(&scan_input, image_input->width, 3, 2, 1, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgba32_kind(&scan_input, image_input->width, 0, 1, 2, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgba32_kind(&scan_input, image_input->width, 2, 1, 0, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_ARGB: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgba32_kind(&scan_input, image_input->width, 1, 2, 3, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_ABGR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;
                fill_rgba32_scan_from_rgba32_kind(&scan_input, image_input->width, 3, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_RGBX: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgbx64_kind(&scan_input, image_input->width, 0, 1, 2, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_BGRX: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgbx64_kind(&scan_input, image_input->width, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_XRGB: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgbx64_kind(&scan_input, image_input->width, 1, 2, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_XBGR: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgbx64_kind(&scan_input, image_input->width, 3, 2, 1, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_RGBA: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgba64_kind(&scan_input, image_input->width, 0, 1, 2, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_BGRA: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgba64_kind(&scan_input, image_input->width, 2, 1, 0, 3, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_ARGB: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgba64_kind(&scan_input, image_input->width, 1, 2, 3, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);
                fill_rgba32_scan_from_rgba64_kind(&scan_input, image_input->width, 3, 2, 1, 0, &scan_output, r, g, b, a);
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP32_CMYK: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint8_t C =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t M =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t Y =  (uint8_t)(*scan_input++ / 100.0);
                    const uint8_t K =  (uint8_t)(*scan_input++ / 100.0);

                    fill_rgba32_pixel(scan_output, r, g, b, a,
                                        (uint8_t)((1-C) * (1-K) * 255), (uint8_t)((1-M) * (1-K) * 255),
                                        (uint8_t)((1-Y) * (1-K) * 255), 255);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP64_CMYK: {
                const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const uint16_t C =  (uint16_t)(*scan_input++ / 100.0);
                    const uint16_t M =  (uint16_t)(*scan_input++ / 100.0);
                    const uint16_t Y =  (uint16_t)(*scan_input++ / 100.0);
                    const uint16_t K =  (uint16_t)(*scan_input++ / 100.0);

                    fill_rgba32_pixel(scan_output, r, g, b, a,
                                        (uint8_t)((1-C) * (1-K) * 255), (uint8_t)((1-M) * (1-K) * 255),
                                        (uint8_t)((1-Y) * (1-K) * 255), 255);
                    scan_output += 4;
                }
                break;
            }
            case SAIL_PIXEL_FORMAT_BPP24_YCBCR: {
                const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

                for (unsigned pixel_index = 0; pixel_index < image_input->width; pixel_index++) {
                    const double Y  = *scan_input++;
                    const double Cb = *scan_input++;
                    const double Cr = *scan_input++;

                    const int rv = (int)(Y                        + 1.402   * (Cr - 128));
                    const int gv = (int)(Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128));
                    const int bv = (int)(Y + 1.772   * (Cb - 128));

                    fill_rgba32_pixel(scan_output, r, g, b, a,
                                        (uint8_t)(max(0, min(255, rv))), (uint8_t)(max(0, min(255, gv))),
                                        (uint8_t)(max(0, min(255, bv))), 255);
                    scan_output += 4;
                }
                break;
            }
            default: {
                const char *pixel_format_str = NULL;
                SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image_input->pixel_format, &pixel_format_str));
                SAIL_LOG_ERROR("Conversion %s -> kind of BPP32-RGBA is not currently supported", pixel_format_str);

                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }
    }

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_convert_image_to_bpp32_rgba_kind(const struct sail_image *image_input, enum SailPixelFormat output_pixel_format, struct sail_image **image_output) {

    SAIL_CHECK_IMAGE(image_input);
    SAIL_CHECK_IMAGE_PTR(image_output);

    int r, g, b, a;
    SAIL_TRY(verify_and_construct_rgba32_indexes(output_pixel_format, &r, &g, &b, &a));

    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image_skeleton(image_input, &image_local));

    image_local->pixel_format = output_pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    const unsigned pixels_size = image_local->height * image_local->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(to_bpp32_rgba_kind(image_input, r, g, b, a, image_local),
                        /* cleanup */ sail_destroy_image(image_local));

    *image_output = image_local;

    return SAIL_OK;
}

sail_status_t sail_convert_image_to_bpp32_rgba_kind_in_place(struct sail_image *image, enum SailPixelFormat output_pixel_format) {

    SAIL_CHECK_IMAGE(image);

    int r, g, b, a;
    SAIL_TRY(verify_and_construct_rgba32_indexes(output_pixel_format, &r, &g, &b, &a));

    if (image->pixel_format == output_pixel_format) {
        return SAIL_OK;
    }

    bool new_image_fits_into_existing;
    SAIL_TRY(sail_greater_equal_bits_per_pixel(image->pixel_format, output_pixel_format, &new_image_fits_into_existing));

    if (!new_image_fits_into_existing) {
        const char *input_pixel_format_str = NULL;
        SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &input_pixel_format_str));
        const char *output_pixel_format_str = NULL;
        SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &output_pixel_format_str));

        SAIL_LOG_ERROR("Conversion from %s to %s pixel format is not supported by this function", input_pixel_format_str, output_pixel_format_str);

        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    SAIL_TRY(to_bpp32_rgba_kind(image, r, g, b, a, image));

    image->pixel_format = output_pixel_format;

    return SAIL_OK;
}
