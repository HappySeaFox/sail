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

#include <stdexcept>

#include <sail/sail.h>

#include <sail-c++/sail-c++.h>

namespace sail
{

class SAIL_HIDDEN load_options::pimpl
{
public:
    pimpl()
        : sail_load_options(nullptr)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_load_options(&sail_load_options),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_load_options(sail_load_options);
    }

    struct sail_load_options* sail_load_options;
    sail::tuning tuning;
};

load_options::load_options(int options)
    : d(new pimpl)
{
    set_options(options);
}

load_options::load_options()
    : d(new pimpl)
{
}

load_options::load_options(const load_options& ro)
    : load_options()
{
    *this = ro;
}

load_options& load_options::operator=(const sail::load_options& load_options)
{
    set_options(load_options.options());
    set_tuning(load_options.tuning());
    set_max_width(load_options.max_width());
    set_max_height(load_options.max_height());
    set_max_pixels(load_options.max_pixels());
    set_max_frames(load_options.max_frames());

    return *this;
}

load_options::load_options(sail::load_options&& load_options) noexcept
{
    *this = std::move(load_options);
}

load_options& load_options::operator=(load_options&& ro) noexcept
{
    d = std::move(ro.d);

    return *this;
}

load_options::~load_options()
{
}

int load_options::options() const
{
    return d->sail_load_options->options;
}

sail::tuning& load_options::tuning()
{
    return d->tuning;
}

const sail::tuning& load_options::tuning() const
{
    return d->tuning;
}

unsigned load_options::max_width() const
{
    return d->sail_load_options->max_width;
}

unsigned load_options::max_height() const
{
    return d->sail_load_options->max_height;
}

std::pair<unsigned, unsigned> load_options::max_dimensions() const
{
    return {max_width(), max_height()};
}

std::uint64_t load_options::max_pixels() const
{
    return d->sail_load_options->max_pixels;
}

unsigned load_options::max_frames() const
{
    return d->sail_load_options->max_frames;
}

void load_options::set_options(int options)
{
    d->sail_load_options->options = options;
}

void load_options::set_tuning(const sail::tuning& tuning)
{
    d->tuning = tuning;
}

void load_options::set_max_width(unsigned max_width)
{
    d->sail_load_options->max_width = max_width;
}

void load_options::set_max_height(unsigned max_height)
{
    d->sail_load_options->max_height = max_height;
}

void load_options::set_max_dimensions(unsigned max_width, unsigned max_height)
{
    set_max_width(max_width);
    set_max_height(max_height);
}

void load_options::set_max_pixels(std::uint64_t max_pixels)
{
    d->sail_load_options->max_pixels = max_pixels;
}

void load_options::set_max_frames(unsigned max_frames)
{
    d->sail_load_options->max_frames = max_frames;
}

load_options::load_options(const sail_load_options* ro)
    : load_options()
{
    if (ro == nullptr)
    {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::load_options(). The object is untouched");
        return;
    }

    set_options(ro->options);
    set_tuning(utils_private::to_cpp_tuning(ro->tuning));
    set_max_width(ro->max_width);
    set_max_height(ro->max_height);
    set_max_pixels(ro->max_pixels);
    set_max_frames(ro->max_frames);
}

sail_status_t load_options::to_sail_load_options(sail_load_options** load_options) const
{
    SAIL_CHECK_PTR(load_options);

    sail_load_options* load_options_local;

    SAIL_TRY(sail_alloc_load_options(&load_options_local));

    load_options_local->options    = d->sail_load_options->options;
    load_options_local->max_width  = d->sail_load_options->max_width;
    load_options_local->max_height = d->sail_load_options->max_height;
    load_options_local->max_pixels = d->sail_load_options->max_pixels;
    load_options_local->max_frames = d->sail_load_options->max_frames;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&load_options_local->tuning),
                        /* cleanup */ sail_destroy_load_options(load_options_local));

    SAIL_TRY_OR_CLEANUP(utils_private::to_sail_tuning(d->tuning, load_options_local->tuning),
                        /* cleanup */ sail_destroy_load_options(load_options_local));

    *load_options = load_options_local;

    return SAIL_OK;
}

} // namespace sail
