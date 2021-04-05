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
//#include <sail/layouts/v4.h>

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
                                                          "</ul>"));
    });

    return SAIL_OK;
}

sail_status_t QtSail::loadImage(const QString &path, QVector<QImage> *qimages, QVector<int> *delays)
{
    qimages->clear();
    delays->clear();

    sail::image_reader reader;
    sail::image image;

    // Initialize reading.
    //
    SAIL_TRY(reader.start_reading(path.toLocal8Bit().constData()));

    // Read all the available image frames in the file.
    //
    sail_status_t res;
    while ((res = reader.read_next_frame(&image)) == SAIL_OK) {

        SAIL_TRY(image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA));

        // Convert to QImage.
        //
        QImage qimage = QImage(reinterpret_cast<const uchar *>(image.pixels()),
                               image.width(),
                               image.height(),
                               image.bytes_per_line(),
                               QImage::Format_RGBA8888).copy();

        delays->append(image.delay());
        qimages->append(qimage);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES) {
        return res;
    }

    SAIL_LOG_DEBUG("Read images: %d", qimages->size());

    SAIL_TRY(reader.stop_reading());

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_image().pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 → %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                );

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    sail::image_writer writer;
    sail::image image;

    image.with_width(qimage.width())
         .with_height(qimage.height())
         .with_pixel_format(qImageFormatToSailPixelFormat(qimage.format()))
         .with_bytes_per_line_auto()
         .with_shallow_pixels(const_cast<uchar *>(qimage.bits()));

    SAIL_TRY(writer.start_writing(path.toLocal8Bit().constData()));
    SAIL_TRY(writer.write_next_frame(image));
    SAIL_TRY(writer.stop_writing());

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

    sail::image_reader reader;
    sail::image image;
    sail::codec_info codec_info;

    sail_status_t res;

    if ((res = reader.probe(path.toLocal8Bit().constData(), &image, &codec_info)) != SAIL_OK) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return res;
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_image().pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsedTimer.elapsed())
                                .arg(codec_info.description().c_str())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
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
