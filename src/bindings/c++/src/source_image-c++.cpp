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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN source_image::pimpl
{
public:
    pimpl()
        : source_image(nullptr)
    {
        SAIL_TRY_OR_SUPPRESS(init());
    }

    ~pimpl()
    {
        sail_destroy_source_image(source_image);
    }

    sail_status_t init()
    {
        SAIL_TRY(sail_alloc_source_image(&source_image));

        return SAIL_OK;
    }

    sail_source_image *source_image;
};

source_image::source_image()
    : d(new pimpl)
{
}

source_image::source_image(const source_image &si)
    : source_image()
{
    *this = si;
}

source_image& source_image::operator=(const source_image &si)
{
    with_pixel_format(si.pixel_format())
        .with_properties(si.properties())
        .with_compression_type(si.compression_type());

    return *this;
}

source_image::~source_image()
{
    delete d;
}

bool source_image::is_valid() const
{
    return d->source_image != nullptr;
}

SailPixelFormat source_image::pixel_format() const
{
    return d->source_image->pixel_format;
}

int source_image::properties() const
{
    return d->source_image->properties;
}

SailCompressionType source_image::compression_type() const
{
    return d->source_image->compression_type;
}

source_image::source_image(const sail_source_image *si)
    : source_image()
{
    if (si == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::source_image(). The object is untouched");
        return;
    }

    with_pixel_format(si->pixel_format)
        .with_properties(si->properties)
        .with_compression_type(si->compression_type);
}

sail_status_t source_image::to_sail_source_image(sail_source_image *si) const
{
    SAIL_CHECK_SOURCE_IMAGE_PTR(si);

    si->pixel_format     = d->source_image->pixel_format;
    si->properties       = d->source_image->properties;
    si->compression_type = d->source_image->compression_type;

    return SAIL_OK;
}

source_image& source_image::with_pixel_format(SailPixelFormat pixel_format)
{
    d->source_image->pixel_format = pixel_format;
    return *this;
}

source_image& source_image::with_properties(int properties)
{
    d->source_image->properties = properties;
    return *this;
}

source_image& source_image::with_compression_type(SailCompressionType compression_type)
{
    d->source_image->compression_type = compression_type;
    return *this;
}


}
