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

class SAIL_HIDDEN palette::pimpl
{
public:
    pimpl()
        : pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , data(nullptr)
        , color_count(0)
    {}

    ~pimpl()
    {
        free(data);
    }

    SailPixelFormat pixel_format;
    void *data;
    unsigned color_count;
};

palette::palette()
    : d(new pimpl)
{
}

palette::palette(const palette &pal)
    : palette()
{
    *this = pal;
}

palette& palette::operator=(const palette &pal)
{
    with_data(pal.pixel_format(), pal.data(), pal.color_count());

    return *this;
}

palette::~palette()
{
    delete d;
}

bool palette::is_valid() const
{
    return d->data != nullptr && d->color_count > 0;
}

SailPixelFormat palette::pixel_format() const
{
    return d->pixel_format;
}

const void* palette::data() const
{
    return d->data;
}

unsigned palette::color_count() const
{
    return d->color_count;
}

palette& palette::with_data(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    SAIL_TRY_OR_SUPPRESS(copy(pixel_format, data, color_count));

    return *this;
}

palette::palette(const sail_palette *pal)
    : palette()
{
    if (pal == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::palette()");
        return;
    }

    with_data(pal->pixel_format, pal->data, pal->color_count);
}

sail_error_t palette::to_sail_palette(sail_palette *pal) const
{
    SAIL_CHECK_PALETTE_PTR(pal);

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(d->pixel_format, &bits_per_pixel));

    unsigned palette_size = d->color_count * bits_per_pixel / 8;

    pal->data = malloc(palette_size);

    if (pal->data == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(pal->data, d->data, palette_size);
    pal->pixel_format = d->pixel_format;
    pal->color_count = d->color_count;

    return 0;
}

sail_error_t palette::copy(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(pixel_format, &bits_per_pixel));

    unsigned palette_size = color_count * bits_per_pixel / 8;
    d->data = malloc(palette_size);

    if (d->data == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    d->pixel_format = pixel_format;
    d->color_count  = color_count;

    memcpy(d->data, data, palette_size);

    return 0;
}

}
