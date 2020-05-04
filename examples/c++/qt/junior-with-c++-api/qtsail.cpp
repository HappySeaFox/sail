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
#include <QShortcut>

#include <sail/sail-common.h>
#include <sail/sail.h>
#include <sail/sail-c++.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

// PIMPL
//
class Q_DECL_HIDDEN QtSail::Private
{
public:
    QScopedPointer<Ui::QtSail> ui;

    QImage qimage;
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
    connect(d->ui->pushSave,  &QPushButton::clicked, this, &QtSail::onSave);

    d->ui->pushOpen->setShortcut(QKeySequence::Open);
    d->ui->pushOpen->setToolTip(d->ui->pushOpen->shortcut().toString());
    d->ui->pushSave->setShortcut(QKeySequence::Save);
    d->ui->pushSave->setToolTip(d->ui->pushSave->shortcut().toString());
}

QtSail::~QtSail()
{
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    sail::image_reader reader;
    sail::image image;

    // read() reads the image and outputs pixels in in RGB pixel format for image formats
    // without transparency support and RGBA otherwise.
    //
    SAIL_TRY(reader.read(path.toLocal8Bit(), &image));

    // Construct QImage from the read image.
    //
    *qimage = QImage(reinterpret_cast<uchar *>(image.bits()),
                     image.width(),
                     image.height(),
                     image.bytes_per_line(),
                     sailPixelFormatToQImageFormat(image.pixel_format())).copy();

    d->ui->labelStatus->setText(tr("%1  [%2x%3]")
                                .arg(QFileInfo(path).fileName())
                                .arg(image.width())
                                .arg(image.height())
                                );

    return 0;
}

sail_error_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    sail::image_writer writer;
    sail::image image;

    image.with_width(qimage.width())
         .with_height(qimage.height())
         .with_pixel_format(qImageFormatToSailPixelFormat(qimage.format()))
         .with_bytes_per_line_auto()
         .with_shallow_bits(qimage.bits());

    SAIL_TRY(writer.write(path.toLocal8Bit(), image));

    return 0;
}

QStringList QtSail::filters() const
{
    // Allocate a local context
    //
    sail::context context;

    const std::vector<sail::plugin_info> plugin_info_list = context.plugin_info_list();
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

    if ((res = saveImage(path, d->qimage)) != 0) {
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

    if (d->qimage.width() > d->ui->scrollArea->viewport()->width() ||
            d->qimage.height() > d->ui->scrollArea->viewport()->height()) {
        pixmap = QPixmap::fromImage(d->qimage.scaled(d->ui->scrollArea->viewport()->size(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
    } else {
        pixmap =  QPixmap::fromImage(d->qimage);
    }

    qobject_cast<QLabel *>(d->ui->scrollArea->widget())->setPixmap(pixmap);
}
