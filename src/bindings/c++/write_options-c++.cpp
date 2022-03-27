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

#include <stdexcept>

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
        SAIL_TRY_OR_EXECUTE(sail_alloc_write_options(&sail_write_options),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_write_options(sail_write_options);
    }

    struct sail_write_options *sail_write_options;
    sail::tuning tuning;
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
    with_options(write_options.options())
        .with_tuning(write_options.tuning())
        .with_compression(write_options.compression())
        .with_compression_level(write_options.compression_level());

    return *this;
}

write_options::write_options(sail::write_options &&write_options) noexcept
{
    *this = std::move(write_options);
}

write_options& write_options::operator=(sail::write_options &&write_options) noexcept
{
    d = std::move(write_options.d);

    return *this;
}

write_options::~write_options()
{
}

int write_options::options() const
{
    return d->sail_write_options->options;
}

sail::tuning& write_options::tuning()
{
    return d->tuning;
}

const sail::tuning& write_options::tuning() const
{
    return d->tuning;
}

SailCompression write_options::compression() const
{
    return d->sail_write_options->compression;
}

double write_options::compression_level() const
{
    return d->sail_write_options->compression_level;
}

write_options& write_options::with_options(int options)
{
    d->sail_write_options->options = options;

    return *this;
}

write_options& write_options::with_tuning(const sail::tuning &tuning)
{
    d->tuning = tuning;

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
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::write_options(). The object is untouched");
        return;
    }

    with_options(wo->options)
        .with_compression(wo->compression)
        .with_compression_level(wo->compression_level);
}

sail_status_t write_options::to_sail_write_options(sail_write_options **write_options) const
{
    SAIL_CHECK_PTR(write_options);

    sail_write_options *write_options_local;

    SAIL_TRY(sail_alloc_write_options(&write_options_local));

    write_options_local->options           = d->sail_write_options->options;
    write_options_local->compression       = d->sail_write_options->compression;
    write_options_local->compression_level = d->sail_write_options->compression_level;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&write_options_local->tuning),
                        /* cleanup */ sail_destroy_write_options(write_options_local));

    SAIL_TRY_OR_CLEANUP(utils_private::cpp_tuning_to_sail_tuning(d->tuning, write_options_local->tuning),
                        /* cleanup */ sail_destroy_write_options(write_options_local));

    *write_options = write_options_local;

    return SAIL_OK;
}

}
