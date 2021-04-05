/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#ifndef QIMAGE_SAIL_PIXEL_FORMATS_H
#define QIMAGE_SAIL_PIXEL_FORMATS_H

#include <QImage>

#include <sail-common/sail-common.h>

inline SailPixelFormat qImageFormatToSailPixelFormat(QImage::Format format) {
    switch (format) {
        case QImage::Format_Mono:       return SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;
        case QImage::Format_Grayscale8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case QImage::Format_Indexed8:   return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        case QImage::Format_RGB888:     return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case QImage::Format_RGBX8888:   return SAIL_PIXEL_FORMAT_BPP32_RGBX;
        case QImage::Format_RGBA8888:   return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case QImage::Format_ARGB32:     return SAIL_PIXEL_FORMAT_BPP32_ARGB;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

#endif
