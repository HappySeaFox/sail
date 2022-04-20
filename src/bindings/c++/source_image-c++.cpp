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

#include <stdexcept>

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN source_image::pimpl
{
public:
    pimpl()
        : sail_source_image(nullptr)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_source_image(&sail_source_image),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_source_image(sail_source_image);
    }

public:
    struct sail_source_image *sail_source_image;
    sail::special_properties special_properties;
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
    d->sail_source_image->pixel_format       = si.pixel_format();
    d->sail_source_image->chroma_subsampling = si.chroma_subsampling();
    d->sail_source_image->orientation        = si.orientation();
    d->sail_source_image->compression        = si.compression();
    d->sail_source_image->interlaced         = si.interlaced();
    d->special_properties                    = si.special_properties();

    return *this;
}

source_image::source_image(source_image &&si) noexcept
{
    *this = std::move(si);
}

source_image& source_image::operator=(source_image &&si) noexcept
{
    d = std::move(si.d);

    return *this;
}

source_image::~source_image()
{
}

bool source_image::is_valid() const
{
    return d->sail_source_image != nullptr;
}

SailPixelFormat source_image::pixel_format() const
{
    return d->sail_source_image->pixel_format;
}

SailChromaSubsampling source_image::chroma_subsampling() const
{
    return d->sail_source_image->chroma_subsampling;
}

SailOrientation source_image::orientation() const
{
    return d->sail_source_image->orientation;
}

SailCompression source_image::compression() const
{
    return d->sail_source_image->compression;
}

bool source_image::interlaced() const
{
    return d->sail_source_image->interlaced;
}

const sail::special_properties &source_image::special_properties() const
{
    return d->special_properties;
}

source_image::source_image(const sail_source_image *si)
    : source_image()
{
    if (si == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::source_image(). The object is untouched");
        return;
    }

    d->sail_source_image->pixel_format       = si->pixel_format;
    d->sail_source_image->chroma_subsampling = si->chroma_subsampling;
    d->sail_source_image->orientation        = si->orientation;
    d->sail_source_image->compression        = si->compression;
    d->sail_source_image->interlaced         = si->interlaced;
    d->special_properties                    = utils_private::c_tuning_to_cpp_tuning(si->special_properties);
}

sail_status_t source_image::to_sail_source_image(sail_source_image **source_image) const
{
    SAIL_CHECK_PTR(source_image);

    sail_source_image *source_image_local;
    SAIL_TRY(sail_alloc_source_image(&source_image_local));

    source_image_local->pixel_format       = d->sail_source_image->pixel_format;
    source_image_local->chroma_subsampling = d->sail_source_image->chroma_subsampling;
    source_image_local->orientation        = d->sail_source_image->orientation;
    source_image_local->compression        = d->sail_source_image->compression;
    source_image_local->interlaced         = d->sail_source_image->interlaced;

    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&source_image_local->special_properties),
                        /* cleanup */ sail_destroy_source_image(source_image_local));

    SAIL_TRY_OR_CLEANUP(utils_private::cpp_tuning_to_sail_tuning(d->special_properties, source_image_local->special_properties),
                        /* cleanup */ sail_destroy_source_image(source_image_local));

    *source_image = source_image_local;

    return SAIL_OK;
}

}
