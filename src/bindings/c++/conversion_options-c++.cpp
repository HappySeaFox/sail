/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-manip.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN conversion_options::pimpl
{
public:
    pimpl()
        : conversion_options(nullptr)
    {
        SAIL_TRY_OR_SUPPRESS(sail_alloc_conversion_options(&conversion_options));
    }
    ~pimpl()
    {
        sail_destroy_conversion_options(conversion_options);
    }

    sail_conversion_options *conversion_options;
};

conversion_options::conversion_options()
    : d(new pimpl)
{
}

conversion_options::conversion_options(const conversion_options &co)
    : conversion_options()
{
    *this = co;
}

conversion_options& conversion_options::operator=(const conversion_options &co)
{
    with_options(co.options())
        .with_background(co.background48())
        .with_background(co.background24());

    return *this;
}

conversion_options::conversion_options(conversion_options &&co) noexcept
{
    d = co.d;
    co.d = nullptr;
}

conversion_options& conversion_options::operator=(conversion_options &&co)
{
    delete d;
    d = co.d;
    co.d = nullptr;

    return *this;
}

conversion_options::~conversion_options()
{
    delete d;
}

int conversion_options::options() const
{
    return d->conversion_options->options;
}

sail_rgb48_t conversion_options::background48() const
{
    return d->conversion_options->background48;
}

sail_rgb24_t conversion_options::background24() const
{
    return d->conversion_options->background24;
}

conversion_options& conversion_options::with_options(int options)
{
    d->conversion_options->options = options;
    return *this;
}

conversion_options& conversion_options::with_background(const sail_rgb48_t &rgb48)
{
    d->conversion_options->background48 = rgb48;
    return *this;
}

conversion_options& conversion_options::with_background(const sail_rgb24_t &rgb24)
{
    d->conversion_options->background24 = rgb24;
    return *this;
}

sail_status_t conversion_options::to_sail_conversion_options(sail_conversion_options **conversion_options) const
{
    SAIL_CHECK_CONVERSION_OPTIONS_PTR(conversion_options);

    SAIL_TRY(sail_alloc_conversion_options(conversion_options));
    **conversion_options = *d->conversion_options;

    return SAIL_OK;
}

}
