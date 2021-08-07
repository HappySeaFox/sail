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
        : sail_palette(nullptr)
    {
        SAIL_TRY_OR_SUPPRESS(sail_alloc_palette(&sail_palette));
    }

    ~pimpl()
    {
        sail_destroy_palette(sail_palette);
    }

    struct sail_palette *sail_palette;
    arbitrary_data data;
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

palette& palette::operator=(const sail::palette &palette)
{
    with_data(palette.pixel_format(), palette.data());

    return *this;
}

palette::palette(sail::palette &&palette) noexcept
{
    d = palette.d;
    palette.d = nullptr;
}

palette& palette::operator=(sail::palette &&palette)
{
    delete d;
    d = palette.d;
    palette.d = nullptr;

    return *this;
}

palette::~palette()
{
    delete d;
}

bool palette::is_valid() const
{
    return !d->data.empty() && d->sail_palette->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN && d->sail_palette->color_count > 0;
}

SailPixelFormat palette::pixel_format() const
{
    return d->sail_palette->pixel_format;
}

const arbitrary_data& palette::data() const
{
    return d->data;
}

unsigned palette::color_count() const
{
    return d->sail_palette->color_count;
}

palette& palette::with_data(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    d->data.clear();
    d->sail_palette->pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    d->sail_palette->color_count  = 0;

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

sail_status_t palette::to_sail_palette(sail_palette **palette) const
{
    SAIL_CHECK_PALETTE_PTR(palette);

    SAIL_TRY(sail_alloc_palette_from_data(d->sail_palette->pixel_format, d->data.data(), d->sail_palette->color_count, palette));

    return SAIL_OK;
}

sail_status_t palette::copy(SailPixelFormat pixel_format, const void *data, unsigned color_count)
{
    unsigned palette_size;
    SAIL_TRY(sail_bytes_per_line(color_count, pixel_format, &palette_size));

    d->data.resize(palette_size);
    memcpy(d->data.data(), data, palette_size);

    d->sail_palette->pixel_format = pixel_format;
    d->sail_palette->color_count  = color_count;

    return SAIL_OK;
}

}
