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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN read_options::pimpl
{
public:
    pimpl()
        : output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , io_options(0)
    {}

    int output_pixel_format;
    int io_options;
};

read_options::read_options()
    : d(new pimpl)
{
}

read_options::read_options(const sail_read_options *ro)
    : read_options()
{
    if (ro == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::read_options()");
        return;
    }

    with_output_pixel_format(ro->output_pixel_format)
        .with_io_options(ro->io_options);
}

read_options::read_options(const read_options &ro)
    : read_options()
{
    *this = ro;
}

read_options& read_options::operator=(const read_options &ro)
{
    with_output_pixel_format(ro.output_pixel_format())
        .with_io_options(ro.io_options());

    return *this;
}

read_options::~read_options()
{
    delete d;
}

int read_options::output_pixel_format() const
{
    return d->output_pixel_format;
}

int read_options::io_options() const
{
    return d->io_options;
}

read_options& read_options::with_output_pixel_format(int output_pixel_format)
{
    d->output_pixel_format = output_pixel_format;
    return *this;
}

read_options& read_options::with_io_options(int io_options)
{
    d->io_options = io_options;
    return *this;
}

sail_error_t read_options::to_sail_read_options(sail_read_options *read_options) const
{
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    read_options->output_pixel_format = d->output_pixel_format;
    read_options->io_options          = d->io_options;

    return 0;
}

}
