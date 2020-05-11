/*
    Copyright (c) 2020 Dmitry Baryshev

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

#include <sail/sail-common.h>

inline QImage::Format sailPixelFormatToQImageFormat(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE: return QImage::Format_Mono;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: return QImage::Format_Grayscale8;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:   return QImage::Format_Indexed8;
        case SAIL_PIXEL_FORMAT_BPP24_RGB:      return QImage::Format_RGB888;
        case SAIL_PIXEL_FORMAT_BPP32_RGBX:     return QImage::Format_RGBX8888;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:     return QImage::Format_RGBA8888;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:     return QImage::Format_ARGB32;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:     return QImage::Format_ARGB32;

        // Cheat and display a distorted image
        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:    return QImage::Format_RGB888;
        case SAIL_PIXEL_FORMAT_BPP32_CMYK:     return QImage::Format_RGBA8888;
        case SAIL_PIXEL_FORMAT_BPP32_YCCK:     return QImage::Format_RGBA8888;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:      return QImage::Format_RGB888;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:     return QImage::Format_RGBA8888;

        default: return QImage::Format_Invalid;
    }
}

inline int qImageFormatToSailPixelFormat(QImage::Format format) {
    switch (format) {
        case QImage::Format_Mono:       return SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;
        case QImage::Format_Grayscale8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case QImage::Format_Indexed8:   return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        case QImage::Format_RGB888:     return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case QImage::Format_RGBX8888:   return SAIL_PIXEL_FORMAT_BPP32_RGBX;
        case QImage::Format_RGBA8888:   return SAIL_PIXEL_FORMAT_BPP32_RGBA;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

#endif
