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

#include <sail-manip/sail-manip.h>

//#define SAIL_CODEC_NAME jpeg
//#include <sail/layout/v7.h>

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
                                                          "<li>Linking against SAIL CMake packages</li>"
                                                          "<li>Playing animations</li>"
                                                          "<li>Conversion with alpha blending</li>"
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
    unsigned width = 0, height = 0;

    /*
     * Starts loading the specified file.
     */
    SAIL_TRY_OR_CLEANUP(sail_start_loading_file(path.toLocal8Bit(), NULL, &state),
                        /* cleanup */ sail_stop_loading(state));

    /*
     * Load all the available image frames in the file.
     */
    sail_status_t res;
    struct sail_image *image;

    while ((res = sail_load_next_frame(state, &image)) == SAIL_OK) {

        /* Mutate alpha into a green color. */
        const struct sail_conversion_options options = {
            SAIL_CONVERSION_OPTION_BLEND_ALPHA,
            { 0, 255 * 257, 0 },
            { 0, 255, 0 }
        };

        struct sail_image *image_converted;
        SAIL_TRY_OR_CLEANUP(sail_convert_image_with_options(image,
                                                            SAIL_PIXEL_FORMAT_BPP24_RGB,
                                                            &options,
                                                            &image_converted),
                            /* cleanup */ sail_stop_loading(state),
                                          sail_destroy_image(image));

        if (width == 0) {
            source_pixel_format = image->source_image->pixel_format;
            pixel_format = image_converted->pixel_format;
            width = image_converted->width;
            height = image_converted->height;
        }

        /*
         * Convert to QImage.
         */
        QImage qimage = QImage(reinterpret_cast<uchar *>(image_converted->pixels),
                               image_converted->width,
                               image_converted->height,
                               image_converted->bytes_per_line,
                               QImage::Format_RGB888).copy();

        qimages->append(qimage);
        delays->append(image->delay);

        sail_destroy_image(image_converted);
        sail_destroy_image(image);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES) {
        sail_stop_loading(state);
        return res;
    }

    SAIL_LOG_DEBUG("Loaded images: %d", qimages->size());

    /*
     * Finish loading.
     */
    SAIL_TRY(sail_stop_loading(state));

    m_ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 → %5]")
                                .arg(QFileInfo(path).fileName())
                                .arg(width)
                                .arg(height)
                                .arg(sail_pixel_format_to_string(source_pixel_format))
                                .arg(sail_pixel_format_to_string(pixel_format))
                                );

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString &path, const QImage &qimage)
{
    const struct sail_codec_info *codec_info;
    SAIL_TRY(sail_codec_info_from_path(path.toLocal8Bit(), &codec_info));

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
                        /* cleanup */ sail_destroy_image(image));

    /*
     * SAIL tries to save an image as is, preserving its pixel format.
     * Particular image formats may support saving in different pixel formats:
     * RGB, Grayscale, etc. Convert the image to the best pixel format for saving here.
     *
     * You can prepare the image for saving by converting its pixel format on your own,
     * without using sail-manip.
     */
    struct sail_image *image_converted;
    SAIL_TRY_OR_CLEANUP(sail_convert_image_for_saving(image, codec_info->save_features, &image_converted),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);
    image = image_converted;

    /*
     * Create save options to pass PNG filters.
     */
    struct sail_save_options *save_options;
    SAIL_TRY_OR_CLEANUP(sail_alloc_save_options_from_features(codec_info->save_features, &save_options),
                        /* cleanup */ sail_destroy_image(image));

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&save_options->tuning),
                        /* cleanup */ sail_destroy_save_options(save_options),
                                      sail_destroy_image(image));

    /*
     * This filter will be handled and applied by the PNG codec only.
     * Possible values: "none", "sub", "up", "avg", "paeth".
     *
     * It's also possible to combine PNG filters with ';' like that:
     * "none;sub;paeth"
     */
    struct sail_variant *value;
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&value),
                        /* cleanup */ sail_destroy_save_options(save_options),
                                      sail_destroy_image(image));

    sail_set_variant_string(value, "none;sub");
    sail_put_hash_map(save_options->tuning, "png-filter", value);
    sail_destroy_variant(value);

    SAIL_TRY_OR_CLEANUP(sail_start_saving_file_with_options(path.toLocal8Bit(), nullptr, save_options, &state),
                        /* cleanup */ sail_destroy_save_options(save_options),
                                      sail_destroy_image(image));

    sail_destroy_save_options(save_options);

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image),
                        /* cleanup */ sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_stop_saving(state),
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

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(elapsedTimer.elapsed())
                                .arg(codec_info->description)
                                .arg(image->width)
                                .arg(image->height)
                                .arg(sail_pixel_format_to_string(image->source_image->pixel_format))
                                .arg(sail_pixel_format_to_string(image->pixel_format))
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
