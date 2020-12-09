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

//#define SAIL_CODEC_NAME jpeg
//#include <sail/layouts/v4.h>

#include "qtsail.h"
#include "ui_qtsail.h"
#include "qimage_sail_pixel_formats.h"

#include "multi-paged-impl.cpp"

QtSail::~QtSail()
{
    sail_finish();
}

sail_status_t QtSail::init()
{
    QTimer::singleShot(0, this, [&]{
        QMessageBox::information(this, tr("Features"), tr("This demo includes:"
                                                          "<ul>"
                                                          "<li>Linking against SAIL pkg-config packages</li>"
                                                          "<li>Output RGB or RGBA pixels only</li>"
                                                          "<li>Playing animations</li>"
                                                          "</ul>"
                                                          "This demo doesn't include:"
                                                          "<ul>"
                                                          "<li>Selecting pixel format to output</li>"
                                                          "<li>Printing all meta data entries into stderr</li>"
                                                          "</ul>"));
    });

    return SAIL_OK;
}

sail_status_t QtSail::loadImage(const QString &path, QVector<QImage> *qimages, QVector<int> *delays)
{
    qimages->clear();
    delays->clear();

    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void *state = NULL;

    /*
     * Save this for displaying in the status bar.
     */
    SailPixelFormat source_pixel_format;
    SailPixelFormat pixel_format;
    unsigned width, height;

    /*
     * Starts reading the specified file.
     * The subsequent calls to sail_read_next_frame() output pixels in the BPP32-RGBA pixel format.
     */
    SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path.toLocal8Bit(), NULL, &state),
                        /* cleanup */ sail_stop_reading(state));

    /*
     * Read all the available image frames in the file.
     */
    sail_status_t res;
    struct sail_image *image;

    while ((res = sail_read_next_frame(state, &image)) == SAIL_OK) {

        const QImage::Format qimageFormat = sailPixelFormatToQImageFormat(image->pixel_format);

        if (qimageFormat == QImage::Format_Invalid) {
            sail_stop_reading(state);
            sail_destroy_image(image);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }

        source_pixel_format = image->source_image->pixel_format;
        pixel_format = image->pixel_format;
        width = image->width;
        height = image->height;

        /*
         * Convert to QImage.
         */
        QImage qimage = QImage(reinterpret_cast<uchar *>(image->pixels),
                               image->width,
                               image->height,
                               image->bytes_per_line,
                               qimageFormat).copy();

        qimages->append(qimage);
        delays->append(image->delay);

        sail_destroy_image(image);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES) {
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

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 → %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(width)
                                .arg(height)
                                .arg(source_pixel_format_str)
                                .arg(pixel_format_str)
                                );

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void *state = nullptr;

    struct sail_image *image;
    SAIL_TRY(sail_alloc_image(&image));

    const int sizeInBytes = qimage.bytesPerLine() * qimage.height();
    image->pixels = malloc(sizeInBytes);
    memcpy(image->pixels, qimage.bits(), sizeInBytes);
    image->width = qimage.width();
    image->height = qimage.height();
    image->pixel_format = qImageFormatToSailPixelFormat(qimage.format());

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image->width, image->pixel_format, &image->bytes_per_line),
                        /* cleanup */sail_destroy_image(image));

    SAIL_TRY_OR_CLEANUP(sail_start_writing_file(path.toLocal8Bit(), nullptr, &state),
                        /* cleanup */ sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image),
                        /* cleanup */ sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_stop_writing(state),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

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

    if ((res = loadImage(path, &m_qimages, &m_delays)) == SAIL_OK) {
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

sail_status_t QtSail::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty()) {
        return SAIL_OK;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    // Probe
    sail_image *image;
    const struct sail_codec_info *codec_info;
    sail_status_t res;

    if ((res = sail_probe_file(path.toLocal8Bit(), &image, &codec_info)) != SAIL_OK) {
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
    const QString path = QFileDialog::getSaveFileName(this,
                                                      tr("Select a file"),
                                                      QString(),
                                                      filters().join(QStringLiteral(";;")));

    if (path.isEmpty()) {
        return;
    }

    sail_status_t res;

    if ((res = saveImage(path, m_qimages.first())) != SAIL_OK) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.")
                              .arg(path)
                              .arg(res));
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if ((res = loadImage(path, &m_qimages, &m_delays)) == SAIL_OK) {
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
