/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <cstring>

#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QTimer>

#include <sail/sail.h>

#include <sail-manip/sail-manip.h>

// #define SAIL_CODEC_NAME jpeg
// #include <sail/layout/v8.h>

#include "qimage_sail_pixel_formats.h"
#include "qtsail.h"

QtSail::QtSail(QWidget* parent)
    : ImageViewer(Probe | MultiFrame, parent)
{
    setWindowTitle(tr("Qt SAIL demo [advanced, C API]"));

    QTimer::singleShot(0, this, [this] {
        QMessageBox::information(this, tr("Features"),
                                 tr("This demo includes:"
                                    "<ul>"
                                    "<li>Linking against SAIL CMake packages</li>"
                                    "<li>Playing animations</li>"
                                    "<li>Conversion with alpha blending</li>"
                                    "</ul>"));
    });
}

sail_status_t QtSail::loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays)
{
    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void* state = nullptr;

    /*
     * Starts loading the specified file.
     */
    SAIL_TRY(sail_start_loading_from_file(path.toLocal8Bit(), nullptr, &state));

    /*
     * Save the properties of the first frame for displaying in the status bar.
     */
    SailPixelFormat source_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    SailPixelFormat pixel_format        = SAIL_PIXEL_FORMAT_UNKNOWN;
    unsigned width                      = 0;
    unsigned height                     = 0;

    /*
     * Load all the available image frames in the file.
     */
    sail_status_t res;
    struct sail_image* image;

    while ((res = sail_load_next_frame(state, &image)) == SAIL_OK)
    {
        /* Mutate alpha into a green color. */
        const struct sail_conversion_options options = {
            SAIL_CONVERSION_OPTION_BLEND_ALPHA, {0, 255 * 257, 0}, {0, 255, 0}};

        struct sail_image* image_converted;
        SAIL_TRY_OR_CLEANUP(
            sail_convert_image_with_options(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &options, &image_converted),
            /* cleanup */ sail_stop_loading(state), sail_destroy_image(image));

        if (qimages->isEmpty())
        {
            source_pixel_format = image->source_image->pixel_format;
            pixel_format        = image_converted->pixel_format;
            width               = image_converted->width;
            height              = image_converted->height;
        }

        /*
         * Convert to QImage.
         */
        qimages->append(QImage(reinterpret_cast<const uchar*>(image_converted->pixels), image_converted->width,
                               image_converted->height, image_converted->bytes_per_line, QImage::Format_RGB888)
                            .copy());
        delays->append(image->delay);

        sail_destroy_image(image_converted);
        sail_destroy_image(image);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES)
    {
        sail_stop_loading(state);
        SAIL_LOG_AND_RETURN(res);
    }

    SAIL_LOG_DEBUG("Loaded images: %d", qimages->size());

    /*
     * Finish loading.
     */
    SAIL_TRY(sail_stop_loading(state));

    /*
     * Empty files are reported by the viewer.
     */
    if (qimages->isEmpty())
    {
        return SAIL_OK;
    }

    setStatus(tr("%1  [%2x%3]  [%4 → %5]")
                  .arg(QFileInfo(path).fileName())
                  .arg(width)
                  .arg(height)
                  .arg(sail_pixel_format_to_string(source_pixel_format))
                  .arg(sail_pixel_format_to_string(pixel_format)));

    return SAIL_OK;
}

sail_status_t QtSail::saveImage(const QString& path, const QImage& qimage)
{
    const SailPixelFormat qimage_pixel_format = qImageFormatToSailPixelFormat(qimage.format());

    if (qimage_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    const struct sail_codec_info* codec_info;
    SAIL_TRY(sail_codec_info_from_path(path.toLocal8Bit(), &codec_info));

    struct sail_image* image;
    SAIL_TRY(sail_alloc_image(&image));

    image->width        = qimage.width();
    image->height       = qimage.height();
    image->pixel_format = qimage_pixel_format;

    /*
     * QImage pads its scan lines, so take the line size from QImage instead of computing
     * the tightly packed one with sail_bytes_per_line(). SAIL accesses pixels line by line.
     */
    image->bytes_per_line = qimage.bytesPerLine();

    const size_t pixels_size = static_cast<size_t>(image->bytes_per_line) * image->height;

    /*
     * Pixels are freed by SAIL, so allocate them with SAIL as well. Mixing allocators
     * from different modules crashes on some platforms.
     */
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image->pixels),
                        /* cleanup */ sail_destroy_image(image));

    memcpy(image->pixels, qimage.constBits(), pixels_size);

    /*
     * SAIL tries to save an image as is, preserving its pixel format.
     * Particular image formats may support saving in different pixel formats:
     * RGB, Grayscale, etc. Convert the image to the best pixel format for saving here.
     *
     * You can prepare the image for saving by converting its pixel format on your own,
     * without using sail-manip.
     */
    struct sail_image* image_converted;
    SAIL_TRY_OR_CLEANUP(sail_convert_image_for_saving(image, codec_info->save_features, &image_converted),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);
    image = image_converted;

    /*
     * Create save options to pass PNG filters.
     */
    struct sail_save_options* save_options;
    SAIL_TRY_OR_CLEANUP(sail_alloc_save_options_from_features(codec_info->save_features, &save_options),
                        /* cleanup */ sail_destroy_image(image));

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&save_options->tuning),
                        /* cleanup */ sail_destroy_save_options(save_options), sail_destroy_image(image));

    /*
     * This filter will be handled and applied by the PNG codec only.
     * Possible values: "none", "sub", "up", "avg", "paeth".
     *
     * It's also possible to combine PNG filters with ';' like that:
     * "none;sub;paeth"
     */
    struct sail_variant* value;
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&value),
                        /* cleanup */ sail_destroy_save_options(save_options), sail_destroy_image(image));

    sail_set_variant_string(value, "none;sub");
    sail_put_hash_map(save_options->tuning, "png-filter", value);
    sail_destroy_variant(value);

    /*
     * Always set the initial state to NULL in C or nullptr in C++.
     */
    void* state = nullptr;

    SAIL_TRY_OR_CLEANUP(sail_start_saving_into_file_with_options(path.toLocal8Bit(), nullptr, save_options, &state),
                        /* cleanup */ sail_destroy_save_options(save_options), sail_destroy_image(image));

    sail_destroy_save_options(save_options);

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image),
                        /* cleanup */ sail_stop_saving(state), sail_destroy_image(image));
    SAIL_TRY_OR_CLEANUP(sail_stop_saving(state),
                        /* cleanup */ sail_destroy_image(image));

    sail_destroy_image(image);

    return SAIL_OK;
}

sail_status_t QtSail::probeImage(const QString& path, QString* info)
{
    struct sail_image* image;
    const struct sail_codec_info* codec_info;
    SAIL_TRY(sail_probe_file(path.toLocal8Bit(), &image, &codec_info));

    *info = tr("Codec: %1\nSize: %2x%3\nSource compression: %4\nSource pixel format: %5\nOutput pixel format: %6")
                .arg(codec_info->description)
                .arg(image->width)
                .arg(image->height)
                .arg(sail_compression_to_string(image->source_image->compression))
                .arg(sail_pixel_format_to_string(image->source_image->pixel_format))
                .arg(sail_pixel_format_to_string(image->pixel_format));

    sail_destroy_image(image);

    return SAIL_OK;
}

QStringList QtSail::filters() const
{
    QStringList filters{QStringLiteral("All Files (*.*)")};

    for (const sail_codec_bundle_node* codec_bundle_node = sail_codec_bundle_list(); codec_bundle_node != nullptr;
         codec_bundle_node                               = codec_bundle_node->next)
    {
        const sail_codec_info* codec_info = codec_bundle_node->codec_bundle->codec_info;

        QStringList masks;

        for (const sail_string_node* extension_node = codec_info->extension_node; extension_node != nullptr;
             extension_node                         = extension_node->next)
        {
            masks.append(QStringLiteral("*.%1").arg(extension_node->string));
        }

        filters.append(QStringLiteral("%1: %2 (%3)")
                           .arg(codec_info->name)
                           .arg(codec_info->description)
                           .arg(masks.join(QStringLiteral(" "))));
    }

    return filters;
}
