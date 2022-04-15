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
#include "sail-common.h"

namespace sail
{

class SAIL_HIDDEN compression_level::pimpl
{
public:
    pimpl()
        : sail_compression_level(nullptr)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_compression_level(&sail_compression_level),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_compression_level(sail_compression_level);
    }

    struct sail_compression_level *sail_compression_level;
};

compression_level::compression_level(const sail::compression_level &cl)
    : compression_level()
{
    *this = cl;
}

compression_level& compression_level::operator=(const sail::compression_level &compression_level)
{
    d->sail_compression_level->min_level     = compression_level.min_level();
    d->sail_compression_level->max_level     = compression_level.max_level();
    d->sail_compression_level->default_level = compression_level.default_level();
    d->sail_compression_level->step          = compression_level.step();

    return *this;
}

compression_level::compression_level(sail::compression_level &&compression_level) noexcept
{
    *this = std::move(compression_level);
}

compression_level& compression_level::operator=(sail::compression_level &&compression_level) noexcept
{
    d = std::move(compression_level.d);

    return *this;
}

compression_level::~compression_level()
{
}

bool compression_level::is_valid() const
{
    return d->sail_compression_level->min_level < d->sail_compression_level->max_level &&
            d->sail_compression_level->default_level >= d->sail_compression_level->min_level &&
            d->sail_compression_level->default_level <= d->sail_compression_level->max_level;
}

double compression_level::min_level() const
{
    return d->sail_compression_level->min_level;
}

double compression_level::max_level() const
{
    return d->sail_compression_level->max_level;
}

double compression_level::default_level() const
{
    return d->sail_compression_level->default_level;
}

double compression_level::step() const
{
    return d->sail_compression_level->step;
}

compression_level::compression_level()
    : d(new pimpl)
{
}

compression_level::compression_level(const sail_compression_level *cl)
    : compression_level()
{
    if (cl == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::compression_level(). The object is untouched");
        return;
    }

    d->sail_compression_level->min_level     = cl->min_level;
    d->sail_compression_level->max_level     = cl->max_level;
    d->sail_compression_level->default_level = cl->default_level;
    d->sail_compression_level->step          = cl->step;
}

}
