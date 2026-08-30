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

#pragma once

#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>

#include <sail-common/status.h>

namespace Ui
{
class ImageViewer;
}

class QImage;
class QLabel;
class QTimer;

/*
 * A widget with everything the Qt examples need except the SAIL calls: file dialogs,
 * scaling, frame navigation, animation playback, and error reporting.
 *
 * Every example subclasses it and implements loading and saving with its own SAIL API.
 */
class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    enum Feature
    {
        NoFeatures = 0,

        /* Show the 'Probe file...' button. Requires probeImage() to be implemented. */
        Probe = 1 << 0,

        /* Show the frame navigation controls and play animations. */
        MultiFrame = 1 << 1,
    };
    Q_DECLARE_FLAGS(Features, Feature)

    explicit ImageViewer(Features features, QWidget* parent = nullptr);
    ~ImageViewer() override;

protected:
    /*
     * Loads all the frames of the specified file. Single-paged examples append a single frame.
     *
     * Delays are in milliseconds. Non-animated frames have a zero delay.
     */
    virtual sail_status_t loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays) = 0;

    /*
     * Saves the currently displayed image into the specified file.
     */
    virtual sail_status_t saveImage(const QString& path, const QImage& qimage) = 0;

    /*
     * Fills in a human readable description of the specified file. Called with the Probe feature only.
     * The elapsed time is measured and reported by the viewer.
     */
    virtual sail_status_t probeImage(const QString& path, QString* info);

    /*
     * Returns the file dialog filters. The default implementation returns 'All Files (*.*)'.
     * The result is queried once and cached.
     */
    virtual QStringList filters() const;

    /*
     * Updates the status bar at the bottom of the widget.
     */
    void setStatus(const QString& text);

private: // slots
    void onOpenFile();
    void onProbe();
    void onSave();
    void onFit();
    void onPrevious();
    void onNext();
    void onStop();

private:
    void openImage(const QString& path);
    void updateImage();
    void updateFrameLabel();
    void detectAnimated();
    const QStringList& cachedFilters() const;

private:
    QScopedPointer<Ui::ImageViewer> m_ui;
    QScopedPointer<QTimer> m_animationTimer;
    QLabel* m_imageLabel;

    const Features m_features;

    QVector<QImage> m_qimages;
    QVector<int> m_delays;
    int m_currentIndex = 0;
    bool m_animated    = false;

    mutable QStringList m_filters;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImageViewer::Features)
