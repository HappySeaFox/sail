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

class SAIL_HIDDEN palette::pimpl
{
public:
    pimpl()
        : pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , color_count(0)
    {}

    SailPixelFormat pixel_format;
    arbitrary_data data;
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
    with_data(pal.pixel_format(), pal.data());

    return *this;
}

palette::palette(palette &&pal) noexcept
{
    d = pal.d;
    pal.d = nullptr;
}

palette& palette::operator=(palette &&pal)
{
    delete d;
    d = pal.d;
    pal.d = nullptr;

    return *this;
}

palette::~palette()
{
    delete d;
}

bool palette::is_valid() const
{
    return !d->data.empty() && d->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN && d->color_count > 0;
}

SailPixelFormat palette::pixel_format() const
{
    return d->pixel_format;
}

const arbitrary_data& palette::data() const
{
    return d->data;
}

unsigned palette::color_count() const
{
    return d->color_count;
}

palette& palette::with_data(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    d->data.clear();
    d->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    d->color_count  = 0;

    if (pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_TRY_OR_SUPPRESS(copy(pixel_format, data, color_count));
    }

    return *this;
}

palette& palette::with_data(SailPixelFormat pixel_format, const arbitrary_data &data)
{
    return with_data(pixel_format, data.data(), static_cast<unsigned>(data.size()));
}

palette::palette(const sail_palette *pal)
    : palette()
{
    if (pal == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::palette(). The object is untouched");
        return;
    }

    with_data(pal->pixel_format, pal->data, pal->color_count);
}

sail_status_t palette::to_sail_palette(sail_palette *pal) const
{
    SAIL_CHECK_PALETTE_PTR(pal);

    SAIL_TRY(sail_malloc(d->data.size(), &pal->data));

    memcpy(pal->data, d->data.data(), d->data.size());

    pal->pixel_format = d->pixel_format;
    pal->color_count  = d->color_count;

    return SAIL_OK;
}

sail_status_t palette::copy(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    unsigned palette_size;
    SAIL_TRY(sail_bytes_per_line(color_count, pixel_format, &palette_size));

    d->data.resize(palette_size);

    d->pixel_format = pixel_format;
    d->color_count  = color_count;

    memcpy(d->data.data(), data, palette_size);

    return SAIL_OK;
}

}
