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

#include <algorithm>
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
#include <QTimer>

#include <sail/sail.h>

//#include <sail/layouts/v2.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

#include "multi-paged-impl.cpp"

QtSail::~QtSail()
{
    sail_finish(m_context);
    m_context = nullptr;
}

sail_error_t QtSail::init()
{
    SAIL_TRY_OR_CLEANUP(sail_init(&m_context),
                        /* cleanup */ QMessageBox::critical(this, tr("Error"), tr("Failed to init SAIL")),
                                      ::exit(1));
    return 0;
}

sail_error_t QtSail::loadImage(const QString &path, QVector<QImage> *qimages, QVector<int> *delays)
{
    qimages->clear();
    delays->clear();

    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void *state = NULL;
    struct sail_image *image = NULL;
    uchar *image_bits = NULL;

    /*
     * Save this for displaying in the status bar.
     */
    SailPixelFormat source_pixel_format;
    SailPixelFormat pixel_format;
    unsigned width, height;

    /*
     * Starts reading the specified file.
     * The subsequent calls to sail_read_next_frame() will output pixels in BPP32-RGBA pixel format for image formats
     * with transparency support and BPP24-RGB otherwise.
     */
    SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path.toLocal8Bit(), m_context, NULL, &state),
                        /* cleanup */ sail_stop_reading(state));

    /*
     * Read all the available image frames in the file.
     */
    sail_error_t res = 0;
    while ((res = sail_read_next_frame(state,
                                       &image,
                                       (void **)&image_bits)) == 0) {

        const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image->pixel_format);

        if (qimageFormat == QImage::Format_Invalid) {
            sail_stop_reading(state);
            sail_destroy_image(image);
            free(image_bits);
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        source_pixel_format = image->source_image->pixel_format;
        pixel_format = image->pixel_format;
        width = image->width;
        height = image->height;

        /*
         * Convert to QImage.
         */
        QImage qimage = QImage(image_bits,
                               image->width,
                               image->height,
                               image->bytes_per_line,
                               qimageFormat).copy();

        qimages->append(qimage);
        delays->append(image->delay);

        sail_destroy_image(image);
        free(image_bits);
    }

    if (res != SAIL_NO_MORE_FRAMES) {
        sail_stop_reading(state);
        return res;
    }

    SAIL_LOG_DEBUG("Read images: %d", qimages->size());

    /*
     * Finish reading.
     */
    SAIL_TRY(sail_stop_reading(state));

    const char *source_pixel_format_str = NULL;
    const char *pixel_format_str = NULL;

    sail_pixel_format_to_string(source_pixel_format, &source_pixel_format_str);
    sail_pixel_format_to_string(pixel_format, &pixel_format_str);

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(width)
                                .arg(height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                );

    return 0;
}

sail_error_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void *state = nullptr;

    struct sail_image *image = nullptr;
    SAIL_TRY(sail_alloc_image(&image));

    image->width = qimage.width();
    image->height = qimage.height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage.format());

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image->width, image->pixel_format, &image->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image));

    SAIL_TRY_OR_CLEANUP(sail_start_writing_file(path.toLocal8Bit(), m_context, nullptr, &state),
                        /* cleanup */ sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image, qimage.bits()),
                        /* cleanup */ sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_stop_writing(state),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

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

    if ((res = loadImage(path, &m_qimages, &m_delays)) == 0) {
        m_currentIndex = 0;
        onFit(m_ui->checkFit->isChecked());
        detectAnimated();
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

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    // Probe
    sail_image *image;
    const struct sail_plugin_info *plugin_info;
    sail_error_t res;

    if ((res = sail_probe_path(path.toLocal8Bit(), m_context, &image, &plugin_info)) != 0) {
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
                                .arg(elapsedTimer.elapsed())
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
    const QString path = QFileDialog::getSaveFileName(this,
                                                      tr("Select a file"),
                                                      QString(),
                                                      filters().join(QStringLiteral(";;")));

    if (path.isEmpty()) {
        return;
    }

    sail_error_t res;

    if ((res = saveImage(path, m_qimages.first())) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if ((res = loadImage(path, &m_qimages, &m_delays)) == 0) {
            m_currentIndex = 0;
            onFit(m_ui->checkFit->isChecked());
            detectAnimated();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.")
                                  .arg(path)
                                  .arg(res));
            return;
        }
    }
}
