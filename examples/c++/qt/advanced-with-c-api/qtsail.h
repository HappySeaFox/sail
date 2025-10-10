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
#include <QVector>
#include <QWidget>

#include <sail-common/status.h>

namespace Ui
{
class QtSail;
}

class QImage;
class QTimer;

class QtSail : public QWidget
{
    Q_OBJECT

public:
    QtSail(QWidget* parent = nullptr);
    ~QtSail();

private:
    sail_status_t init();
    sail_status_t loadImage(const QString& path, QVector<QImage>* qimages, QVector<int>* delays);
    sail_status_t saveImage(const QString& path, const QImage& qimage);
    QStringList filters() const;
    void detectAnimated();
    void updateCurrentFrameLabel();

private: // slots
    void onOpenFile();
    sail_status_t onProbe();
    void onSave();
    void onFit(bool fit);
    void onPrevious();
    void onNext();
    void onStop();

private:
    QScopedPointer<Ui::QtSail> m_ui;

    QVector<QImage> m_qimages;
    QVector<int> m_delays;
    bool m_animated;
    QScopedPointer<QTimer> m_animationTimer;
    int m_currentIndex = 0;
    QString m_suffix;
};
