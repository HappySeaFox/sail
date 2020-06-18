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

class SAIL_HIDDEN iccp::pimpl
{
public:
    pimpl()
        : data(nullptr)
        , data_length(0)
    {}

    ~pimpl()
    {
        free(data);
    }

    void *data;
    unsigned data_length;
};

iccp::iccp()
    : d(new pimpl)
{
}

iccp::iccp(const iccp &ic)
    : iccp()
{
    *this = ic;
}

iccp& iccp::operator=(const iccp &ic)
{
    with_data(ic.data(), ic.data_length());

    return *this;
}

iccp::~iccp()
{
    delete d;
}

bool iccp::is_valid() const
{
    return d->data != nullptr && d->data_length > 0;
}

void* iccp::data() const
{
    return d->data;
}

unsigned iccp::data_length() const
{
    return d->data_length;
}

iccp& iccp::with_data(const void *data, unsigned data_length)
{
    free(d->data);

    d->data = nullptr;
    d->data_length = 0;

    if (data == nullptr || data_length == 0) {
        return *this;
    }

    d->data = malloc(data_length);

    if (d->data == nullptr) {
        SAIL_LOG_ERROR("Memory allocation failed of ICC data length %u", data_length);
        return *this;
    }

    memcpy(d->data, data, data_length);
    d->data_length = data_length;

    return *this;
}

iccp::iccp(const sail_iccp *ic)
    : iccp()
{
    if (ic == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::iccp(). The object is untouched");
        return;
    }

    with_data(ic->data, ic->data_length);
}

sail_error_t iccp::to_sail_iccp(sail_iccp *ic) const
{
    SAIL_CHECK_ICCP_PTR(ic);

    ic->data = malloc(d->data_length);

    if (ic->data == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(ic->data, d->data, d->data_length);
    ic->data_length = d->data_length;

    return 0;
}

}
