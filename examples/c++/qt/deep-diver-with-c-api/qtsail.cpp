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
#include <QTimer>

#include <sail-common/sail-common.h>
#include <sail/sail.h>

//#define SAIL_CODEC_NAME jpeg
//#include <sail/layouts/v4.h>

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
    connect(m_ui->pushProbe, &QPushButton::clicked, this, &QtSail::onProbe);
    connect(m_ui->pushSave,  &QPushButton::clicked, this, &QtSail::onSave);
    connect(m_ui->checkFit,  &QCheckBox::toggled,   this, &QtSail::onFit);

    init();
}

QtSail::~QtSail()
{
    sail_finish();
}

sail_status_t QtSail::init()
{
    SAIL_TRY_OR_CLEANUP(sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));

    QTimer::singleShot(0, this, [&]{
        QMessageBox::information(this, tr("Features"), tr("This demo includes:"
                                                          "<ul>"
                                                          "<li>Linking against SAIL CMake packages</li>"
                                                          "<li>Printing all meta data entries into stderr</li>"
                                                          "</ul>"
                                                          "This demo doesn't include:"
                                                          "<ul>"
                                                          "<li>Playing animations</li>"
                                                          "</ul>"));
    });

    return SAIL_OK;
}

sail_status_t QtSail::loadImage(const QString &path, QImage *qimage)
{
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
     * Find the codec info by a file magic number.
     * See https://en.wikipedia.org/wiki/File_format#Magic_number.
     */
    const struct sail_codec_info *codec_info;
    SAIL_TRY(sail_codec_info_by_magic_number_from_path(path.toLocal8Bit(), &codec_info));

    sail_read_options *read_options;

    /*
     * Allocate new read options and copy defaults from the codec-specific read features
     * (preferred output pixel format etc.).
     */
    SAIL_TRY(sail_alloc_read_options_from_features(codec_info->read_features, &read_options));

    const qint64 beforeDialog = elapsed.elapsed();

    elapsed.restart();

    /*
     * Read the whole file into a memory buffer.
     */
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the file: %1").arg(file.errorString()));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EOF);
    }

    const QByteArray buf = file.readAll();

    /*
     * Initialize reading with our options. The options will be deep copied.
     */
    SAIL_TRY_OR_CLEANUP(sail_start_reading_mem_with_options(buf,
                                                            buf.length(),
                                                            codec_info,
                                                            read_options,
                                                            &state),
                        /* cleanup */ sail_destroy_read_options(read_options));

    /*
     * Our read options are not needed anymore.
     */
    sail_destroy_read_options(read_options);

    sail_image *image;

    /*
     * Read just the first frame in the image.
     */
    SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image),
                        /* cleanup */ sail_stop_reading(state));

    const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image->pixel_format);

    if (qimageFormat == QImage::Format_Invalid) {
        sail_stop_reading(state);
        sail_destroy_image(image);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /*
     * Convert to QImage.
     */
    *qimage = QImage(reinterpret_cast<uchar *>(image->pixels),
                     image->width,
                     image->height,
                     image->bytes_per_line,
                     qimageFormat).copy();

    SAIL_LOG_DEBUG("Has ICC profile: %s (%u bytes)",
                   image->iccp == NULL ? "no" : "yes",
                   image->iccp == NULL ? 0 : image->iccp->data_length);

    /*
     * Apply palette.
     */
    if (qimageFormat == QImage::Format_Indexed8) {
        /*
         * Assume palette is BPP24-RGB or BPP32-RGBA.
         */
        unsigned palette_shift;
        if (image->palette->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB) {
            palette_shift = 3;
        } else if (image->palette->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
            palette_shift = 4;
        } else {
            sail_stop_reading(state);
            sail_destroy_image(image);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }

        QVector<QRgb> colorTable;
        const unsigned char *palette = reinterpret_cast<const unsigned char *>(image->palette->data);

        for (unsigned i = 0; i < image->palette->color_count; i++) {
            colorTable.append(qRgb(*palette, *(palette+1), *(palette+2)));
            palette += palette_shift;
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
    struct sail_meta_data_node *node = image->meta_data_node;

    if (node != nullptr) {
        const char *meta_data_str = nullptr;

        if (node->key == SAIL_META_DATA_UNKNOWN) {
            meta_data_str = node->key_unknown;
        } else {
            SAIL_TRY_OR_SUPPRESS(sail_meta_data_to_string(node->key, &meta_data_str));
        }

        if (meta.isEmpty() && node->value_type == SAIL_META_DATA_TYPE_STRING) {
            meta = tr("%1: %2").arg(meta_data_str).arg(QString((const char *)node->value).left(24).replace('\n', ' '));
        }

        while (node != nullptr) {
            if (node->value_type == SAIL_META_DATA_TYPE_STRING) {
                SAIL_LOG_DEBUG("[META] %s: %s", meta_data_str, (const char *)node->value);
            } else {
                SAIL_LOG_DEBUG("[META] %s: <%u bytes>", meta_data_str, (unsigned)node->value_length);
            }
            node = node->next;
        }
    }

    const char *source_pixel_format_str;
    const char *pixel_format_str;

    sail_pixel_format_to_string(image->source_image->pixel_format, &source_pixel_format_str);
    sail_pixel_format_to_string(image->pixel_format, &pixel_format_str);

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 → %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                .arg(meta)
                                );

    sail_destroy_image(image);

    m_suffix = QFileInfo(path).suffix();

    /* Optional: unload all codecs to free up some memory. */
    sail_unload_codecs();

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QImage &qimage, void *buffer, size_t buffer_length,
                               size_t *written)
{
    /*
     * WARNING: Memory cleanup on error is not fully implemented in this demo. Please don't forget
     * to free memory (pointers, image pixels etc.) on error in a real application.
     *
     * For example, you can use SAIL_TRY_OR_CLEANUP().
     */

    // Always set the initial state to NULL in C or nullptr in C++.
    //
    void *state = nullptr;

    // Create a new image to be passed into the SAIL writing functions.
    //
    sail_image *image;
    SAIL_TRY(sail_alloc_image(&image));

    const int sizeInBytes = qimage.bytesPerLine() * qimage.height();
    image->pixels = malloc(sizeInBytes);
    memcpy(image->pixels, qimage.bits(), sizeInBytes);
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
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
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
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    // Time counter.
    //
    QElapsedTimer elapsed;
    elapsed.start();

    const struct sail_codec_info *codec_info;
    SAIL_TRY(sail_codec_info_from_extension(m_suffix.toUtf8(), &codec_info));

    // Allocate new write options and copy defaults from the write features
    // (preferred output pixel format etc.).
    //
    sail_write_options *write_options;
    SAIL_TRY(sail_alloc_write_options_from_features(codec_info->write_features, &write_options));

    const qint64 beforeDialog = elapsed.elapsed();

    elapsed.restart();

    // Initialize writing with our options.
    //
    SAIL_TRY(sail_start_writing_mem_with_options(buffer,
                                                  buffer_length,
                                                  codec_info,
                                                  write_options,
                                                  &state));

    // Save some meta data...
    //
    if (write_options->io_options & SAIL_IO_OPTION_META_DATA) {
        struct sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node_from_known_string(
                     SAIL_META_DATA_SOFTWARE,
                     "SAIL",
                     &meta_data_node));

        image->meta_data_node = meta_data_node;
    }

    SAIL_LOG_DEBUG("Image size: %dx%d", image->width, image->height);

    // Seek and write the next image frame into the file.
    //
    SAIL_TRY(sail_write_next_frame(state, image));

    // Finish writing.
    //
    SAIL_TRY(sail_stop_writing_with_written(state, written));

    SAIL_LOG_INFO("Saved in %lld ms.", elapsed.elapsed() + beforeDialog);

    sail_destroy_write_options(write_options);
    sail_destroy_image(image);

    /* Optional: unload all codecs to free up some memory. */
    sail_unload_codecs();

    return SAIL_OK;
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

    sail_status_t res;

    if ((res = loadImage(path, &m_qimage)) == SAIL_OK) {
        onFit(m_ui->checkFit->isChecked());
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }
}

sail_status_t QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return SAIL_OK;
    }

    QElapsedTimer elapsed;
    elapsed.start();

    // Probe
    sail_image *image;
    const struct sail_codec_info *codec_info;
    sail_status_t res;

    // Load the file into memeory
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the file. Error: %1").arg(file.errorString()));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    const QByteArray buffer = file.readAll();

    // Probe from memory
    if ((res = sail_probe_mem(buffer.constData(), buffer.length(), &image, &codec_info)) != SAIL_OK) {
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
                                .arg(codec_info->description)
                                .arg(image->width)
                                .arg(image->height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                             );

    sail_destroy_image(image);

    return SAIL_OK;
}

void QtSail::onSave()
{
    sail_status_t res;

    /*
     * Allocate 50 Mb.
     */
    const size_t buffer_length = 50*1024*1024;
    char *buffer = new char [buffer_length];

    size_t written;

    if ((res = saveImage(m_qimage, buffer, buffer_length, &written)) != SAIL_OK) {
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
