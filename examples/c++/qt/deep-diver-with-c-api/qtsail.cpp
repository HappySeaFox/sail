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

#include <sail/sail.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "readoptions.h"
#include "writeoptions.h"
#include "ui_qtsail.h"

// PIMPL
//
class Q_DECL_HIDDEN QtSail::Private
{
public:
    QScopedPointer<Ui::QtSail> ui;

    QImage qimage;

    sail_context *context = nullptr;
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

sail_error_t QtSail::init()
{
    SAIL_TRY_OR_CLEANUP(sail_init(&d->context),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));
    return 0;
}

QtSail::~QtSail()
{
    sail_finish(d->context);
    d->context = nullptr;
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
    sail_read_options *read_options = nullptr;

    sail_image *image = nullptr;
    uchar *image_bits = nullptr;
    void *pimpl = nullptr;

    SAIL_TRY_OR_CLEANUP(loadImageImpl(path, &read_options, &pimpl, &image, &image_bits),
                        /* cleanup */ free(image_bits),
                                      sail_stop_reading(pimpl),
                                      sail_destroy_read_options(read_options),
                                      sail_destroy_image(image));

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(read_options->pixel_format);

    // Convert to QImage.
    //
    *qimage = QImage(image_bits,
                     image->width,
                     image->height,
                     sail_bytes_per_line(image->width, image->pixel_format),
                     qimageFormat).copy();

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

    free(image_bits);

    sail_destroy_read_options(read_options);
    sail_destroy_image(image);

    /* Optional: unload all plugins to free up some memory. */
    sail_unload_plugins(d->context);

    return 0;
}

sail_error_t QtSail::loadImageImpl(const QString &path, sail_read_options **read_options, void **pimpl,
                                   sail_image **image, uchar **image_bits)
{
    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    // Find the codec info by a file extension.
    //
    const struct sail_plugin_info *plugin_info;
    SAIL_TRY(sail_plugin_info_by_extension(d->context, QFileInfo(path).suffix().toLocal8Bit(), &plugin_info));

    // Load the found codec.
    //
    const struct sail_plugin *plugin;
    SAIL_TRY(sail_load_plugin(d->context, plugin_info, &plugin));
    pluginInfo(plugin_info);

    // Allocate new read options and copy defaults from the read features
    // (preferred output pixel format etc.).
    //
    SAIL_TRY(sail_alloc_read_options_from_features(plugin_info->read_features, read_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    ReadOptions readOptions(QString::fromUtf8(plugin_info->description), plugin_info->read_features, this);

    if (readOptions.exec() == QDialog::Accepted) {
        (*read_options)->pixel_format = readOptions.pixelFormat();
    } else {
        // Force RGB888 output format.
        //
        (*read_options)->pixel_format = SAIL_PIXEL_FORMAT_RGB;
    }

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat((*read_options)->pixel_format);

    if (qimageFormat == QImage::Format_Invalid) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    elapsed.restart();

    // Initialize reading with our options.
    //
    SAIL_TRY(sail_start_reading_with_plugin(path.toLocal8Bit(), d->context, plugin_info, plugin, *read_options, pimpl));

    // Seek and read the next image frame in the file.
    //
    SAIL_TRY(sail_read_next_frame(*pimpl, image, reinterpret_cast<void **>(image_bits)));

    // Finish reading.
    //
    SAIL_TRY(sail_stop_reading(*pimpl));

    SAIL_LOG_INFO("Loaded in %lld ms.", elapsed.elapsed() + beforeDialog);

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
    sail_image *image = nullptr;
    sail_write_options *write_options = nullptr;
    void *pimpl = nullptr;

    // Create a new image to be passed into the SAIL writing functions.
    //
    SAIL_TRY(sail_alloc_image(&image));

    image->width = qimage.width();
    image->height = qimage.height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage.format());
    image->bytes_per_line = sail_bytes_per_line(image->width, image->pixel_format);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_destroy_image(image);
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    SAIL_TRY_OR_CLEANUP(saveImageImpl(path, &write_options, &pimpl, image, qimage.bits()),
                        /* cleanup */ sail_stop_writing(pimpl),
                                      sail_destroy_write_options(write_options),
                                      sail_destroy_image(image));

    sail_destroy_write_options(write_options);
    sail_destroy_image(image);

    /* Optional: unload all plugins to free up some memory. */
    sail_unload_plugins(d->context);

    return 0;
}

sail_error_t QtSail::saveImageImpl(const QString &path, sail_write_options **write_options, void **pimpl,
                                   sail_image *image, const uchar *image_bits)
{
    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    const struct sail_plugin_info *plugin_info;
    SAIL_TRY(sail_plugin_info_by_extension(d->context, QFileInfo(path).suffix().toLocal8Bit(), &plugin_info));

    const struct sail_plugin *plugin;
    SAIL_TRY(sail_load_plugin(d->context, plugin_info, &plugin));
    pluginInfo(plugin_info);

    // Allocate new write options and copy defaults from the write features
    // (preferred output pixel format etc.).
    //
    SAIL_TRY(sail_alloc_write_options_from_features(plugin_info->write_features, write_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    WriteOptions writeOptions(QString::fromUtf8(plugin_info->description), plugin_info->write_features, this);

    if (writeOptions.exec() == QDialog::Accepted) {
        (*write_options)->pixel_format = writeOptions.pixelFormat();
        (*write_options)->compression = writeOptions.compression();
    }

    elapsed.restart();

    // Initialize writing with our options.
    //
    SAIL_TRY(sail_start_writing_with_plugin(path.toLocal8Bit(), d->context, plugin_info, plugin, *write_options, pimpl));

    // Save some meta info...
    //
    if ((*write_options)->io_options & SAIL_IO_OPTION_META_INFO) {
        struct sail_meta_entry_node *meta_entry_node;

        SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
        SAIL_TRY(sail_strdup("Comment", &meta_entry_node->key));
        SAIL_TRY(sail_strdup("SAIL demo comment", &meta_entry_node->value));

        image->meta_entry_node = meta_entry_node;
    }

    SAIL_LOG_DEBUG("Image size: %dx%d", image->width, image->height);
    SAIL_LOG_DEBUG("Output pixel format: %s", sail_pixel_format_to_string((*write_options)->pixel_format));

    // Seek and write the next image frame into the file.
    //
    SAIL_TRY(sail_write_next_frame(*pimpl, image, image_bits));

    // Finish writing.
    //
    SAIL_TRY(sail_stop_writing(*pimpl));

    SAIL_LOG_INFO("Saved in %lld ms.", elapsed.elapsed() + beforeDialog);

    return 0;
}

sail_error_t QtSail::pluginInfo(const struct sail_plugin_info *plugin_info) const
{
    SAIL_LOG_DEBUG("SAIL plugin layout version: %d", plugin_info->layout);
    SAIL_LOG_DEBUG("SAIL plugin version: %s", plugin_info->version);
    SAIL_LOG_DEBUG("SAIL plugin description: %s", plugin_info->description);
    SAIL_LOG_DEBUG("SAIL plugin path: %s", plugin_info->path);

    const struct sail_string_node *node = plugin_info->extension_node;

    while (node != nullptr) {
        SAIL_LOG_DEBUG("SAIL extension '%s'", node->value);
        node = node->next;
    }

    node = plugin_info->mime_type_node;

    while (node != nullptr) {
        SAIL_LOG_DEBUG("SAIL mime type '%s'", node->value);
        node = node->next;
    }

    return 0;
}

QStringList QtSail::filters() const
{
    QStringList filters;
    const sail_plugin_info_node *plugin_info_node = sail_plugin_info_list(d->context);

    while (plugin_info_node != nullptr) {
        QStringList masks;

        sail_string_node *extension_node = plugin_info_node->plugin_info->extension_node;

        while (extension_node != nullptr) {
            masks.append(QStringLiteral("*.%1").arg(extension_node->value));
            extension_node = extension_node->next;
        }

        filters.append(QStringLiteral("%1: %2 (%3)")
                       .arg(plugin_info_node->plugin_info->name)
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

    QElapsedTimer elapsed;
    elapsed.start();

    // Probe
    sail_image *image;
    const struct sail_plugin_info *plugin_info;
    sail_error_t res;

    if ((res = sail_probe(path.toLocal8Bit(), d->context, &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return;
    }

    pluginInfo(plugin_info);

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsed.elapsed())
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
