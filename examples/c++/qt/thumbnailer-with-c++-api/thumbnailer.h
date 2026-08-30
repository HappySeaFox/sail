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

#pragma once

#include <QFileIconProvider>
#include <QScopedPointer>
#include <QSet>
#include <QString>
#include <QThread>
#include <QWidget>

namespace Ui
{
class Thumbnailer;
}

class QEvent;
class QImage;
class QModelIndex;
class QStandardItemModel;

class ThumbnailLoader;

/*
 * A folder view with image thumbnails decoded by SAIL in a worker thread. Files SAIL has
 * no codec for are shown with the system icons provided by Qt.
 */
class Thumbnailer : public QWidget
{
    Q_OBJECT

public:
    explicit Thumbnailer(QWidget* parent = nullptr);
    ~Thumbnailer() override;

protected:
    void changeEvent(QEvent* event) override;

signals:
    void thumbnailRequested(const QString& path, int row, int generation);

private: // slots
    void onOpenFolder();
    void onActivated(const QModelIndex& index);
    void onThumbnailLoaded(int row, int generation, const QImage& thumbnail);

private:
    void openFolder(const QString& path);
    void updateGridSize();

private:
    QScopedPointer<Ui::Thumbnailer> m_ui;
    QStandardItemModel* m_model;
    QFileIconProvider m_iconProvider;

    /* Lower case file extensions SAIL has codecs for. */
    QSet<QString> m_extensions;

    QThread m_thread;
    ThumbnailLoader* m_loader;

    QString m_path;

    /* Incremented on every folder change to invalidate the thumbnails still being decoded. */
    int m_generation = 0;
};
