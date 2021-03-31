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

#include <stdbool.h>
#include <stdlib.h>

#include "config.h"

#include "sail-common.h"

#include "ycbcr.h"

void convert_ycbcr_to_rgb(uint8_t y, uint8_t cb, uint8_t cr, uint8_t *r, uint8_t *g, uint8_t *b) {

    SAIL_THREAD_LOCAL static int R_CR[256];
    SAIL_THREAD_LOCAL static int G_CB[256];
    SAIL_THREAD_LOCAL static int G_CR[256];
    SAIL_THREAD_LOCAL static int B_CB[256];

    SAIL_THREAD_LOCAL static bool cache_initialized = false;

    if (!cache_initialized) {
        for (int i = 0; i < 256; i++) {
            R_CR[i] = (int)(1.40200 * (i - 128));
            G_CB[i] = (int)(0.34414 * (i - 128));
            G_CR[i] = (int)(0.71414 * (i - 128));
            B_CB[i] = (int)(1.77200 * (i - 128));
        }

        cache_initialized = true;
    }

    *r = (uint8_t)(max(0, min(255, y            + R_CR[cr])));
    *g = (uint8_t)(max(0, min(255, y - G_CB[cb] - G_CR[cr])));
    *b = (uint8_t)(max(0, min(255, y + B_CB[cb])));
}
