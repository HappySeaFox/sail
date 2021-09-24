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

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN write_options::pimpl
{
public:
    pimpl()
        : sail_write_options(nullptr)
    {
        SAIL_TRY_OR_SUPPRESS(sail_alloc_write_options(&sail_write_options));
    }

    ~pimpl()
    {
        sail_destroy_write_options(sail_write_options);
    }

    struct sail_write_options *sail_write_options;
};

write_options::write_options()
    : d(new pimpl)
{
}

write_options::write_options(const write_options &wo)
    : write_options()
{
    *this = wo;
}

write_options& write_options::operator=(const sail::write_options &write_options)
{
    with_io_options(write_options.io_options())
        .with_compression(write_options.compression())
        .with_compression_level(write_options.compression_level());

    return *this;
}

write_options::write_options(sail::write_options &&write_options) noexcept
{
    d = write_options.d;
    write_options.d = nullptr;
}

write_options& write_options::operator=(sail::write_options &&write_options)
{
    delete d;
    d = write_options.d;
    write_options.d = nullptr;

    return *this;
}

write_options::~write_options()
{
    delete d;
}

int write_options::io_options() const
{
    return d->sail_write_options->io_options;
}

SailCompression write_options::compression() const
{
    return d->sail_write_options->compression;
}

double write_options::compression_level() const
{
    return d->sail_write_options->compression_level;
}

write_options& write_options::with_io_options(int io_options)
{
    d->sail_write_options->io_options = io_options;
    return *this;
}

write_options& write_options::with_compression(SailCompression compression)
{
    d->sail_write_options->compression = compression;
    return *this;
}

write_options& write_options::with_compression_level(double compression_level)
{
    d->sail_write_options->compression_level = compression_level;
    return *this;
}

write_options::write_options(const sail_write_options *wo)
    : write_options()
{
    if (wo == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::write_options(). The object is untouched");
        return;
    }

    with_io_options(wo->io_options)
        .with_compression(wo->compression)
        .with_compression_level(wo->compression_level);
}

sail_status_t write_options::to_sail_write_options(sail_write_options *write_options) const
{
    SAIL_CHECK_PTR(write_options);

    write_options->io_options        = d->sail_write_options->io_options;
    write_options->compression       = d->sail_write_options->compression;
    write_options->compression_level = d->sail_write_options->compression_level;

    return SAIL_OK;
}

}
