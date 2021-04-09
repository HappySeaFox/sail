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

void fill_rgba32_pixel_from_uint8_values(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_DROP_ALPHA) == 0) {
        const double opacity = av / 255.0;

        *(scan+r) = (uint8_t)(opacity * rv + (1 - opacity) * options->background.component1 / 257.0);
        *(scan+g) = (uint8_t)(opacity * gv + (1 - opacity) * options->background.component2 / 257.0);
        *(scan+b) = (uint8_t)(opacity * bv + (1 - opacity) * options->background.component3 / 257.0);
    } else {
        *(scan+r) = rv;
        *(scan+g) = gv;
        *(scan+b) = bv;
    }

    if (a >= 0) {
        *(scan+a) = av;
    }
}

void fill_rgba32_pixel_from_uint16_values(uint16_t rv, uint16_t gv, uint16_t bv, uint16_t av, uint8_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_DROP_ALPHA) == 0) {
        const double opacity = av / 65535.0;

        *(scan+r) = (uint8_t)((opacity * rv + (1 - opacity) * options->background.component1) / 257.0);
        *(scan+g) = (uint8_t)((opacity * gv + (1 - opacity) * options->background.component2) / 257.0);
        *(scan+b) = (uint8_t)((opacity * bv + (1 - opacity) * options->background.component3) / 257.0);
    } else {
        *(scan+r) = (uint8_t)(rv / 257.0);
        *(scan+g) = (uint8_t)(gv / 257.0);
        *(scan+b) = (uint8_t)(bv / 257.0);
    }

    if (a >= 0) {
        *(scan+a) = (uint8_t)(av / 257.0);
    }
}

void fill_rgba64_pixel_from_uint8_values(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_DROP_ALPHA) == 0) {
        const double opacity = av / 255.0;

        *(scan+r) = (uint16_t)(opacity * (rv * 257) + (1 - opacity) * options->background.component1);
        *(scan+g) = (uint16_t)(opacity * (gv * 257) + (1 - opacity) * options->background.component2);
        *(scan+b) = (uint16_t)(opacity * (bv * 257) + (1 - opacity) * options->background.component3);
    } else {
        *(scan+r) = rv * 257;
        *(scan+g) = gv * 257;
        *(scan+b) = bv * 257;
    }

    if (a >= 0) {
        *(scan+a) = av * 257;
    }
}

void fill_rgba64_pixel_from_uint16_values(uint16_t rv, uint16_t gv, uint16_t bv, uint16_t av, uint16_t *scan, int r, int g, int b, int a, const struct sail_conversion_options *options) {

    if (a < 0 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_DROP_ALPHA) == 0) {
        const double opacity = av / 65535.0;

        *(scan+r) = (uint16_t)(opacity * rv + (1 - opacity) * options->background.component1);
        *(scan+g) = (uint16_t)(opacity * gv + (1 - opacity) * options->background.component2);
        *(scan+b) = (uint16_t)(opacity * bv + (1 - opacity) * options->background.component3);
    } else {
        *(scan+r) = rv;
        *(scan+g) = gv;
        *(scan+b) = bv;
    }

    if (a >= 0) {
        *(scan+a) = av;
    }
}
