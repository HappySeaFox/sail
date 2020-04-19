/*
    Copyright (c) 2020 Dmitry Baryshev

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

#include <QLabel>
#include <QSlider>

#include <stdio.h>
#include <sail/sail.h>

#include "readoptions.h"
#include "ui_readoptions.h"

class Q_DECL_HIDDEN ReadOptions::Private
{
public:
    QScopedPointer<Ui::ReadOptions> ui;
};

ReadOptions::ReadOptions(const QString &codecDescription, const sail_read_features *read_features, QWidget *parent)
    : QDialog(parent)
    , d(new Private)
{
    d->ui.reset(new Ui::ReadOptions);
    d->ui->setupUi(this);

    d->ui->labelCodec->setText(codecDescription);

    if (read_features->output_pixel_formats_length == 0) {
        d->ui->labelColor->setText(tr("Output color selection is not available"));
        d->ui->labelColor->setEnabled(false);
        d->ui->comboColor->setEnabled(false);
    } else {
        d->ui->labelColor->setText(tr("Output color:"));

        for (int i = 0; i < read_features->output_pixel_formats_length; i++) {
            d->ui->comboColor->addItem(sail_pixel_format_to_string(read_features->output_pixel_formats[i]),
                                       /* user data */read_features->output_pixel_formats[i]);
        }

        d->ui->comboColor->setCurrentText(sail_pixel_format_to_string(read_features->preferred_output_pixel_format));
    }
}

ReadOptions::~ReadOptions()
{
}

int ReadOptions::pixelFormat() const
{
    return d->ui->comboColor->currentData().toInt();
}
