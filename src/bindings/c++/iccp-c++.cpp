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

#include <cstring>

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN iccp::pimpl
{
public:
    pimpl()
    {
    }

    void reset()
    {
        data.clear();
    }

    arbitrary_data data;
};

iccp::iccp()
    : d(new pimpl)
{
}

iccp::iccp(const void *data, unsigned data_length)
    : iccp()
{
    with_data(data, data_length);
}

iccp::iccp(const arbitrary_data &data)
    : iccp()
{
    with_data(data);
}

iccp::iccp(const sail::iccp &ic)
    : iccp()
{
    *this = ic;
}

iccp& iccp::operator=(const sail::iccp &iccp)
{
    d->reset();

    if (iccp.is_valid()) {
        with_data(iccp.data());
    }

    return *this;
}

iccp::iccp(sail::iccp &&iccp) noexcept
{
    d = iccp.d;
    iccp.d = nullptr;
}

iccp& iccp::operator=(sail::iccp &&iccp) noexcept
{
    delete d;
    d = iccp.d;
    iccp.d = nullptr;

    return *this;
}

iccp::~iccp()
{
    delete d;
}

bool iccp::is_valid() const
{
    return !d->data.empty();
}

const arbitrary_data& iccp::data() const
{
    return d->data;
}

iccp& iccp::with_data(const void *data, unsigned data_length)
{
    d->reset();

    SAIL_TRY_OR_EXECUTE(copy(data, data_length),
                        /* on error */ return *this);

    return *this;
}

iccp& iccp::with_data(const arbitrary_data &data)
{
    d->reset();

    with_data(data.data(), static_cast<unsigned>(data.size()));

    return *this;
}

iccp::iccp(const sail_iccp *ic)
    : iccp()
{
    if (ic == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::iccp(). The object is untouched");
        return;
    }

    with_data(ic->data, ic->data_length);
}

sail_status_t iccp::to_sail_iccp(sail_iccp **iccp) const
{
    SAIL_CHECK_PTR(iccp);

    sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp_from_data(d->data.data(), static_cast<unsigned>(d->data.size()), &iccp_local));

    *iccp = iccp_local;

    return SAIL_OK;
}

sail_status_t iccp::copy(const void *data, unsigned data_length)
{
    SAIL_CHECK_PTR(data);

    if (data_length == 0) {
        SAIL_LOG_ERROR("ICCP data length is 0");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    d->data.resize(data_length);
    memcpy(d->data.data(), data, data_length);

    return SAIL_OK;
}

}
