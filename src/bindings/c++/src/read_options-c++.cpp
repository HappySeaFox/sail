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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN read_options::pimpl
{
public:
    pimpl()
        : output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , io_options(0)
    {}

    SailPixelFormat output_pixel_format;
    int io_options;
};

read_options::read_options()
    : d(new pimpl)
{
}

read_options::read_options(const sail_read_options *ro)
    : read_options()
{
    if (ro == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::read_options(). The object is untouched");
        return;
    }

    with_output_pixel_format(ro->output_pixel_format)
        .with_io_options(ro->io_options);
}

read_options::read_options(const read_options &ro)
    : read_options()
{
    *this = ro;
}

read_options& read_options::operator=(const read_options &ro)
{
    with_output_pixel_format(ro.output_pixel_format())
        .with_io_options(ro.io_options());

    return *this;
}

read_options::~read_options()
{
    delete d;
}

SailPixelFormat read_options::output_pixel_format() const
{
    return d->output_pixel_format;
}

int read_options::io_options() const
{
    return d->io_options;
}

read_options& read_options::with_output_pixel_format(SailPixelFormat output_pixel_format)
{
    d->output_pixel_format = output_pixel_format;
    return *this;
}

read_options& read_options::with_io_options(int io_options)
{
    d->io_options = io_options;
    return *this;
}

sail_error_t read_options::to_sail_read_options(sail_read_options *read_options) const
{
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    read_options->output_pixel_format = d->output_pixel_format;
    read_options->io_options          = d->io_options;

    return 0;
}

}
