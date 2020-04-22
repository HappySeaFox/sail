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
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>

#include <sail/sail-common.h>
#include <sail/sail-c++.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "ui_qtsail.h"

// PIMPL
//
class Q_DECL_HIDDEN QtSail::Private
{
public:
    QScopedPointer<Ui::QtSail> ui;

    QImage qimage;

    sail::context context;
};

QtSail::QtSail(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    d->ui.reset(new Ui::QtSail);
    d->ui->setupUi(this);
    QLabel *l = new QLabel;
    l->setAlignment(Qt::AlignCenter);
    d->ui->scrollArea->setWidget(l);

    connect(d->ui->pushOpen,  &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(d->ui->pushProbe, &QPushButton::clicked, this, &QtSail::onProbe);
    connect(d->ui->pushSave,  &QPushButton::clicked, this, &QtSail::onSave);
    connect(d->ui->checkFit,  &QCheckBox::toggled,   this, &QtSail::onFit);

    d->ui->pushOpen->setShortcut(QKeySequence::Open);
    d->ui->pushOpen->setToolTip(d->ui->pushOpen->shortcut().toString());
    d->ui->pushSave->setShortcut(QKeySequence::Save);
    d->ui->pushSave->setToolTip(d->ui->pushSave->shortcut().toString());

    init();
}

QtSail::~QtSail()
{
}

sail_error_t QtSail::init()
{
    if (!d->context.is_valid()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL"));
        ::exit(1);
    }

    return 0;
}

static QImage::Format sailPixelFormatToQImageFormat(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_MONO:      return QImage::Format_Mono;
        case SAIL_PIXEL_FORMAT_GRAYSCALE: return QImage::Format_Grayscale8;
        case SAIL_PIXEL_FORMAT_INDEXED:   return QImage::Format_Indexed8;
        case SAIL_PIXEL_FORMAT_RGB:       return QImage::Format_RGB888;
        case SAIL_PIXEL_FORMAT_RGBX:      return QImage::Format_RGBX8888;
        case SAIL_PIXEL_FORMAT_RGBA:      return QImage::Format_RGBA8888;

        default: return QImage::Format_Invalid;
    }
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    sail::image_reader reader(&d->context);
    sail::image *image = nullptr;

    // Auto cleanup when the method exits.
    //
    SAIL_AT_SCOPE_EXIT (
        delete image;
    );

    SAIL_TRY(reader.start_reading(path.toLocal8Bit().constData()));
    SAIL_TRY(reader.read_next_frame(&image));
    SAIL_TRY(reader.stop_reading());

    *qimage = QImage(reinterpret_cast<const uchar *>(image->bits()),
                     image->width(),
                     image->height(),
                     image->bytes_per_line(),
                     sailPixelFormatToQImageFormat(image->pixel_format())).copy();

    d->ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width())
                                .arg(image->height())
                                .arg(sail_pixel_format_to_string(image->source_pixel_format()))
                                .arg(sail_pixel_format_to_string(image->pixel_format()))
                                );

    return 0;
}

static int qImageFormatToSailPixelFormat(QImage::Format format) {
    switch (format) {
        case QImage::Format_Mono:       return SAIL_PIXEL_FORMAT_MONO;
        case QImage::Format_Grayscale8: return SAIL_PIXEL_FORMAT_GRAYSCALE;
        case QImage::Format_Indexed8:   return SAIL_PIXEL_FORMAT_INDEXED;
        case QImage::Format_RGB888:     return SAIL_PIXEL_FORMAT_RGB;
        case QImage::Format_RGBX8888:   return SAIL_PIXEL_FORMAT_RGBX;
        case QImage::Format_RGBA8888:   return SAIL_PIXEL_FORMAT_RGBA;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

sail_error_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    sail::image_writer writer(&d->context);
    sail::image image;

    int bytes_per_line;
    SAIL_TRY(sail::image::bytes_per_line(image, &bytes_per_line));

    image.with_width(qimage.width())
         .with_height(qimage.height())
         .with_pixel_format(qImageFormatToSailPixelFormat(qimage.format()))
         .with_bytes_per_line(bytes_per_line)
         .with_shallow_bits(qimage.bits());

    SAIL_TRY(writer.start_writing(path.toLocal8Bit().constData()));
    SAIL_TRY(writer.write_next_frame(&image));
    SAIL_TRY(writer.stop_writing());

    return 0;
}

QStringList QtSail::filters() const
{
    const std::vector<sail::plugin_info> plugin_info_list = d->context.plugin_info_list();
    QStringList filters;

    for (const sail::plugin_info &plugin_info : plugin_info_list) {
        QStringList masks;

        const std::vector<std::string> extensions = plugin_info.extensions();

        for (const std::string &extension : extensions) {
            masks.append(QStringLiteral("*.%1").arg(extension.c_str()));
        }

        filters.append(QStringLiteral("%1 (%2)")
                       .arg(plugin_info.description().c_str())
                       .arg(masks.join(QStringLiteral(" "))));
    }

    return filters;
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

    if ((res = loadImage(path, &d->qimage)) == 0) {
        onFit(d->ui->checkFit->isChecked());
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }
}

void QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    sail::image_reader reader(&d->context);
    sail::image *image = nullptr;
    sail::plugin_info *plugin_info = nullptr;

    // Auto cleanup when the method exits.
    //
    SAIL_AT_SCOPE_EXIT (
        delete image;
        delete plugin_info;
    );

    sail_error_t res;

    if ((res = reader.probe(path.toLocal8Bit().constData(), &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return;
    }

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsedTimer.elapsed())
                                .arg(plugin_info->description().c_str())
                                .arg(image->width())
                                .arg(image->height())
                                .arg(sail_pixel_format_to_string(image->source_pixel_format()))
                                .arg(sail_pixel_format_to_string(image->pixel_format()))
                             );
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

    if ((res = saveImage(path, d->qimage)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if ((res = loadImage(path, &d->qimage)) == 0) {
            onFit(d->ui->checkFit->isChecked());
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                                  .arg(path)
                                  .arg(res));
            return;
        }
    }
}

void QtSail::onFit(bool fit)
{
    QPixmap pixmap;

    if (fit) {
        if (d->qimage.width() > d->ui->scrollArea->viewport()->width() ||
                d->qimage.height() > d->ui->scrollArea->viewport()->height()) {
            pixmap = QPixmap::fromImage(d->qimage.scaled(d->ui->scrollArea->viewport()->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
        } else {
            pixmap =  QPixmap::fromImage(d->qimage);
        }
    } else {
        pixmap =  QPixmap::fromImage(d->qimage);
    }

    qobject_cast<QLabel *>(d->ui->scrollArea->widget())->setPixmap(pixmap);
}
