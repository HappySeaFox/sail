/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#include "config.h"

// libsail-common.
#include "common.h"
#include "error.h"
#include "log.h"

// libsail.
#include "plugin_info.h"
#include "string_node.h"

#include "write_options-c++.h"

namespace sail
{

class SAIL_HIDDEN write_options::pimpl
{
public:
    pimpl()
        : pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , io_options(0)
        , compression_type(0)
        , compression(0)
    {}

    int pixel_format;
    int io_options;
    int compression_type;
    int compression;
};

write_options::write_options()
    : d(new pimpl)
{
}

write_options::write_options(const sail_write_options *wo)
    : write_options()
{
    if (wo == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::write_options()");
        return;
    }

    with_pixel_format(wo->pixel_format)
        .with_io_options(wo->io_options)
        .with_compression_type(wo->compression_type)
        .with_compression(wo->compression);
}

write_options::write_options(const write_options &wo)
    : write_options()
{
    *this = wo;
}

write_options& write_options::operator=(const write_options &wo)
{
    with_pixel_format(wo.pixel_format())
        .with_io_options(wo.io_options())
        .with_compression_type(wo.compression_type())
        .with_compression(wo.compression());

    return *this;
}

write_options::~write_options()
{
    delete d;
}

int write_options::pixel_format() const
{
    return d->pixel_format;
}

int write_options::io_options() const
{
    return d->io_options;
}

int write_options::compression_type() const
{
    return d->compression_type;
}

int write_options::compression() const
{
    return d->compression;
}

write_options& write_options::with_pixel_format(int pixel_format)
{
    d->pixel_format = pixel_format;
    return *this;
}

write_options& write_options::with_io_options(int io_options)
{
    d->io_options = io_options;
    return *this;
}

write_options& write_options::with_compression_type(int compression_type)
{
    d->compression_type = compression_type;
    return *this;
}

write_options& write_options::with_compression(int compression)
{
    d->compression = compression;
    return *this;
}

sail_error_t write_options::to_sail_write_options(sail_write_options **write_options) const
{
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    SAIL_TRY(sail_alloc_write_options(write_options));

    (*write_options)->pixel_format     = d->pixel_format;
    (*write_options)->io_options       = d->io_options;
    (*write_options)->compression_type = d->compression_type;
    (*write_options)->compression      = d->compression;

    return 0;
}

}
