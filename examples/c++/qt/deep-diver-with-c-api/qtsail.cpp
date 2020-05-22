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

#include <sail-common/sail-common.h>
#include <sail/sail.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "readoptions.h"
#include "writeoptions.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

// PIMPL
//
class Q_DECL_HIDDEN QtSail::Private
{
public:
    QScopedPointer<Ui::QtSail> ui;

    QImage qimage;
    QString suffix;

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
    SAIL_TRY_OR_CLEANUP(sail_init_with_flags(&d->context, SAIL_FLAG_PRELOAD_PLUGINS),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));
    return 0;
}

QtSail::~QtSail()
{
    sail_finish(d->context);
    d->context = nullptr;
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    sail_read_options *read_options = nullptr;

    sail_image *image = nullptr;
    uchar *image_bits = nullptr;

    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void *state = nullptr;

    /*
     *  Time counter.
     */
    QElapsedTimer elapsed;
    elapsed.start();

    /*
     * Find the codec info by a file extension.
     */
    const struct sail_plugin_info *plugin_info;
    SAIL_TRY(sail_plugin_info_from_path(path.toLocal8Bit(), d->context, &plugin_info));

    pluginInfo(plugin_info);

    /*
     * Allocate new read options and copy defaults from the plugin-specific read features
     * (preferred output pixel format etc.).
     */
    SAIL_TRY(sail_alloc_read_options_from_features(plugin_info->read_features, &read_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    ReadOptions readOptions(QString::fromUtf8(plugin_info->description), plugin_info->read_features, this);

    if (readOptions.exec() == QDialog::Accepted) {
        read_options->output_pixel_format = readOptions.pixelFormat();
    }

    elapsed.restart();

    /*
     * Read the whole file into a memory buffer.
     */
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the file: %1").arg(file.errorString()));
        return SAIL_IO_EOF;
    }

    const QByteArray buf = file.readAll();

    /*
     * Initialize reading with our options. The options will be deep copied.
     */
    SAIL_TRY_OR_CLEANUP(sail_start_reading_mem_with_options(buf,
                                                            buf.length(),
                                                            d->context,
                                                            plugin_info,
                                                            read_options,
                                                            &state),
                        /* cleanup */ sail_destroy_read_options(read_options));

    /*
     * Our read options are not needed anymore.
     */
    sail_destroy_read_options(read_options);

    /*
     * Read the next available image frame in the file.
     */
    SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state,
                                             &image,
                                             (void **)&image_bits),
                        /* cleanup */ sail_stop_reading(state));

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image->pixel_format);

    if (qimageFormat == QImage::Format_Invalid) {
        sail_stop_reading(state);
        sail_destroy_image(image);
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    /*
     * Finish reading.
     */
    SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                        /* cleanup */ free(image_bits),
                                      sail_destroy_image(image));

    SAIL_LOG_INFO("Loaded in %lld ms.", elapsed.elapsed() + beforeDialog);

    /*
     * Convert to QImage.
     */
    *qimage = QImage(image_bits,
                     image->width,
                     image->height,
                     image->bytes_per_line,
                     qimageFormat).copy();

    /*
     * Apply palette.
     */
    if (qimageFormat == QImage::Format_Indexed8) {
        /*
         * Assume palette is BPP24-RGB.
         */
        if (image->palette_pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB) {
            free(image_bits);
            sail_destroy_image(image);
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        QVector<QRgb> colorTable;
        unsigned char *palette = reinterpret_cast<unsigned char *>(image->palette);

        for (int i = 0; i < image->palette_color_count; i++) {
            colorTable.append(qRgb(*palette, *(palette+1), *(palette+2)));
            palette += 3;
        }

        qimage->setColorTable(colorTable);
    }

    QString meta;
    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != nullptr) {
        meta = tr("%1: %2").arg(node->key).arg(node->value);
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    sail_pixel_format_to_string(image->source_pixel_format, &source_pixel_format_str);
    sail_pixel_format_to_string(image->pixel_format, &pixel_format_str);

    d->ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                .arg(meta)
                                );

    d->suffix = QFileInfo(path).suffix();

    free(image_bits);
    sail_destroy_image(image);

    /* Optional: unload all plugins to free up some memory. */
    sail_unload_plugins(d->context);

    return 0;
}

sail_error_t QtSail::saveImage(const QImage &qimage, void *buffer, size_t buffer_length,
                               size_t *written)
{
    /*
     * WARNING: Memory cleanup on error is not implemented in this demo. Please don't forget
     * to free memory (pointers, image bits etc.) on error in a real application.
     */

    sail_image *image = nullptr;
    sail_write_options *write_options = nullptr;

    // Always set the initial state to NULL in C or nullptr in C++.
    //
    void *state = nullptr;

    // Create a new image to be passed into the SAIL writing functions.
    //
    SAIL_TRY(sail_alloc_image(&image));

    image->width = qimage.width();
    image->height = qimage.height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage.format());
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &image->bytes_per_line));

    if (image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_destroy_image(image);
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    const struct sail_plugin_info *plugin_info;
    SAIL_TRY(sail_plugin_info_from_extension(d->suffix.toUtf8(), d->context, &plugin_info));

    pluginInfo(plugin_info);

    // Allocate new write options and copy defaults from the write features
    // (preferred output pixel format etc.).
    //
    SAIL_TRY(sail_alloc_write_options_from_features(plugin_info->write_features, &write_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    WriteOptions writeOptions(QString::fromUtf8(plugin_info->description),
                              plugin_info->write_features,
                              image->pixel_format,
                              this);

    if (writeOptions.exec() == QDialog::Accepted) {
        write_options->output_pixel_format = writeOptions.pixelFormat();
        write_options->compression = writeOptions.compression();
    }

    elapsed.restart();

    // Initialize writing with our options.
    //
    SAIL_TRY(sail_start_writing_mem_with_options(buffer,
                                                  buffer_length,
                                                  d->context,
                                                  plugin_info,
                                                  write_options,
                                                  &state));

    // Save some meta info...
    //
    if (write_options->io_options & SAIL_IO_OPTION_META_INFO) {
        struct sail_meta_entry_node *meta_entry_node;

        SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
        SAIL_TRY(sail_strdup("Comment", &meta_entry_node->key));
        SAIL_TRY(sail_strdup("SAIL demo comment", &meta_entry_node->value));

        image->meta_entry_node = meta_entry_node;
    }

    const char *pixel_format_str;
    SAIL_TRY(sail_pixel_format_to_string(write_options->output_pixel_format, &pixel_format_str));

    SAIL_LOG_DEBUG("Image size: %dx%d", image->width, image->height);
    SAIL_LOG_DEBUG("Output pixel format: %s", pixel_format_str);

    // Seek and write the next image frame into the file.
    //
    SAIL_TRY(sail_write_next_frame(state, image, qimage.bits()));

    // Finish writing.
    //
    SAIL_TRY(sail_stop_writing_with_written(state, written));

    SAIL_LOG_INFO("Saved in %lld ms.", elapsed.elapsed() + beforeDialog);

    sail_destroy_write_options(write_options);
    sail_destroy_image(image);

    /* Optional: unload all plugins to free up some memory. */
    sail_unload_plugins(d->context);

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

sail_error_t QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return 0;
    }

    QElapsedTimer elapsed;
    elapsed.start();

    // Probe
    sail_image *image;
    const struct sail_plugin_info *plugin_info;
    sail_error_t res;

    if ((res = sail_probe(path.toLocal8Bit(), d->context, &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return res;
    }

    pluginInfo(plugin_info);

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail_pixel_format_to_string(image->source_pixel_format, &source_pixel_format_str));
    SAIL_TRY(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsed.elapsed())
                                .arg(plugin_info->description)
                                .arg(image->width)
                                .arg(image->height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                             );

    sail_destroy_image(image);

    return 0;
}

void QtSail::onSave()
{
    sail_error_t res;

    /*
     * Allocate 10 Mb.
     */
    size_t buffer_length = 10*1024*1024;
    char *buffer = new char [buffer_length];

    size_t written;

    if ((res = saveImage(d->qimage, buffer, buffer_length, &written)) != 0) {
        delete [] buffer;
        QMessageBox::critical(this, tr("Error"), tr("Failed to save to memory buffer. Error: %1.")
                              .arg(res));
        return;
    }

    QMessageBox::information(this,
                             tr("Success"),
                             tr("The image has been saved into a memory buffer. Saved bytes: %1")
                             .arg(written));

    delete [] buffer;
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
