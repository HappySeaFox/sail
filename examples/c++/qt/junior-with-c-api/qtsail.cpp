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

#include <sail/sail.h>

//#include <sail/layouts/v2.h>

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

    connect(m_ui->pushOpen,  &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(m_ui->pushSave,  &QPushButton::clicked, this, &QtSail::onSave);
}

QtSail::~QtSail()
{
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    struct sail_image *image;
    uchar *image_bits;

    /*
     * sail_read() reads the image and outputs pixels in BPP24-RGB pixel format for image formats
     * without transparency support and BPP32-RGBA otherwise.
     */
    SAIL_TRY(sail_read(path.toLocal8Bit(),
                       &image,
                       reinterpret_cast<void **>(&image_bits)));

    // Construct QImage from the read image bits.
    //
    *qimage = QImage(image_bits,
                     image->width,
                     image->height,
                     image->bytes_per_line,
                     sailPixelFormatToQImageFormat(image->pixel_format)).copy();

    m_ui->labelStatus->setText(tr("%1  [%2x%3]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                );
    free(image_bits);
    sail_destroy_image(image);

    return 0;
}

sail_error_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    struct sail_image *image = nullptr;
    SAIL_TRY(sail_alloc_image(&image));

    image->width = qimage.width();
    image->height = qimage.height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage.format());
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image->width, image->pixel_format, &image->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image));

    SAIL_TRY_OR_CLEANUP(sail_write(path.toLocal8Bit(),
                                   image,
                                   reinterpret_cast<const void *>(qimage.bits())),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

    return 0;
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

    sail_error_t res;

    if ((res = loadImage(path, &m_qimage)) == 0) {
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

    sail_error_t res;

    if ((res = saveImage(path, m_qimage)) != 0) {
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
