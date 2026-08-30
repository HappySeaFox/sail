/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

#include <QImage>

#include <sail-c++/sail-c++.h>

#include "thumbnail_loader.h"

ThumbnailLoader::ThumbnailLoader(const QSize& thumbnailSize, QObject* parent)
    : QObject(parent)
    , m_thumbnailSize(thumbnailSize)
    , m_generation(0)
{
}

void ThumbnailLoader::setGeneration(int generation)
{
    m_generation.storeRelease(generation);
}

void ThumbnailLoader::load(const QString& path, int row, int generation)
{
    // The user has navigated to another folder already, drop the outdated request.
    //
    if (generation != m_generation.loadAcquire())
    {
        return;
    }

    // Load the first frame of the file. Failures are not reported: such files just keep
    // the system icon assigned to them by the GUI thread.
    //
    sail::image image;

    if (image.load(path.toLocal8Bit().constData()) != SAIL_OK)
    {
        return;
    }

    if (image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA) != SAIL_OK)
    {
        return;
    }

    // QImage shares the pixels with the SAIL image, scaled() makes an independent copy of them.
    //
    const QImage qimage(reinterpret_cast<const uchar*>(image.pixels()), image.width(), image.height(),
                        image.bytes_per_line(), QImage::Format_RGBA8888);

    emit loaded(row, generation, qimage.scaled(m_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
