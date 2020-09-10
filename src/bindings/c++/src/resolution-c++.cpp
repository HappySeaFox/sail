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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN resolution::pimpl
{
public:
    pimpl()
    {
        resolution.unit = SAIL_RESOLUTION_UNIT_UNKNOWN;
        resolution.x    = 0;
        resolution.y    = 0;
    }

    sail_resolution resolution;
};

resolution::resolution()
    : d(new pimpl)
{
}

resolution::resolution(const resolution &res)
    : resolution()
{
    *this = res;
}

resolution& resolution::operator=(const resolution &res)
{
    with_unit(res.unit())
        .with_x(res.x())
        .with_y(res.y());

    return *this;
}

resolution::~resolution()
{
    delete d;
}

bool resolution::is_valid() const
{
    return d->resolution.unit != SAIL_RESOLUTION_UNIT_UNKNOWN && d->resolution.x > 0 && d->resolution.y > 0;
}

SailResolutionUnit resolution::unit() const
{
    return d->resolution.unit;
}

float resolution::x() const
{
    return d->resolution.x;
}

float resolution::y() const
{
    return d->resolution.y;
}

resolution& resolution::with_unit(SailResolutionUnit unit)
{
    d->resolution.unit = unit;
    return *this;
}

resolution& resolution::with_x(float x)
{
    d->resolution.x = x;
    return *this;
}

resolution& resolution::with_y(float y)
{
    d->resolution.y = y;
    return *this;
}

resolution::resolution(const sail_resolution *res)
    : resolution()
{
    if (res == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::resolution(). The object is untouched");
        return;
    }

    d->resolution = *res;
}

sail_status_t resolution::to_sail_resolution(sail_resolution *res) const
{
    SAIL_CHECK_RESOLUTION_PTR(res);

    *res = d->resolution;

    return SAIL_OK;
}

}
