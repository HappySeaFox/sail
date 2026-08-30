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

#include <cstring>

#include <QFileInfo>
#include <QImage>

#include <sail/sail.h>

#include <sail-manip/sail-manip.h>

// #define SAIL_CODEC_NAME jpeg
// #include <sail/layout/v8.h>

#include "qimage_sail_pixel_formats.h"
#include "qtsail.h"

QtSail::QtSail(QWidget* parent)
    : ImageViewer(NoFeatures, parent)
{
    setWindowTitle(tr("Qt SAIL demo [junior, C API]"));
}

sail_status_t QtSail::loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays)
{
    struct sail_image* image;
    SAIL_TRY(sail_load_from_file(path.toLocal8Bit(), &image));

    struct sail_image* image_converted;
    SAIL_TRY_OR_CLEANUP(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP32_RGBA, &image_converted),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

    // Construct QImage from the converted image pixels.
    //
    qimages->append(QImage(reinterpret_cast<const uchar*>(image_converted->pixels), image_converted->width,
                           image_converted->height, image_converted->bytes_per_line, QImage::Format_RGBA8888)
                        .copy());

    // Static images have no delay.
    //
    delays->append(0);

    setStatus(
        tr("%1  [%2x%3]").arg(QFileInfo(path).fileName()).arg(image_converted->width).arg(image_converted->height));

    sail_destroy_image(image_converted);

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString& path, const QImage& qimage)
{
    const SailPixelFormat pixel_format = qImageFormatToSailPixelFormat(qimage.format());

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const struct sail_codec_info* codec_info;
    SAIL_TRY(sail_codec_info_from_path(path.toLocal8Bit(), &codec_info));

    struct sail_image* image;
    SAIL_TRY(sail_alloc_image(&image));

    image->width        = qimage.width();
    image->height       = qimage.height();
    image->pixel_format = pixel_format;

    /*
     * QImage pads its scan lines, so take the line size from QImage instead of computing
     * the tightly packed one with sail_bytes_per_line(). SAIL accesses pixels line by line.
     */
    image->bytes_per_line = qimage.bytesPerLine();

    const size_t pixels_size = static_cast<size_t>(image->bytes_per_line) * image->height;

    /*
     * Pixels are freed by SAIL, so allocate them with SAIL as well. Mixing allocators
     * from different modules crashes on some platforms.
     */
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image->pixels),
                        /* cleanup */ sail_destroy_image(image));

    memcpy(image->pixels, qimage.constBits(), pixels_size);

    /*
     * SAIL tries to save an image as is, preserving its pixel format.
     * Particular image formats may support saving in different pixel formats:
     * RGB, Grayscale, etc. Convert the image to the best pixel format for saving here.
     *
     * You can prepare the image for saving by converting its pixel format on your own,
     * without using sail-manip.
     */
    struct sail_image* image_converted;
    SAIL_TRY_OR_CLEANUP(sail_convert_image_for_saving(image, codec_info->save_features, &image_converted),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

    SAIL_TRY_OR_CLEANUP(sail_save_into_file(path.toLocal8Bit(), image_converted),
                        /* cleanup */ sail_destroy_image(image_converted));

    sail_destroy_image(image_converted);

    return SAIL_OK;
}
