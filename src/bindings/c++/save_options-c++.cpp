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

class SAIL_HIDDEN save_options::pimpl
{
public:
    pimpl()
        : sail_save_options(nullptr)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_save_options(&sail_save_options),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_save_options(sail_save_options);
    }

    struct sail_save_options *sail_save_options;
    sail::tuning tuning;
};

save_options::save_options()
    : d(new pimpl)
{
}

save_options::save_options(const save_options &wo)
    : save_options()
{
    *this = wo;
}

save_options& save_options::operator=(const sail::save_options &save_options)
{
    set_options(save_options.options());
    set_compression(save_options.compression());
    set_compression_level(save_options.compression_level());
    set_tuning(save_options.tuning());

    return *this;
}

save_options::save_options(sail::save_options &&save_options) noexcept
{
    *this = std::move(save_options);
}

save_options& save_options::operator=(sail::save_options &&save_options) noexcept
{
    d = std::move(save_options.d);

    return *this;
}

save_options::~save_options()
{
}

int save_options::options() const
{
    return d->sail_save_options->options;
}

SailCompression save_options::compression() const
{
    return d->sail_save_options->compression;
}

double save_options::compression_level() const
{
    return d->sail_save_options->compression_level;
}

sail::tuning& save_options::tuning()
{
    return d->tuning;
}

const sail::tuning& save_options::tuning() const
{
    return d->tuning;
}

void save_options::set_options(int options)
{
    d->sail_save_options->options = options;
}

void save_options::set_compression(SailCompression compression)
{
    d->sail_save_options->compression = compression;
}

void save_options::set_compression_level(double compression_level)
{
    d->sail_save_options->compression_level = compression_level;
}

void save_options::set_tuning(const sail::tuning &tuning)
{
    d->tuning = tuning;
}

save_options::save_options(const sail_save_options *wo)
    : save_options()
{
    if (wo == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::save_options(). The object is untouched");
        return;
    }

    set_options(wo->options);
    set_compression(wo->compression);
    set_compression_level(wo->compression_level);
    set_tuning(utils_private::c_tuning_to_cpp_tuning(wo->tuning));
}

sail_status_t save_options::to_sail_save_options(sail_save_options **save_options) const
{
    SAIL_CHECK_PTR(save_options);

    sail_save_options *save_options_local;

    SAIL_TRY(sail_alloc_save_options(&save_options_local));

    save_options_local->options           = d->sail_save_options->options;
    save_options_local->compression       = d->sail_save_options->compression;
    save_options_local->compression_level = d->sail_save_options->compression_level;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&save_options_local->tuning),
                        /* cleanup */ sail_destroy_save_options(save_options_local));

    SAIL_TRY_OR_CLEANUP(utils_private::cpp_tuning_to_sail_tuning(d->tuning, save_options_local->tuning),
                        /* cleanup */ sail_destroy_save_options(save_options_local));

    *save_options = save_options_local;

    return SAIL_OK;
}

}
