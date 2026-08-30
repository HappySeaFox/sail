/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <QFileInfo>
#include <QImage>

#include <sail-c++/sail-c++.h>

// #define SAIL_CODEC_NAME jpeg
// #include <sail/layout/v8.h>

#include "qimage_sail_pixel_formats.h"
#include "qtsail.h"

QtSail::QtSail(QWidget* parent)
    : ImageViewer(NoFeatures, parent)
{
    setWindowTitle(tr("Qt SAIL demo [junior, C++ API]"));
}

sail_status_t QtSail::loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays)
{
    // Load the first frame of the file.
    //
    sail::image image;
    SAIL_TRY(image.load(path.toLocal8Bit().constData()));

    // Convert to RGBA
    //
    SAIL_TRY(image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA));

    // Construct QImage from the loaded image.
    //
    qimages->append(QImage(reinterpret_cast<const uchar*>(image.pixels()), image.width(), image.height(),
                           image.bytes_per_line(), QImage::Format_RGBA8888)
                        .copy());

    // Static images have no delay.
    //
    delays->append(0);

    setStatus(tr("%1  [%2x%3]").arg(QFileInfo(path).fileName()).arg(image.width()).arg(image.height()));

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString& path, const QImage& qimage)
{
    const SailPixelFormat pixel_format = qImageFormatToSailPixelFormat(qimage.format());

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const sail::codec_info codec_info = sail::codec_info::from_path(path.toLocal8Bit().constData());

    if (!codec_info.is_valid())
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    // The image just wraps the QImage pixels, it doesn't copy or own them. QImage pads
    // its scan lines, so pass its line size explicitly.
    //
    sail::image image(const_cast<uchar*>(qimage.constBits()), pixel_format, qimage.width(), qimage.height(),
                      qimage.bytesPerLine());

    // SAIL tries to save an image as is, preserving its pixel format.
    // Particular image formats may support saving in different pixel formats:
    // RGB, Grayscale, etc. Convert the image to the best pixel format for saving here.
    //
    // You can prepare the image for saving by converting its pixel format on your own,
    // without using conversion methods.
    //
    SAIL_TRY(image.convert(codec_info.save_features()));

    SAIL_TRY(image.save(path.toLocal8Bit().constData()));

    return SAIL_OK;
}
