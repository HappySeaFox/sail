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

#include <sail/sail.h>

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

    sail_context *context = nullptr;

    QFileInfoList files;
    int currentFile;
};

// Auto cleanup at scope exit
//
template<typename T>
class CleanUp
{
public:
    CleanUp(T func)
        : m_func(func)
    {}

    ~CleanUp()
    {
        m_func();
    }

private:
    T m_func;
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

    connect(d->ui->pushOpen,     &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(d->ui->pushOpenDir,  &QPushButton::clicked, this, &QtSail::onOpenDir);
    connect(d->ui->pushProbe,    &QPushButton::clicked, this, &QtSail::onProbe);
    connect(d->ui->pushSave   ,  &QPushButton::clicked, this, &QtSail::onSave);
    connect(d->ui->pushPrevious, &QPushButton::clicked, this, &QtSail::onPrevious);
    connect(d->ui->pushNext,     &QPushButton::clicked, this, &QtSail::onNext);
    connect(d->ui->pushFirst,    &QPushButton::clicked, this, &QtSail::onFirst);
    connect(d->ui->pushLast,     &QPushButton::clicked, this, &QtSail::onLast);
    connect(d->ui->checkFit,     &QCheckBox::toggled,   this, &QtSail::onFit);

    d->ui->pushOpen->setShortcut(QKeySequence::Open);
    d->ui->pushOpen->setToolTip(d->ui->pushOpen->shortcut().toString());
    d->ui->pushSave->setShortcut(QKeySequence::Save);
    d->ui->pushSave->setToolTip(d->ui->pushSave->shortcut().toString());
    d->ui->pushPrevious->setShortcut(Qt::Key_Backspace);
    d->ui->pushPrevious->setToolTip(d->ui->pushPrevious->shortcut().toString());
    d->ui->pushNext->setShortcut(Qt::Key_Space);
    d->ui->pushNext->setToolTip(d->ui->pushNext->shortcut().toString());
    d->ui->pushFirst->setShortcut(Qt::Key_Home);
    d->ui->pushFirst->setToolTip(d->ui->pushFirst->shortcut().toString());
    d->ui->pushLast->setShortcut(Qt::Key_End);
    d->ui->pushLast->setToolTip(d->ui->pushLast->shortcut().toString());

    connect(new QShortcut(Qt::Key_F, this), &QShortcut::activated, this, [&]{
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });

    init();
}

QtSail::~QtSail()
{
    SAIL_LOG_INFO("Finish");
    sail_finish(d->context);
    d->context = nullptr;
}

sail_error_t QtSail::init()
{
    SAIL_LOG_INFO("Init");
    SAIL_TRY_OR_CLEANUP(sail_init(&d->context),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));
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
        case SAIL_PIXEL_FORMAT_ARGB:      return QImage::Format_ARGB32;

        default: return QImage::Format_Invalid;
    }
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    qint64 v = QDateTime::currentMSecsSinceEpoch();

    const struct sail_plugin_info *plugin_info;
    void *pimpl;

    struct sail_image *image = nullptr;
    uchar *image_bits = nullptr;

    auto cleanup_func = [&] {
        SAIL_LOG_DEBUG("Read clean up");

        free(image_bits);

        sail_destroy_image(image);
        image = nullptr;
    };

    CleanUp<decltype(cleanup_func)> cleanUp(cleanup_func);

    SAIL_TRY(sail_start_reading(path.toLocal8Bit(), d->context, &plugin_info/* or NULL */, &pimpl));
    SAIL_TRY(sail_read_next_frame(pimpl, &image, (void **)&image_bits));
    SAIL_TRY(sail_stop_reading(pimpl));

    SAIL_LOG_INFO("Loaded in %lld ms.", QDateTime::currentMSecsSinceEpoch() - v);

    *qimage = QImage(image_bits,
                     image->width,
                     image->height,
                     image->bytes_per_line,
                     sailPixelFormatToQImageFormat(image->pixel_format)).copy();

    QString meta;
    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != nullptr) {
        meta = tr("%1: %2").arg(node->key).arg(node->value);
    }

    d->ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                .arg(sail_pixel_format_to_string(image->source_pixel_format))
                                .arg(sail_pixel_format_to_string(image->pixel_format))
                                .arg(meta)
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
        case QImage::Format_ARGB32:     return SAIL_PIXEL_FORMAT_ARGB;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

sail_error_t QtSail::saveImage(const QString &path, QImage *qimage)
{
    qint64 v = QDateTime::currentMSecsSinceEpoch();

    const struct sail_plugin_info *plugin_info;
    void *pimpl;

    struct sail_image *image = nullptr;

    auto cleanup_func = [&] {
        SAIL_LOG_DEBUG("Read clean up");

        sail_destroy_image(image);
        image = nullptr;
    };

    SAIL_TRY(sail_alloc_image(&image));

    image->width = qimage->width();
    image->height = qimage->height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage->format());
    image->passes = 1;

    CleanUp<decltype(cleanup_func)> cleanUp(cleanup_func);

    SAIL_TRY(sail_start_writing(path.toLocal8Bit(), d->context, &plugin_info/* or NULL */, &pimpl));
    SAIL_TRY(sail_write_next_frame(pimpl, image, qimage->bits()));
    SAIL_TRY(sail_stop_writing(pimpl));

    SAIL_LOG_INFO("Saved in %lld ms.", QDateTime::currentMSecsSinceEpoch() - v);

    return 0;
}

void QtSail::loadFileFromDir()
{
    if (d->currentFile < 0 || d->currentFile >= d->files.size()) {
        return;
    }

    d->ui->labelCounter->setText(tr("%1/%2").arg(d->currentFile+1).arg(d->files.size()));

    if (loadImage(d->files[d->currentFile].absoluteFilePath(), &d->qimage) == 0) {
        onFit(d->ui->checkFit->isChecked());
    }
}

QStringList QtSail::filters() const
{
    QStringList filters;
    sail_plugin_info_node *plugin_info_node = d->context->plugin_info_node;

    while (plugin_info_node != nullptr) {
        QStringList masks;

        sail_string_node *extension_node = plugin_info_node->plugin_info->extension_node;

        while (extension_node != nullptr) {
            masks.append(QStringLiteral("*.%1").arg(extension_node->value));
            extension_node = extension_node->next;
        }

        filters.append(QStringLiteral("%1 (%2)")
                       .arg(plugin_info_node->plugin_info->description)
                       .arg(masks.join(QStringLiteral(" "))));

        plugin_info_node = plugin_info_node->next;
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

    d->files.clear();

    int res;

    if ((res = loadImage(path, &d->qimage)) == 0) {
        onFit(d->ui->checkFit->isChecked());
        d->ui->labelCounter->setText(QStringLiteral("1/1"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }
}

void QtSail::onOpenDir()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Select a file"));

    if (path.isEmpty()) {
        return;
    }

    d->files = QDir(path).entryInfoList(QStringList() << QStringLiteral("*.*"),
                                        QDir::Files,
                                        QDir::Name);
    d->currentFile = 0;
    loadFileFromDir();
}

void QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return;
    }

    const qint64 v = QDateTime::currentMSecsSinceEpoch();

    // Probe
    sail_image *image;
    const struct sail_plugin_info *plugin_info;
    int res;

    if ((res = sail_probe_image(path.toLocal8Bit(), d->context, &plugin_info, &image)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return;
    }

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(QDateTime::currentMSecsSinceEpoch() - v)
                                .arg(plugin_info->description)
                                .arg(image->width)
                                .arg(image->height)
                                .arg(sail_pixel_format_to_string(image->source_pixel_format))
                                .arg(sail_pixel_format_to_string(image->pixel_format))
                             );

    sail_destroy_image(image);
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

    int res;

    if ((res = saveImage(path, &d->qimage)) == 0) {
        onFit(d->ui->checkFit->isChecked());
        d->ui->labelCounter->setText(QStringLiteral("1/1"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if (!loadImage(path, &d->qimage)) {
            return;
        }
    }
}

void QtSail::onPrevious()
{
    d->currentFile--;
    loadFileFromDir();
}

void QtSail::onNext()
{
    d->currentFile++;
    loadFileFromDir();
}

void QtSail::onFirst()
{
    d->currentFile = 0;
    loadFileFromDir();
}

void QtSail::onLast()
{
    d->currentFile = d->files.size()-1;
    loadFileFromDir();
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
