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

class SAIL_HIDDEN write_options::pimpl
{
public:
    pimpl()
        : output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , io_options(0)
        , compression(SAIL_COMPRESSION_UNSUPPORTED)
        , compression_level(0)
    {}

    SailPixelFormat output_pixel_format;
    int io_options;
    SailCompression compression;
    double compression_level;
};

write_options::write_options()
    : d(new pimpl)
{
}

write_options::write_options(const sail_write_options *wo)
    : write_options()
{
    if (wo == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::write_options(). The object is untouched");
        return;
    }

    with_output_pixel_format(wo->output_pixel_format)
        .with_io_options(wo->io_options)
        .with_compression(wo->compression)
        .with_compression_level(wo->compression_level);
}

write_options::write_options(const write_options &wo)
    : write_options()
{
    *this = wo;
}

write_options& write_options::operator=(const write_options &wo)
{
    with_output_pixel_format(wo.output_pixel_format())
        .with_io_options(wo.io_options())
        .with_compression(wo.compression())
        .with_compression_level(wo.compression_level());

    return *this;
}

write_options::write_options(write_options &&wo)
{
    d = wo.d;
    wo.d = nullptr;
}

write_options& write_options::operator=(write_options &&wo)
{
    delete d;
    d = wo.d;
    wo.d = nullptr;

    return *this;
}

write_options::~write_options()
{
    delete d;
}

SailPixelFormat write_options::output_pixel_format() const
{
    return d->output_pixel_format;
}

int write_options::io_options() const
{
    return d->io_options;
}

SailCompression write_options::compression() const
{
    return d->compression;
}

double write_options::compression_level() const
{
    return d->compression_level;
}

write_options& write_options::with_output_pixel_format(SailPixelFormat output_pixel_format)
{
    d->output_pixel_format = output_pixel_format;
    return *this;
}

write_options& write_options::with_io_options(int io_options)
{
    d->io_options = io_options;
    return *this;
}

write_options& write_options::with_compression(SailCompression compression)
{
    d->compression = compression;
    return *this;
}

write_options& write_options::with_compression_level(double compression_level)
{
    d->compression_level = compression_level;
    return *this;
}

sail_status_t write_options::to_sail_write_options(sail_write_options *write_options) const
{
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    write_options->output_pixel_format = d->output_pixel_format;
    write_options->io_options          = d->io_options;
    write_options->compression         = d->compression;
    write_options->compression_level   = d->compression_level;

    return SAIL_OK;
}

}
