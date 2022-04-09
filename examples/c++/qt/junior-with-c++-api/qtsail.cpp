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

#include <cstdlib>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include <sail-c++/sail-c++.h>

//#define SAIL_CODEC_NAME jpeg
//#include <sail/layout/v7.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

QtSail::QtSail(QWidget *parent)
    : QWidget(parent)
{
    m_ui.reset(new Ui::QtSail);
    m_ui->setupUi(this);
    QLabel *l = new QLabel;
    l->setAlignment(Qt::AlignCenter);
    m_ui->scrollArea->setWidget(l);

    connect(m_ui->pushOpen, &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(m_ui->pushSave, &QPushButton::clicked, this, &QtSail::onSave);
}

QtSail::~QtSail()
{
}

sail_status_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    sail::image_input image_input;
    sail::image image = image_input.load(path.toLocal8Bit().constData());

    if (!image.is_valid()) {
        return SAIL_ERROR_BROKEN_IMAGE;
    }

    // Convert to RGBA
    //
    SAIL_TRY(image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA));

    // Construct QImage from the loaded image.
    //
    *qimage = QImage(reinterpret_cast<const uchar *>(image.pixels()),
                     image.width(),
                     image.height(),
                     image.bytes_per_line(),
                     QImage::Format_RGBA8888).copy();

    m_ui->labelStatus->setText(tr("%1  [%2x%3]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image.width())
                                .arg(image.height())
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

    SAIL_TRY(image_output.save(path.toLocal8Bit().constData(), image));

    return SAIL_OK;
}

QStringList QtSail::filters() const
{
    return QStringList { QStringLiteral("All Files (*.*)") };
}

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

    if ((res = loadImage(path, &m_qimage)) == SAIL_OK) {
        fit();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }
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

    if ((res = saveImage(path, m_qimage)) != SAIL_OK) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    QMessageBox::information(this, tr("Success"), tr("%1 has been saved succesfully.").arg(path));
}

void QtSail::fit()
{
    QPixmap pixmap;

    if (m_qimage.width() > m_ui->scrollArea->viewport()->width() ||
            m_qimage.height() > m_ui->scrollArea->viewport()->height()) {
        pixmap = QPixmap::fromImage(m_qimage.scaled(m_ui->scrollArea->viewport()->size(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
    } else {
        pixmap =  QPixmap::fromImage(m_qimage);
    }

    qobject_cast<QLabel *>(m_ui->scrollArea->widget())->setPixmap(pixmap);
}
