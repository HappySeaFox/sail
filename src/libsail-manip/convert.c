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

#include "cmyk.h"
#include "ycbcr.h"

/*
 * Private functions.
 */

struct output_context {
    struct sail_image *image;
    int r;
    int g;
    int b;
    int a;
    const struct sail_conversion_options *options;
};

typedef void (*pixel_consumer_t)(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64);

static void pixel_consumer_rgb24_kind(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64) {

    uint8_t *scan = (uint8_t *)output_context->image->pixels + output_context->image->bytes_per_line * row + column * 3;

    if (rgba32 != NULL) {
        fill_rgb24_pixel_from_uint8_values(rgba32, scan, output_context->r, output_context->g, output_context->b, output_context->options);
    } else {
        fill_rgb24_pixel_from_uint16_values(rgba64, scan, output_context->r, output_context->g, output_context->b, output_context->options);
    }
}

static void pixel_consumer_rgb48_kind(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64) {

    uint16_t *scan = (uint16_t *)((uint8_t *)output_context->image->pixels + output_context->image->bytes_per_line * row + column * 6);

    if (rgba32 != NULL) {
        fill_rgb48_pixel_from_uint8_values(rgba32, scan, output_context->r, output_context->g, output_context->b, output_context->options);
    } else {
        fill_rgb48_pixel_from_uint16_values(rgba64, scan, output_context->r, output_context->g, output_context->b, output_context->options);
    }
}

static void pixel_consumer_rgba32_kind(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64) {

    uint8_t *scan = (uint8_t *)output_context->image->pixels + output_context->image->bytes_per_line * row + column * 4;

    if (rgba32 != NULL) {
        fill_rgba32_pixel_from_uint8_values(rgba32, scan, output_context->r, output_context->g, output_context->b, output_context->a, output_context->options);
    } else {
        fill_rgba32_pixel_from_uint16_values(rgba64, scan, output_context->r, output_context->g, output_context->b, output_context->a, output_context->options);
    }
}

static void pixel_consumer_rgba64_kind(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64) {

    uint16_t *scan = (uint16_t *)((uint8_t *)output_context->image->pixels + output_context->image->bytes_per_line * row + column * 8);

    if (rgba32 != NULL) {
        fill_rgba64_pixel_from_uint8_values(rgba32, scan, output_context->r, output_context->g, output_context->b, output_context->a, output_context->options);
    } else {
        fill_rgba64_pixel_from_uint16_values(rgba64, scan, output_context->r, output_context->g, output_context->b, output_context->a, output_context->options);
    }
}

static void pixel_consumer_ycbcr(const struct output_context *output_context, unsigned row, unsigned column, const sail_rgba32_t *rgba32, const sail_rgba64_t *rgba64) {

    uint8_t *scan = (uint8_t *)output_context->image->pixels + output_context->image->bytes_per_line * row + column * 3;

    if (rgba32 != NULL) {
        fill_ycbcr_pixel_from_uint8_values(rgba32, scan, output_context->options);
    } else {
        fill_ycbcr_pixel_from_uint16_values(rgba64, scan, output_context->options);
    }
}

static sail_status_t verify_and_construct_rgba_indexes(enum SailPixelFormat output_pixel_format, pixel_consumer_t *pixel_consumer, int *r, int *g, int *b, int *a) {

    switch (output_pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB: { *pixel_consumer = pixel_consumer_rgb24_kind; *r = 0; *g = 1; *b = 2; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP24_BGR: { *pixel_consumer = pixel_consumer_rgb24_kind; *r = 2; *g = 1; *b = 0; *a = -1; break; }

        case SAIL_PIXEL_FORMAT_BPP48_RGB: { *pixel_consumer = pixel_consumer_rgb48_kind; *r = 0; *g = 1; *b = 2; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP48_BGR: { *pixel_consumer = pixel_consumer_rgb48_kind; *r = 2; *g = 1; *b = 0; *a = -1; break; }

        case SAIL_PIXEL_FORMAT_BPP32_RGBX: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 0; *g = 1; *b = 2; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_BGRX: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 2; *g = 1; *b = 0; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_XRGB: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 1; *g = 2; *b = 3; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_XBGR: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 3; *g = 2; *b = 1; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 0; *g = 1; *b = 2; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 2; *g = 1; *b = 0; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 1; *g = 2; *b = 3; *a = 0;  break; }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: { *pixel_consumer = pixel_consumer_rgba32_kind; *r = 3; *g = 2; *b = 1; *a = 0;  break; }

        case SAIL_PIXEL_FORMAT_BPP64_RGBX: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 0; *g = 1; *b = 2; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_BGRX: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 2; *g = 1; *b = 0; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_XRGB: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 1; *g = 2; *b = 3; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_XBGR: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 3; *g = 2; *b = 1; *a = -1; break; }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 0; *g = 1; *b = 2; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_BGRA: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 2; *g = 1; *b = 0; *a = 3;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_ARGB: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 1; *g = 2; *b = 3; *a = 0;  break; }
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: { *pixel_consumer = pixel_consumer_rgba64_kind; *r = 3; *g = 2; *b = 1; *a = 0;  break; }

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: { *pixel_consumer = pixel_consumer_ycbcr; *r = *g = *b = *a = -1; /* unused. */ break; }

        default: {
            const char *pixel_format_str = NULL;
            SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(output_pixel_format, &pixel_format_str));
            SAIL_LOG_ERROR("Conversion to %s is not supported", pixel_format_str);

            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp1_indexed_or_grayscale(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    const bool is_indexed = image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED;
    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width;) {
            unsigned bit_shift = 7;
            unsigned bit_mask = 1 << 7;
            const uint8_t byte = *scan_input++;

            while (bit_mask > 0 && column < image_input->width) {
                const uint8_t index = (byte & bit_mask) >> bit_shift;

                if (is_indexed) {
                    SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba32));
                } else {
                    spread_gray8_to_rgba32(index == 0 ? 0 : 255, &rgba32);
                }

                pixel_consumer(output_context, row, column, &rgba32, NULL);

                bit_shift--;
                bit_mask >>= 1;
                column++;
            }
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp2_indexed_or_grayscale(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    const bool is_indexed = image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED;
    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width;) {
            unsigned bit_shift = 6;
            unsigned bit_mask = 3 << 6; /* 11000000 */
            const uint8_t byte = *scan_input++;

            while (bit_mask > 0 && column < image_input->width) {
                const uint8_t index = (byte & bit_mask) >> bit_shift;

                if (is_indexed) {
                    SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba32));
                } else {
                    spread_gray8_to_rgba32(index * 85, &rgba32);
                }

                pixel_consumer(output_context, row, column, &rgba32, NULL);

                bit_shift -= 2;
                bit_mask >>= 2;
                column++;
            }
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp4_indexed_or_grayscale(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    const bool is_indexed = image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED;
    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width;) {
            unsigned bit_shift = 4;
            unsigned bit_mask = 15 << 4; /* 11110000 */
            const uint8_t byte = *scan_input++;

            while (bit_mask > 0 && column < image_input->width) {
                const uint8_t index = (byte & bit_mask) >> bit_shift;

                if (is_indexed) {
                    SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba32));
                } else {
                    spread_gray8_to_rgba32(index * 17, &rgba32);
                }

                pixel_consumer(output_context, row, column, &rgba32, NULL);

                bit_shift -= 4;
                bit_mask >>= 4;
                column++;
            }
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp8_indexed_or_grayscale(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    const bool is_indexed = image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            const uint8_t index = *scan_input++;

            if (is_indexed) {
                SAIL_TRY(get_palette_rgba32(image_input->palette, index, &rgba32));
            } else {
                spread_gray8_to_rgba32(index, &rgba32);
            }

            pixel_consumer(output_context, row, column, &rgba32, NULL);
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp16_grayscale(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba64_t rgba64;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            spread_gray16_to_rgba64(*scan_input++, &rgba64);

            pixel_consumer(output_context, row, column, NULL, &rgba64);
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp16_grayscale_alpha(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            spread_gray8_to_rgba32(*scan_input++, &rgba32);
            rgba32.component4 = *scan_input++;

            pixel_consumer(output_context, row, column, &rgba32, NULL);
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp32_grayscale_alpha(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba64_t rgba64;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            spread_gray16_to_rgba64(*scan_input++, &rgba64);
            rgba64.component4 = *scan_input++;

            pixel_consumer(output_context, row, column, NULL, &rgba64);
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp16_rgb555(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;
    rgba32.component4 = 255;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba32.component1 = ((*scan_input >> 0)  & 0x1f) << 3;
            rgba32.component2 = ((*scan_input >> 5)  & 0x1f) << 3;
            rgba32.component3 = ((*scan_input >> 10) & 0x1f) << 3;

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input++;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp16_bgr555(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;
    rgba32.component4 = 255;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba32.component1 = ((*scan_input >> 10) & 0x1f) << 3;
            rgba32.component2 = ((*scan_input >> 5)  & 0x1f) << 3;
            rgba32.component3 = ((*scan_input >> 0)  & 0x1f) << 3;

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input++;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp24_rgb_kind(const struct sail_image *image_input, int ri, int gi, int bi, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;
    rgba32.component4 = 255;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba32.component1 = *(scan_input+ri);
            rgba32.component2 = *(scan_input+gi);
            rgba32.component3 = *(scan_input+bi);

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input += 3;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp48_rgb_kind(const struct sail_image *image_input, int ri, int gi, int bi, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba64_t rgba64;
    rgba64.component4 = 65535;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba64.component1 = *(scan_input+ri);
            rgba64.component2 = *(scan_input+gi);
            rgba64.component3 = *(scan_input+bi);

            pixel_consumer(output_context, row, column, NULL, &rgba64);
            scan_input += 3;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp32_rgba_kind(const struct sail_image *image_input, int ri, int gi, int bi, int ai, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba32.component1 = *(scan_input+ri);
            rgba32.component2 = *(scan_input+gi);
            rgba32.component3 = *(scan_input+bi);
            rgba32.component4 = ai >= 0 ? *(scan_input+ai) : 255;

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input += 4;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp64_rgba_kind(const struct sail_image *image_input, int ri, int gi, int bi, int ai, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba64_t rgba64;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint16_t *scan_input = (uint16_t *)((uint8_t *)image_input->pixels + image_input->bytes_per_line * row);

        for (unsigned column = 0; column < image_input->width; column++) {
            rgba64.component1 = *(scan_input+ri);
            rgba64.component2 = *(scan_input+gi);
            rgba64.component3 = *(scan_input+bi);
            rgba64.component4 = ai >= 0 ? *(scan_input+ai) : 65535;

            pixel_consumer(output_context, row, column, NULL, &rgba64);
            scan_input += 4;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp32_cmyk(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            SAIL_TRY(convert_cmyk32_to_rgba32(*(scan_input+0), *(scan_input+1), *(scan_input+2), *(scan_input+3), &rgba32));

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input += 4;
        }
    }

    return SAIL_OK;
}

static sail_status_t convert_from_bpp24_ycbcr(const struct sail_image *image_input, pixel_consumer_t pixel_consumer, const struct output_context *output_context) {

    sail_rgba32_t rgba32;

    for (unsigned row = 0; row < image_input->height; row++) {
        const uint8_t *scan_input = (uint8_t *)image_input->pixels + image_input->bytes_per_line * row;

        for (unsigned column = 0; column < image_input->width; column++) {
            convert_ycbcr24_to_rgba32(*(scan_input+0), *(scan_input+1), *(scan_input+2), &rgba32);

            pixel_consumer(output_context, row, column, &rgba32, NULL);
            scan_input += 3;
        }
    }

    return SAIL_OK;
}

static sail_status_t to_bpp32_rgba_kind(
    const struct sail_image *image_input,
    struct sail_image *image_output,
    pixel_consumer_t pixel_consumer,
    int r, /* Index of RED component. */
    int g, /* Index of GREEN component. */
    int b, /* Index of BLUE component. */
    int a, /* Index of ALPHA component. */
    const struct sail_conversion_options *options) {

    const struct output_context output_context = { image_output, r, g, b, a, options };

    switch (image_input->pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE: {
            SAIL_TRY(convert_from_bpp1_indexed_or_grayscale(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE: {
            SAIL_TRY(convert_from_bpp2_indexed_or_grayscale(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE: {
            SAIL_TRY(convert_from_bpp4_indexed_or_grayscale(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: {
            SAIL_TRY(convert_from_bpp8_indexed_or_grayscale(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: {
            SAIL_TRY(convert_from_bpp16_grayscale(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: {
            SAIL_TRY(convert_from_bpp16_grayscale_alpha(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
            SAIL_TRY(convert_from_bpp32_grayscale_alpha(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP16_RGB555: {
            SAIL_TRY(convert_from_bpp16_rgb555(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP16_BGR555: {
            SAIL_TRY(convert_from_bpp16_bgr555(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            SAIL_TRY(convert_from_bpp24_rgb_kind(image_input, 0, 1, 2, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP24_BGR: {
            SAIL_TRY(convert_from_bpp24_rgb_kind(image_input, 2, 1, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP48_RGB: {
            SAIL_TRY(convert_from_bpp48_rgb_kind(image_input, 0, 1, 2, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP48_BGR: {
            SAIL_TRY(convert_from_bpp48_rgb_kind(image_input, 2, 1, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBX: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 0, 1, 2, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_BGRX: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 2, 1, 0, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_XRGB: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 1, 2, 3, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_XBGR: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 3, 2, 1, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 0, 1, 2, 3, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 2, 1, 0, 3, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 1, 2, 3, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: {
            SAIL_TRY(convert_from_bpp32_rgba_kind(image_input, 3, 2, 1, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBX: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 0, 1, 2, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_BGRX: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 2, 1, 0, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_XRGB: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 1, 2, 3, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_XBGR: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 3, 2, 1, -1, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 0, 1, 2, 3, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_BGRA: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 2, 1, 0, 3, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_ARGB: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 1, 2, 3, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
            SAIL_TRY(convert_from_bpp64_rgba_kind(image_input, 3, 2, 1, 0, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_CMYK: {
            SAIL_TRY(convert_from_bpp32_cmyk(image_input, pixel_consumer, &output_context));
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: {
            SAIL_TRY(convert_from_bpp24_ycbcr(image_input, pixel_consumer, &output_context));
            break;
        }
        default: {
            const char *pixel_format_str = NULL;
            SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image_input->pixel_format, &pixel_format_str));
            SAIL_LOG_ERROR("Conversion from %s is not currently supported", pixel_format_str);

            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_convert_image(const struct sail_image *image_input,
                                 enum SailPixelFormat output_pixel_format,
                                 struct sail_image **image_output) {

    SAIL_TRY(sail_convert_image_with_options(image_input, output_pixel_format, NULL /* options */, image_output));

    return SAIL_OK;
}

sail_status_t sail_convert_image_with_options(const struct sail_image *image_input,
                                              enum SailPixelFormat output_pixel_format,
                                              const struct sail_conversion_options *options,
                                              struct sail_image **image_output) {

    SAIL_TRY(sail_check_image_valid(image_input));
    SAIL_CHECK_IMAGE_PTR(image_output);

    int r, g, b, a;
    pixel_consumer_t pixel_consumer;
    SAIL_TRY(verify_and_construct_rgba_indexes(output_pixel_format, &pixel_consumer, &r, &g, &b, &a));

    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image_skeleton(image_input, &image_local));

    image_local->pixel_format = output_pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    const unsigned pixels_size = image_local->height * image_local->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(to_bpp32_rgba_kind(image_input, image_local, pixel_consumer, r, g, b, a, options),
                        /* cleanup */ sail_destroy_image(image_local));

    *image_output = image_local;

    return SAIL_OK;
}

sail_status_t sail_update_image(struct sail_image *image_input, enum SailPixelFormat output_pixel_format) {

    SAIL_TRY(sail_update_image_with_options(image_input, output_pixel_format, NULL /* options */));

    return SAIL_OK;
}

sail_status_t sail_update_image_with_options(struct sail_image *image,
                                             enum SailPixelFormat output_pixel_format,
                                             const struct sail_conversion_options *options) {

    SAIL_TRY(sail_check_image_valid(image));

    int r, g, b, a;
    pixel_consumer_t pixel_consumer;
    SAIL_TRY(verify_and_construct_rgba_indexes(output_pixel_format, &pixel_consumer, &r, &g, &b, &a));

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

    SAIL_TRY(to_bpp32_rgba_kind(image, image, pixel_consumer, r, g, b, a, options));

    image->pixel_format = output_pixel_format;

    return SAIL_OK;
}
