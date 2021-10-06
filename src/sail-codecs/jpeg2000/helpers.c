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

#include "sail-common.h"

#include "helpers.h"

enum SailPixelFormat jpeg2000_private_sail_pixel_format(jas_clrspc_t jasper_color_space, int bpp) {

    switch (jasper_color_space) {
        case JAS_CLRSPC_FAM_GRAY: {
            switch (bpp) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
        case JAS_CLRSPC_FAM_RGB: {
            switch (bpp) {
                case 24: return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case 32: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                case 48: return SAIL_PIXEL_FORMAT_BPP48_RGB;
                case 64: return SAIL_PIXEL_FORMAT_BPP64_RGBA;

                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
        case JAS_CLRSPC_FAM_YCBCR: return SAIL_PIXEL_FORMAT_BPP24_YCBCR;

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}
