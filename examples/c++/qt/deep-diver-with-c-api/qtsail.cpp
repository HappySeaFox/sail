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

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include <sail-common/sail-common.h>
#include <sail/sail.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "readoptions.h"
#include "writeoptions.h"
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
    connect(m_ui->pushProbe, &QPushButton::clicked, this, &QtSail::onProbe);
    connect(m_ui->pushSave,  &QPushButton::clicked, this, &QtSail::onSave);
    connect(m_ui->checkFit,  &QCheckBox::toggled,   this, &QtSail::onFit);

    init();
}

QtSail::~QtSail()
{
    sail_finish(m_context);
    m_context = nullptr;
}

sail_error_t QtSail::init()
{
    SAIL_TRY_OR_CLEANUP(sail_init_with_flags(&m_context, SAIL_FLAG_PRELOAD_PLUGINS),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));
    return 0;
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
    SAIL_TRY(sail_plugin_info_from_path(path.toLocal8Bit(), m_context, &plugin_info));

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
                                                            m_context,
                                                            plugin_info,
                                                            read_options,
                                                            &state),
                        /* cleanup */ sail_destroy_read_options(read_options));

    /*
     * Our read options are not needed anymore.
     */
    sail_destroy_read_options(read_options);

    /*
     * Read just the first frame in the image.
     */
    SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state,
                                             &image,
                                             (void **)&image_bits),
                        /* cleanup */ sail_stop_reading(state));

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image->pixel_format);

    if (qimageFormat == QImage::Format_Invalid) {
        sail_stop_reading(state);
        sail_destroy_image(image);
        free(image_bits);
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    /*
     * Convert to QImage.
     */
    *qimage = QImage(image_bits,
                     image->width,
                     image->height,
                     image->bytes_per_line,
                     qimageFormat).copy();

    free(image_bits);

    SAIL_LOG_DEBUG("Has ICC profile: %s (%u bytes)",
                   image->iccp == NULL ? "no" : "yes",
                   image->iccp == NULL ? 0 : image->iccp->data_length);

    /*
     * Apply palette.
     */
    if (qimageFormat == QImage::Format_Indexed8) {
        /*
         * Assume palette is BPP24-RGB.
         */
        if (image->palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB) {
            sail_stop_reading(state);
            sail_destroy_image(image);
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        QVector<QRgb> colorTable;
        const unsigned char *palette = reinterpret_cast<const unsigned char *>(image->palette->data);

        for (unsigned i = 0; i < image->palette->color_count; i++) {
            colorTable.append(qRgb(*palette, *(palette+1), *(palette+2)));
            palette += 3;
        }

        qimage->setColorTable(colorTable);
    }

    /*
     * Finish reading.
     */
    SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                        /* cleanup */ sail_destroy_image(image));

    SAIL_LOG_INFO("Loaded in %lld ms.", elapsed.elapsed() + beforeDialog);

    QString meta;
    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != nullptr) {
        meta = tr("%1: %2").arg(node->key).arg(QString(node->value).left(24).replace('\n', ' '));
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    sail_pixel_format_to_string(image->source_image->pixel_format, &source_pixel_format_str);
    sail_pixel_format_to_string(image->pixel_format, &pixel_format_str);

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                .arg(meta)
                                );

    sail_destroy_image(image);

    m_suffix = QFileInfo(path).suffix();

    /* Optional: unload all plugins to free up some memory. */
    sail_unload_plugins(m_context);

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

    /*
     * Apply palette.
     */
    if (qimage.format() == QImage::Format_Indexed8) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette(&image->palette),
                            /* cleanup */ sail_destroy_image(image));

        const QVector<QRgb> colorTable = qimage.colorTable();

        image->palette->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
        image->palette->color_count = colorTable.size();
        image->palette->data = malloc(3 * image->palette->color_count);

        if (image->palette->data == NULL) {
            sail_destroy_image(image);
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        unsigned char *rgbData = reinterpret_cast<unsigned char *>(image->palette->data);

        for(QRgb rgb : colorTable) {
            *rgbData++ = qRed(rgb);
            *rgbData++ = qGreen(rgb);
            *rgbData++ = qBlue(rgb);
        }
    }

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
    SAIL_TRY(sail_plugin_info_from_extension(m_suffix.toUtf8(), m_context, &plugin_info));

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
                                                  m_context,
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
    sail_unload_plugins(m_context);

    return 0;
}

#include "filters-impl-c.cpp"

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
        onFit(m_ui->checkFit->isChecked());
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

    if ((res = sail_probe(path.toLocal8Bit(), m_context, &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return res;
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail_pixel_format_to_string(image->source_image->pixel_format, &source_pixel_format_str));
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
     * Allocate 50 Mb.
     */
    size_t buffer_length = 50*1024*1024;
    char *buffer = new char [buffer_length];

    size_t written;

    if ((res = saveImage(m_qimage, buffer, buffer_length, &written)) != 0) {
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
        if (m_qimage.width() > m_ui->scrollArea->viewport()->width() ||
                m_qimage.height() > m_ui->scrollArea->viewport()->height()) {
            pixmap = QPixmap::fromImage(m_qimage.scaled(m_ui->scrollArea->viewport()->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
        } else {
            pixmap =  QPixmap::fromImage(m_qimage);
        }
    } else {
        pixmap =  QPixmap::fromImage(m_qimage);
    }

    qobject_cast<QLabel *>(m_ui->scrollArea->widget())->setPixmap(pixmap);
}
