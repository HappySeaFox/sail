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

resolution::resolution(SailResolutionUnit unit, double x, double y)
    : resolution()
{
    set_unit(unit);
    set_x(x);
    set_y(y);
}

resolution::resolution(const resolution &res)
    : resolution()
{
    *this = res;
}

resolution& resolution::operator=(const resolution &res)
{
    set_unit(res.unit());
    set_x(res.x());
    set_y(res.y());

    return *this;
}

resolution::resolution(resolution &&res) noexcept
{
    *this = std::move(res);
}

resolution& resolution::operator=(resolution &&res) noexcept
{
    d = std::move(res.d);

    return *this;
}

resolution::~resolution()
{
}

bool resolution::is_valid() const
{
    return d->resolution.unit != SAIL_RESOLUTION_UNIT_UNKNOWN && d->resolution.x > 0 && d->resolution.y > 0;
}

SailResolutionUnit resolution::unit() const
{
    return d->resolution.unit;
}

double resolution::x() const
{
    return d->resolution.x;
}

double resolution::y() const
{
    return d->resolution.y;
}

void resolution::set_unit(SailResolutionUnit unit)
{
    d->resolution.unit = unit;
}

void resolution::set_x(double x)
{
    d->resolution.x = x;
}

void resolution::set_y(double y)
{
    d->resolution.y = y;
}

const char* resolution::resolution_unit_to_string(SailResolutionUnit resolution_unit)
{
    return sail_resolution_unit_to_string(resolution_unit);
}

SailResolutionUnit resolution::resolution_unit_from_string(const std::string &str)
{
    return sail_resolution_unit_from_string(str.c_str());
}

resolution::resolution(const sail_resolution *res)
    : resolution()
{
    if (res == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::resolution(). The object is untouched");
        return;
    }

    d->resolution = *res;
}

sail_status_t resolution::to_sail_resolution(sail_resolution **resolution) const
{
    SAIL_CHECK_PTR(resolution);

    SAIL_TRY(sail_alloc_resolution(resolution));
    **resolution = d->resolution;

    return SAIL_OK;
}

}
