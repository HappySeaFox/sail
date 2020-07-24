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

#ifndef QT_SAIL_H
#define QT_SAIL_H

#include <QWidget>
#include <QScopedPointer>
#include <QImage>

#include <sail-common/error.h>

namespace Ui {
    class QtSail;
}

class QtSail : public QWidget
{
    Q_OBJECT

public:
    QtSail(QWidget *parent = nullptr);
    ~QtSail();

private:
    sail_status_t loadImage(const QString &path, QImage *qimage);
    sail_status_t saveImage(const QString &path, const QImage &qimage);
    QStringList filters() const;

private: // slots
    void onOpenFile();
    void onSave();

private:
    void fit();

private:
    QScopedPointer<Ui::QtSail> m_ui;

    QImage m_qimage;
};

#endif
