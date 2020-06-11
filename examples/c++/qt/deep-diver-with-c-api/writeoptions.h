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

#ifndef WRITEOPTIONS_H
#define WRITEOPTIONS_H

#include <QDialog>
#include <QScopedPointer>

#include <sail-common/common.h>
#include <sail-common/error.h>

struct sail_write_features;

class WriteOptions : public QDialog
{
    Q_OBJECT

public:
    explicit WriteOptions(const QString &codecDescription,
                          const sail_write_features *write_features,
                          int input_pixel_format,
                          QWidget *parent = nullptr);
    ~WriteOptions();

    SailPixelFormat pixelFormat() const;

    int compression() const;

private:
    sail_error_t init(const sail_write_features *write_features, int input_pixel_format);
    void disable();

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif // WRITEOPTIONS_H
