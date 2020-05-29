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
#include <QShortcut>
#include <QTimer>

#include <sail-c++/sail-c++.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

#include "multi-paged-impl.cpp"

QtSail::~QtSail()
{
}

sail_error_t QtSail::init()
{
    if (m_context.status() != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL. Error: %d").arg(m_context.status()));
        ::exit(1);
    }

    return 0;
}

sail_error_t QtSail::loadImage(const QString &path, QVector<QImage> *qimages, QVector<int> *delays)
{
    qimages->clear();
    delays->clear();

    sail::image_reader reader(&m_context);
    sail::image image;

    // Initialize reading.
    //
    SAIL_TRY(reader.start_reading(path.toLocal8Bit()));

    // Read all the available image frames in the file.
    //
    sail_error_t res = 0;
    while ((res = reader.read_next_frame(&image)) == 0) {

        const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image.pixel_format());

        if (qimageFormat == QImage::Format_Invalid) {
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        // Convert to QImage.
        //
        QImage qimage = QImage(reinterpret_cast<uchar *>(image.bits()),
                               image.width(),
                               image.height(),
                               image.bytes_per_line(),
                               qimageFormat).copy();

        // Apply palette.
        //
        if (qimageFormat == QImage::Format_Indexed8) {
            // Assume palette is BPP24-RGB.
            //
            if (image.palette_pixel_format() != SAIL_PIXEL_FORMAT_BPP24_RGB) {
                return SAIL_UNSUPPORTED_PIXEL_FORMAT;
            }

            QVector<QRgb> colorTable;
            unsigned char *palette = reinterpret_cast<unsigned char *>(image.palette());

            for (int i = 0; i < image.palette_color_count(); i++) {
                colorTable.append(qRgb(*palette, *(palette+1), *(palette+2)));
                palette += 3;
            }

            qimage.setColorTable(colorTable);
        }

        delays->append(image.delay());
        qimages->append(qimage);
    }

    if (res != SAIL_NO_MORE_FRAMES) {
        return res;
    }

    SAIL_LOG_DEBUG("Read images: %d", qimages->size());

    SAIL_TRY(reader.stop_reading());

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                );

    return 0;
}

sail_error_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    sail::image_writer writer(&m_context);
    sail::image image;

    image.with_width(qimage.width())
         .with_height(qimage.height())
         .with_pixel_format(qImageFormatToSailPixelFormat(qimage.format()))
         .with_bytes_per_line_auto()
         .with_shallow_bits(qimage.bits());

    SAIL_TRY(writer.start_writing(path.toLocal8Bit().constData()));
    SAIL_TRY(writer.write_next_frame(image));
    SAIL_TRY(writer.stop_writing());

    return 0;
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

    sail_error_t res;

    if ((res = loadImage(path, &m_qimages, &m_delays)) == 0) {
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

sail_error_t QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return 0;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    sail::image_reader reader(&m_context);
    sail::image image;
    sail::plugin_info plugin_info;

    sail_error_t res;

    if ((res = reader.probe(path.toLocal8Bit().constData(), &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return res;
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsedTimer.elapsed())
                                .arg(plugin_info.description().c_str())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                             );

    return 0;
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

    sail_error_t res;

    if ((res = saveImage(path, m_qimages.first())) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if ((res = loadImage(path, &m_qimages, &m_delays)) == 0) {
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
