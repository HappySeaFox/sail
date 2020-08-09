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

class SAIL_HIDDEN read_features::pimpl
{
public:
    pimpl()
        : sail_read_features_c(nullptr)
        , default_output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , features(0)
    {}

    const sail_read_features *sail_read_features_c;

    std::vector<SailPixelFormat> output_pixel_formats;
    SailPixelFormat default_output_pixel_format;
    int features;
};

read_features::read_features(const read_features &rf)
    : read_features()
{
    *this = rf;
}

read_features& read_features::operator=(const read_features &rf)
{
    d->sail_read_features_c = rf.d->sail_read_features_c;

    with_output_pixel_formats(rf.output_pixel_formats())
        .with_default_output_pixel_format(rf.default_output_pixel_format())
        .with_features(rf.features());

    return *this;
}

read_features::~read_features()
{
    delete d;
}

const std::vector<SailPixelFormat>& read_features::output_pixel_formats() const
{
    return d->output_pixel_formats;
}

SailPixelFormat read_features::default_output_pixel_format() const
{
    return d->default_output_pixel_format;
}

int read_features::features() const
{
    return d->features;
}

sail_status_t read_features::to_read_options(read_options *sread_options) const
{
    SAIL_CHECK_READ_FEATURES_PTR(d->sail_read_features_c);
    SAIL_CHECK_READ_OPTIONS_PTR(sread_options);

    sail_read_options *sail_read_options;

    SAIL_TRY(sail_alloc_read_options_from_features(d->sail_read_features_c, &sail_read_options));

    *sread_options = read_options(sail_read_options);

    sail_destroy_read_options(sail_read_options);

    return SAIL_OK;
}

read_features::read_features()
    : d(new pimpl)
{
}

read_features::read_features(const sail_read_features *rf)
    : read_features()
{
    if (rf == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::read_features(). The object is untouched");
        return;
    }

    d->sail_read_features_c = rf;

    std::vector<SailPixelFormat> output_pixel_formats;

    if (rf->output_pixel_formats != nullptr && rf->output_pixel_formats_length > 0) {
        output_pixel_formats.reserve(rf->output_pixel_formats_length);

        for (unsigned i = 0; i < rf->output_pixel_formats_length; i++) {
            output_pixel_formats.push_back(rf->output_pixel_formats[i]);
        }
    }

    with_output_pixel_formats(output_pixel_formats)
        .with_default_output_pixel_format(rf->default_output_pixel_format)
        .with_features(rf->features);
}

read_features& read_features::with_output_pixel_formats(const std::vector<SailPixelFormat> &output_pixel_formats)
{
    d->output_pixel_formats = output_pixel_formats;
    return *this;
}

read_features& read_features::with_default_output_pixel_format(SailPixelFormat default_output_pixel_format)
{
    d->default_output_pixel_format = default_output_pixel_format;
    return *this;
}

read_features& read_features::with_features(int features)
{
    d->features = features;
    return *this;
}

const sail_read_features* read_features::sail_read_features_c() const
{
    return d->sail_read_features_c;
}

}
