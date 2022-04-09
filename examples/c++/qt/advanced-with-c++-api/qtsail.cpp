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

#include <algorithm>
#include <cstdlib>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include <sail-c++/sail-c++.h>

//#define SAIL_CODEC_NAME jpeg
//#include <sail/layout/v7.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

#include "multi-paged-impl.cpp"

QtSail::~QtSail()
{
}

sail_status_t QtSail::init()
{
    QTimer::singleShot(0, this, [&]{
        QMessageBox::information(this, tr("Features"), tr("This demo includes:"
                                                          "<ul>"
                                                          "<li>Linking against SAIL CMake packages</li>"
                                                          "<li>Playing animations</li>"
                                                          "<li>Conversion with alpha blending</li>"
                                                          "</ul>"));
    });

    return SAIL_OK;
}

sail_status_t QtSail::loadImage(const QString &path, QVector<QImage> *qimages, QVector<int> *delays)
{
    qimages->clear();
    delays->clear();

    sail::image_input image_input;
    sail::image image;
    sail::image first_image;

    // Initialize loading.
    //
    sail::io_file io_file(path.toLocal8Bit().constData());
    SAIL_TRY(image_input.start(io_file));

    // Load all the available image frames in the file.
    //
    sail_status_t res;
    while ((res = image_input.next_frame(&image)) == SAIL_OK) {

        // Mutate alpha into a green color.
        //
        const sail::conversion_options options{
            SAIL_CONVERSION_OPTION_BLEND_ALPHA,
            sail_rgb24_t{ 0, 255, 0 }
        };

        if (!first_image.is_valid()) {
            first_image = image;
        }

        SAIL_TRY(image.convert(SAIL_PIXEL_FORMAT_BPP24_RGB, options));

        // Convert to QImage.
        //
        QImage qimage = QImage(reinterpret_cast<const uchar *>(image.pixels()),
                               image.width(),
                               image.height(),
                               image.bytes_per_line(),
                               QImage::Format_RGB888).copy();

        delays->append(image.delay());
        qimages->append(qimage);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES) {
        return res;
    }

    SAIL_LOG_DEBUG("Loaded images: %d", qimages->size());

    // Optional
    SAIL_TRY(image_input.stop());

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 → %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(first_image.width())
                                .arg(first_image.height())
                                .arg(sail::image::pixel_format_to_string(first_image.source_image().pixel_format()))
                                .arg(sail::image::pixel_format_to_string(first_image.pixel_format()))
                                );

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    const sail::codec_info codec_info = sail::codec_info::from_path(path.toLocal8Bit().constData());

    if (!codec_info.is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    sail::image_output image_output;
    sail::image image(const_cast<uchar *>(qimage.bits()), qImageFormatToSailPixelFormat(qimage.format()), qimage.width(), qimage.height(), qimage.bytesPerLine());

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

    SAIL_TRY(image_output.start(path.toLocal8Bit().constData(), save_options));
    SAIL_TRY(image_output.next_frame(image));
    // Optional
    SAIL_TRY(image_output.stop());

    return SAIL_OK;
}

#include "filters-impl-c++.cpp"

void QtSail::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                      tr("Select a file"),
                                                      QString(),
                                                      filters().join(QStringLiteral(";;")));

    if (path.isEmpty()) {
        return;
    }

    sail_status_t res;

    if ((res = loadImage(path, &m_qimages, &m_delays)) == SAIL_OK) {
        m_currentIndex = 0;
        onFit(m_ui->checkFit->isChecked());
        detectAnimated();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }
}

sail_status_t QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return SAIL_OK;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    sail::image_input image_input;
    auto [image, codec_info] = image_input.probe(path.toLocal8Bit().constData());

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsedTimer.elapsed())
                                .arg(codec_info.description().c_str())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(sail::image::pixel_format_to_string(image.source_image().pixel_format()))
                                .arg(sail::image::pixel_format_to_string(image.pixel_format()))
                             );

    return SAIL_OK;
}

void QtSail::onSave()
{
    const QString path = QFileDialog::getSaveFileName(this,
                                                      tr("Select a file"),
                                                      QString(),
                                                      filters().join(QStringLiteral(";;")));

    if (path.isEmpty()) {
        return;
    }

    sail_status_t res;

    if ((res = saveImage(path, m_qimages.first())) != SAIL_OK) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if ((res = loadImage(path, &m_qimages, &m_delays)) == SAIL_OK) {
            m_currentIndex = 0;
            onFit(m_ui->checkFit->isChecked());
            detectAnimated();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                                  .arg(path)
                                  .arg(res));
            return;
        }
    }
}
