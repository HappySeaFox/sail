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

#include <string>
#include <vector>

#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QTimer>

#include <sail-c++/sail-c++.h>

// #define SAIL_CODEC_NAME jpeg
// #include <sail/layout/v8.h>

#include "qimage_sail_pixel_formats.h"
#include "qtsail.h"

QtSail::QtSail(QWidget* parent)
    : ImageViewer(Probe | MultiFrame, parent)
{
    setWindowTitle(tr("Qt SAIL demo [advanced, C++ API]"));

    QTimer::singleShot(0, this, [this] {
        QMessageBox::information(this, tr("Features"),
                                 tr("This demo includes:"
                                    "<ul>"
                                    "<li>Linking against SAIL CMake packages</li>"
                                    "<li>Playing animations</li>"
                                    "<li>Conversion with alpha blending</li>"
                                    "</ul>"));
    });
}

sail_status_t QtSail::loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays)
{
    // Initialize loading.
    //
    sail::image_input image_input(path.toLocal8Bit().constData());

    // Save the properties of the first frame for displaying in the status bar.
    //
    QString source_pixel_format;
    QString pixel_format;
    unsigned width  = 0;
    unsigned height = 0;

    // Load all the available image frames in the file.
    //
    sail::image image;
    sail_status_t res;

    while ((res = image_input.next_frame(&image)) == SAIL_OK)
    {
        // Mutate alpha into a green color.
        //
        const sail::conversion_options options{SAIL_CONVERSION_OPTION_BLEND_ALPHA, sail_rgb24_t{0, 255, 0}};

        const bool first_frame = qimages->isEmpty();

        if (first_frame)
        {
            source_pixel_format = sail::image::pixel_format_to_string(image.source_image().pixel_format());
        }

        SAIL_TRY(image.convert(SAIL_PIXEL_FORMAT_BPP24_RGB, options));

        if (first_frame)
        {
            pixel_format = sail::image::pixel_format_to_string(image.pixel_format());
            width        = image.width();
            height       = image.height();
        }

        // Convert to QImage.
        //
        qimages->append(QImage(reinterpret_cast<const uchar*>(image.pixels()), image.width(), image.height(),
                               image.bytes_per_line(), QImage::Format_RGB888)
                            .copy());
        delays->append(image.delay());
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES)
    {
        SAIL_LOG_AND_RETURN(res);
    }

    SAIL_LOG_DEBUG("Loaded images: %d", qimages->size());

    // Empty files are reported by the viewer.
    //
    if (qimages->isEmpty())
    {
        return SAIL_OK;
    }

    setStatus(tr("%1  [%2x%3]  [%4 → %5]")
                  .arg(QFileInfo(path).fileName())
                  .arg(width)
                  .arg(height)
                  .arg(source_pixel_format)
                  .arg(pixel_format));

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

    sail::save_options save_options;
    SAIL_TRY(codec_info.save_features().to_options(&save_options));

    // This filter will be handled and applied by the PNG codec only.
    // Possible values: "none", "sub", "up", "avg", "paeth".
    //
    // It's also possible to combine PNG filters with ';' like that:
    // "none;sub;paeth"
    //
    save_options.tuning()["png-filter"] = std::string("none;sub");

    sail::image_output image_output(path.toLocal8Bit().constData());
    image_output.with(save_options);
    SAIL_TRY(image_output.next_frame(image));
    SAIL_TRY(image_output.finish());

    return SAIL_OK;
}

sail_status_t QtSail::probeImage(const QString& path, QString* info)
{
    const auto [image, codec_info] = sail::image_input(path.toLocal8Bit().constData()).probe();

    // Probed images have no pixels, so check the codec info to detect failures.
    //
    if (!codec_info.is_valid())
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    *info = tr("Codec: %1\nSize: %2x%3\nSource compression: %4\nSource pixel format: %5\nOutput pixel format: %6")
                .arg(codec_info.description().c_str())
                .arg(image.width())
                .arg(image.height())
                .arg(sail::image::compression_to_string(image.source_image().compression()))
                .arg(sail::image::pixel_format_to_string(image.source_image().pixel_format()))
                .arg(sail::image::pixel_format_to_string(image.pixel_format()));

    return SAIL_OK;
}

QStringList QtSail::filters() const
{
    QStringList filters{QStringLiteral("All Files (*.*)")};

    for (const sail::codec_info& codec_info : sail::codec_info::list())
    {
        QStringList masks;

        for (const std::string& extension : codec_info.extensions())
        {
            masks.append(QStringLiteral("*.%1").arg(extension.c_str()));
        }

        filters.append(QStringLiteral("%1: %2 (%3)")
                           .arg(codec_info.name().c_str())
                           .arg(codec_info.description().c_str())
                           .arg(masks.join(QStringLiteral(" "))));
    }

    return filters;
}
