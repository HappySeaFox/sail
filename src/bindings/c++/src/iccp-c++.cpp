/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
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
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::iccp()");
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
