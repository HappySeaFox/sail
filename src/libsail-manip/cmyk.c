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

void convert_cmyk32_to_rgba32(uint8_t c, uint8_t m, uint8_t y, uint8_t k, sail_rgba32_t *rgba32) {

#if 0
    const uint8_t C =  (uint8_t)(c / 100.0);
    const uint8_t M =  (uint8_t)(m / 100.0);
    const uint8_t Y =  (uint8_t)(y / 100.0);
    const uint8_t K =  (uint8_t)(k / 100.0);

    *r = (uint8_t)((1-C) * (1-K) * 255);
    *g = (uint8_t)((1-M) * (1-K) * 255);
    *b = (uint8_t)((1-Y) * (1-K) * 255);
#else
    rgba32->component1 = (uint8_t)((double)c * k / 255.0 + 0.5);
    rgba32->component2 = (uint8_t)((double)m * k / 255.0 + 0.5);
    rgba32->component3 = (uint8_t)((double)y * k / 255.0 + 0.5);
    rgba32->component4 = 255;
#endif
}
