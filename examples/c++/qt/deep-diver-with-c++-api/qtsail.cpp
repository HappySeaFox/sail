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

#include <sail/sail.h>
#include <sail-c++/sail-c++.h>

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
}

sail_error_t QtSail::init()
{
    if (m_context.status() != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL. Error: %d").arg(m_context.status()));
        ::exit(1);
    }

    return 0;
}

sail_error_t QtSail::loadImage(const QString &path, QImage *qimage)
{
    sail::image_reader reader(&m_context);

    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    // Find the codec info by a file extension.
    //
    sail::plugin_info plugin_info;
    SAIL_TRY(m_context.plugin_info_from_path(path.toLocal8Bit(), &plugin_info));
    pluginInfo(plugin_info);

    // Allocate new read options and copy defaults from the read features
    // (preferred output pixel format etc.).
    //
    sail::read_options read_options;
    SAIL_TRY(plugin_info.read_features().to_read_options(&read_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    ReadOptions readOptions(plugin_info.description().c_str(), plugin_info.read_features(), this);

    if (readOptions.exec() == QDialog::Accepted) {
        read_options.with_output_pixel_format(readOptions.pixelFormat());
    }

    elapsed.restart();

    // Read the whole file into a memory buffer.
    //
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the file: %1").arg(file.errorString()));
        return SAIL_IO_EOF;
    }

    const QByteArray buf = file.readAll();

    // Initialize reading with our options.
    //
    SAIL_TRY(reader.start_reading(buf, buf.length(), plugin_info, read_options));

    // Seek and read the next image frame in the file.
    //
    sail::image image;

    // Read just the first frame in the image.
    //
    SAIL_TRY(reader.read_next_frame(&image));

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image.pixel_format());

    if (qimageFormat == QImage::Format_Invalid) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    // Convert to QImage.
    //
    *qimage = QImage(reinterpret_cast<uchar *>(image.bits()),
                     image.width(),
                     image.height(),
                     image.bytes_per_line(),
                     qimageFormat).copy();

    SAIL_LOG_DEBUG("Has ICC profile: %s (%u bytes)",
                   image.iccp().is_valid() ? "yes" : "no",
                   image.iccp().data_length());

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

        qimage->setColorTable(colorTable);
    }

    // Finish reading.
    //
    SAIL_TRY(reader.stop_reading());

    SAIL_LOG_INFO("Loaded in %lld ms.", elapsed.elapsed() + beforeDialog);

    QString meta;
    const std::map<std::string, std::string> meta_entries = image.meta_entries();

    if (!meta_entries.empty()) {
        const std::pair<std::string, std::string> first_pair = *meta_entries.begin();
        meta = tr("%1: %2")
                .arg(first_pair.first.c_str())
                .arg(QString(first_pair.second.c_str()).left(24).replace('\n', ' '));
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    m_suffix = QFileInfo(path).suffix();

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image.width())
                                .arg(image.height())
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                .arg(meta)
                                );

    // Optional: unload all plugins to free up some memory.
    //
    m_context.unload_plugins();

    return 0;
}

sail_error_t QtSail::saveImage(const QImage &qimage, void *buffer, size_t buffer_length,
                               size_t *written)
{
    sail::image_writer writer(&m_context);

    // Create a new image to be passed into the SAIL writing functions.
    //
    sail::image image;
    image.with_width(qimage.width())
         .with_height(qimage.height())
         .with_pixel_format(qImageFormatToSailPixelFormat(qimage.format()))
         .with_bytes_per_line_auto()
         .with_shallow_bits(qimage.bits());

    if (image.pixel_format() == SAIL_PIXEL_FORMAT_UNKNOWN) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    sail::plugin_info plugin_info;
    SAIL_TRY(m_context.plugin_info_from_extension(m_suffix.toUtf8(), &plugin_info));
    pluginInfo(plugin_info);

    // Allocate new write options and copy defaults from the write features
    // (preferred output pixel format etc.).
    //
    sail::write_options write_options;
    SAIL_TRY(plugin_info.write_features().to_write_options(&write_options));

    const qint64 beforeDialog = elapsed.elapsed();

    // Ask the user to provide his/her preferred output options.
    //
    WriteOptions writeOptions(QString::fromUtf8(plugin_info.description().c_str()),
                              plugin_info.write_features(),
                              image.pixel_format(),
                              this);

    if (writeOptions.exec() == QDialog::Accepted) {
        write_options
                .with_output_pixel_format(writeOptions.pixelFormat())
                .with_compression(writeOptions.compression());
    }

    elapsed.restart();

    // Initialize writing with our options.
    //
    SAIL_TRY(writer.start_writing(buffer, buffer_length, plugin_info, write_options));

    // Save some meta info...
    //
    if (write_options.io_options() & SAIL_IO_OPTION_META_INFO) {
        std::map<std::string, std::string> meta_entries {
            { "Comment", "SAIL demo comment" }
        };
        image.with_meta_entries(meta_entries);
    }

    const char *output_pixel_format_str;
    SAIL_TRY(sail::image::pixel_format_to_string(write_options.output_pixel_format(), &output_pixel_format_str));

    SAIL_LOG_DEBUG("Image size: %dx%d", image.width(), image.height());
    SAIL_LOG_DEBUG("Output pixel format: %s", output_pixel_format_str);

    // Seek and write the next image frame into the file.
    //
    SAIL_TRY(writer.write_next_frame(image));

    // Finish writing.
    //
    SAIL_TRY(writer.stop_writing(written));

    SAIL_LOG_INFO("Saved in %lld ms.", elapsed.elapsed() + beforeDialog);

    // Optional: unload all plugins to free up some memory.
    //
    m_context.unload_plugins();

    return 0;
}

sail_error_t QtSail::pluginInfo(const sail::plugin_info &plugin_info) const
{
    SAIL_LOG_DEBUG("SAIL plugin version: %s", plugin_info.version().c_str());
    SAIL_LOG_DEBUG("SAIL plugin description: %s", plugin_info.description().c_str());
    SAIL_LOG_DEBUG("SAIL plugin path: %s", plugin_info.path().c_str());

    const std::vector<std::string> extensions = plugin_info.extensions();

    for (const std::string &extension : extensions) {
        SAIL_LOG_DEBUG("SAIL extension '%s'", extension.c_str());
    }

    const std::vector<std::string> mime_types = plugin_info.mime_types();

    for (const std::string &mime_type : mime_types) {
        SAIL_LOG_DEBUG("SAIL mime type '%s'", mime_type.c_str());
    }

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
    sail_error_t res;
    sail::image_reader reader(&m_context);

    sail::image image;
    sail::plugin_info plugin_info;

    if ((res = reader.probe(path.toLocal8Bit(), &image, &plugin_info)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return res;
    }

    pluginInfo(plugin_info);

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    SAIL_TRY(sail::image::pixel_format_to_string(image.source_pixel_format(), &source_pixel_format_str));
    SAIL_TRY(sail::image::pixel_format_to_string(image.pixel_format(), &pixel_format_str));

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsed.elapsed())
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
