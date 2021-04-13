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

#ifndef SAIL_MANIP_COMMON_H
#define SAIL_MANIP_COMMON_H

/*
 * Options to control color conversion behavior.
 */
enum SailConversionOption {

    /*
     * Drops the input alpha channel if the output alpha channel doesn't exist.
     * For example, when we convert RGBA pixels to RGB.
     *
     * SAIL_CONVERSION_OPTION_DROP_ALPHA and SAIL_CONVERSION_OPTION_BLEND_ALPHA are mutually
     * exclusive. If both are specified, SAIL_CONVERSION_OPTION_BLEND_ALPHA wins.
     */
    SAIL_CONVERSION_OPTION_DROP_ALPHA  = 1 << 0,

    /*
     * Blend the input alpha channel into the other color components if the output alpha channel
     * doesn't exist. For example, when we convert RGBA pixels to RGB.
     *
     * Formula:
     *   opacity = alpha / max_alpha (to convert to [0, 1])
     *   output_pixel = opacity * input_pixel + (1 - opacity) * background
     */
    SAIL_CONVERSION_OPTION_BLEND_ALPHA = 1 << 1,
};

#endif
