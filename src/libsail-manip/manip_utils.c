/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include "sail-manip.h"

/* https://en.wikipedia.org/wiki/Grayscale */
static const double R_TO_GRAY_COEFFICIENT = 0.299;
static const double G_TO_GRAY_COEFFICIENT = 0.587;
static const double B_TO_GRAY_COEFFICIENT = 0.114;

sail_status_t get_palette_rgba32(const struct sail_palette *palette, unsigned index, sail_rgba32_t *rgba32) {

    if (index >= palette->color_count) {
        SAIL_LOG_ERROR("Palette index %u is out of range [0; %u)", index, palette->color_count);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    switch (palette->pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            const uint8_t *entry = (uint8_t *)palette->data + index * 3;

            rgba32->component1 = *(entry+0);
            rgba32->component2 = *(entry+1);
            rgba32->component3 = *(entry+2);
            rgba32->component4 = 255;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            const uint8_t *entry = (uint8_t *)palette->data + index * 4;

            rgba32->component1 = *(entry+0);
            rgba32->component2 = *(entry+1);
            rgba32->component3 = *(entry+2);
            rgba32->component4 = *(entry+3);
            break;
        }
        default: {
            SAIL_LOG_ERROR("Palette pixel format %s is not currently supported", sail_pixel_format_to_string(palette->pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    return SAIL_OK;
}

void spread_gray8_to_rgba32(uint8_t value, sail_rgba32_t *rgba32) {

    rgba32->component1 = rgba32->component2 = rgba32->component3 = value;
    rgba32->component4 = 255;
}

void spread_gray16_to_rgba32(uint16_t value, sail_rgba32_t *rgba32) {

    rgba32->component1 = rgba32->component2 = rgba32->component3 = (uint8_t)(value / 257.0);
    rgba32->component4 = 255;
}

void spread_gray8_to_rgba64(uint8_t value, sail_rgba64_t *rgba64) {

    rgba64->component1 = rgba64->component2 = rgba64->component3 = (uint16_t)value * 257;
    rgba64->component4 = 65535;
}

void spread_gray16_to_rgba64(uint16_t value, sail_rgba64_t *rgba64) {

    rgba64->component1 = rgba64->component2 = rgba64->component3 = value;
    rgba64->component4 = 65535;
}

void fill_gray8_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, const struct sail_conversion_options *options) {

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    } else {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    *scan = (uint8_t)((R_TO_GRAY_COEFFICIENT * rgb24.component1) + (G_TO_GRAY_COEFFICIENT * rgb24.component2) + (B_TO_GRAY_COEFFICIENT * rgb24.component3));
}

void fill_gray8_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, const struct sail_conversion_options *options) {

    sail_rgb24_t rgb24;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        rgb24.component1 = (uint8_t)((opacity * rgba64->component1 + (1 - opacity) * options->background48.component1) / 257.0);
        rgb24.component2 = (uint8_t)((opacity * rgba64->component2 + (1 - opacity) * options->background48.component2) / 257.0);
        rgb24.component3 = (uint8_t)((opacity * rgba64->component3 + (1 - opacity) * options->background48.component3) / 257.0);
    } else {
        rgb24.component1 = (uint8_t)(rgba64->component1 / 257.0);
        rgb24.component2 = (uint8_t)(rgba64->component2 / 257.0);
        rgb24.component3 = (uint8_t)(rgba64->component3 / 257.0);
    }

    *scan = (uint8_t)((R_TO_GRAY_COEFFICIENT * rgb24.component1) + (G_TO_GRAY_COEFFICIENT * rgb24.component2) + (B_TO_GRAY_COEFFICIENT * rgb24.component3));
}

void fill_gray16_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, const struct sail_conversion_options *options) {

    sail_rgb48_t rgb48;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        rgb48.component1 = (uint16_t)(opacity * (rgba32->component1 * 257) + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * (rgba32->component2 * 257) + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * (rgba32->component3 * 257) + (1 - opacity) * options->background48.component3);
    } else {
        rgb48.component1 = rgba32->component1 * 257;
        rgb48.component2 = rgba32->component2 * 257;
        rgb48.component3 = rgba32->component3 * 257;
    }

    *scan = (uint16_t)((R_TO_GRAY_COEFFICIENT * rgb48.component1) + (G_TO_GRAY_COEFFICIENT * rgb48.component2) + (B_TO_GRAY_COEFFICIENT * rgb48.component3));
}

void fill_gray16_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, const struct sail_conversion_options *options) {

    sail_rgb48_t rgb48;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        rgb48.component1 = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    } else {
        rgb48.component1 = rgba64->component1;
        rgb48.component2 = rgba64->component2;
        rgb48.component3 = rgba64->component3;
    }

    *scan = (uint16_t)((R_TO_GRAY_COEFFICIENT * rgb48.component1) + (G_TO_GRAY_COEFFICIENT * rgb48.component2) + (B_TO_GRAY_COEFFICIENT * rgb48.component3));
}

void fill_rgb24_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, int r, int g, int b, const struct sail_conversion_options *options) {

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        *(scan+r) = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        *(scan+g) = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        *(scan+b) = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    } else {
        *(scan+r) = rgba32->component1;
        *(scan+g) = rgba32->component2;
        *(scan+b) = rgba32->component3;
    }
}

void fill_rgb24_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, int r, int g, int b, const struct sail_conversion_options *options) {

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan+r) = (uint8_t)((opacity * rgba64->component1 + (1 - opacity) * options->background48.component1) / 257.0);
        *(scan+g) = (uint8_t)((opacity * rgba64->component2 + (1 - opacity) * options->background48.component2) / 257.0);
        *(scan+b) = (uint8_t)((opacity * rgba64->component3 + (1 - opacity) * options->background48.component3) / 257.0);
    } else {
        *(scan+r) = (uint8_t)(rgba64->component1 / 257.0);
        *(scan+g) = (uint8_t)(rgba64->component2 / 257.0);
        *(scan+b) = (uint8_t)(rgba64->component3 / 257.0);
    }
}

void fill_rgb48_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, int r, int g, int b, const struct sail_conversion_options *options) {

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        *(scan+r) = (uint16_t)(opacity * (rgba32->component1 * 257) + (1 - opacity) * options->background48.component1);
        *(scan+g) = (uint16_t)(opacity * (rgba32->component2 * 257) + (1 - opacity) * options->background48.component2);
        *(scan+b) = (uint16_t)(opacity * (rgba32->component3 * 257) + (1 - opacity) * options->background48.component3);
    } else {
        *(scan+r) = rgba32->component1 * 257;
        *(scan+g) = rgba32->component2 * 257;
        *(scan+b) = rgba32->component3 * 257;
    }
}

void fill_rgb48_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, int r, int g, int b, const struct sail_conversion_options *options) {

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan+r) = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        *(scan+g) = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        *(scan+b) = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    } else {
        *(scan+r) = rgba64->component1;
        *(scan+g) = rgba64->component2;
        *(scan+b) = rgba64->component3;
    }
}

void fill_rgba32_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        *(scan+r) = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        *(scan+g) = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        *(scan+b) = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    } else {
        *(scan+r) = rgba32->component1;
        *(scan+g) = rgba32->component2;
        *(scan+b) = rgba32->component3;
    }

    if (a >= 0) {
        *(scan+a) = rgba32->component4;
    }
}

void fill_rgba32_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan+r) = (uint8_t)((opacity * rgba64->component1 + (1 - opacity) * options->background48.component1) / 257.0);
        *(scan+g) = (uint8_t)((opacity * rgba64->component2 + (1 - opacity) * options->background48.component2) / 257.0);
        *(scan+b) = (uint8_t)((opacity * rgba64->component3 + (1 - opacity) * options->background48.component3) / 257.0);
    } else {
        *(scan+r) = (uint8_t)(rgba64->component1 / 257.0);
        *(scan+g) = (uint8_t)(rgba64->component2 / 257.0);
        *(scan+b) = (uint8_t)(rgba64->component3 / 257.0);
    }

    if (a >= 0) {
        *(scan+a) = (uint8_t)(rgba64->component4 / 257.0);
    }
}

void fill_rgba64_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        *(scan+r) = (uint16_t)(opacity * (rgba32->component1 * 257) + (1 - opacity) * options->background48.component1);
        *(scan+g) = (uint16_t)(opacity * (rgba32->component2 * 257) + (1 - opacity) * options->background48.component2);
        *(scan+b) = (uint16_t)(opacity * (rgba32->component3 * 257) + (1 - opacity) * options->background48.component3);
    } else {
        *(scan+r) = rgba32->component1 * 257;
        *(scan+g) = rgba32->component2 * 257;
        *(scan+b) = rgba32->component3 * 257;
    }

    if (a >= 0) {
        *(scan+a) = rgba32->component4 * 257;
    }
}

void fill_rgba64_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan+r) = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        *(scan+g) = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        *(scan+b) = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    } else {
        *(scan+r) = rgba64->component1;
        *(scan+g) = rgba64->component2;
        *(scan+b) = rgba64->component3;
    }

    if (a >= 0) {
        *(scan+a) = rgba64->component4;
    }
}

void fill_ycbcr_pixel_from_uint8_values(const sail_rgba32_t *rgba32, uint8_t *scan, const struct sail_conversion_options *options) {

    sail_rgba32_t rgba32_no_alpha;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba32->component4 / 255.0;

        rgba32_no_alpha.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgba32_no_alpha.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgba32_no_alpha.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    } else {
        rgba32_no_alpha = *rgba32;
    }

    convert_rgba32_to_ycbcr24(&rgba32_no_alpha, scan+0, scan+1, scan+2);
}

void fill_ycbcr_pixel_from_uint16_values(const sail_rgba64_t *rgba64, uint8_t *scan, const struct sail_conversion_options *options) {

    sail_rgba32_t rgba32_no_alpha;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA)) {
        const double opacity = rgba64->component4 / 65535.0;

        rgba32_no_alpha.component1 = (uint8_t)((opacity * rgba64->component1 + (1 - opacity) * options->background48.component1) / 257.0);
        rgba32_no_alpha.component2 = (uint8_t)((opacity * rgba64->component2 + (1 - opacity) * options->background48.component2) / 257.0);
        rgba32_no_alpha.component3 = (uint8_t)((opacity * rgba64->component3 + (1 - opacity) * options->background48.component3) / 257.0);
    } else {
        rgba32_no_alpha.component1 = (uint8_t)(rgba64->component1 / 257.0);
        rgba32_no_alpha.component2 = (uint8_t)(rgba64->component2 / 257.0);
        rgba32_no_alpha.component3 = (uint8_t)(rgba64->component3 / 257.0);
    }

    convert_rgba32_to_ycbcr24(&rgba32_no_alpha, scan+0, scan+1, scan+2);
}
