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

class SAIL_HIDDEN read_features::pimpl
{
public:
    pimpl()
        : sail_read_features_c(nullptr)
        , preferred_output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , features(0)
    {}

    const sail_read_features *sail_read_features_c;

    std::vector<SailPixelFormat> output_pixel_formats;
    SailPixelFormat preferred_output_pixel_format;
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
        .with_preferred_output_pixel_format(rf.preferred_output_pixel_format())
        .with_features(rf.features());

    return *this;
}

read_features::~read_features()
{
    delete d;
}

std::vector<SailPixelFormat> read_features::output_pixel_formats() const
{
    return d->output_pixel_formats;
}

SailPixelFormat read_features::preferred_output_pixel_format() const
{
    return d->preferred_output_pixel_format;
}

int read_features::features() const
{
    return d->features;
}

sail_error_t read_features::to_read_options(read_options *sread_options) const
{
    SAIL_CHECK_READ_FEATURES_PTR(d->sail_read_features_c);
    SAIL_CHECK_READ_OPTIONS_PTR(sread_options);

    sail_read_options *sail_read_options;

    SAIL_TRY(sail_alloc_read_options_from_features(d->sail_read_features_c, &sail_read_options));

    *sread_options = read_options(sail_read_options);

    sail_destroy_read_options(sail_read_options);

    return 0;
}

read_features::read_features()
    : d(new pimpl)
{
}

read_features::read_features(const sail_read_features *rf)
    : read_features()
{
    if (rf == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::read_features()");
        return;
    }

    d->sail_read_features_c = rf;

    std::vector<SailPixelFormat> output_pixel_formats;

    if (rf->output_pixel_formats != nullptr && rf->output_pixel_formats_length > 0) {
        output_pixel_formats.reserve(rf->output_pixel_formats_length);

        for (int i = 0; i < rf->output_pixel_formats_length; i++) {
            output_pixel_formats.push_back(rf->output_pixel_formats[i]);
        }
    }

    with_output_pixel_formats(output_pixel_formats)
        .with_preferred_output_pixel_format(rf->preferred_output_pixel_format)
        .with_features(rf->features);
}

read_features& read_features::with_output_pixel_formats(const std::vector<SailPixelFormat> &output_pixel_formats)
{
    d->output_pixel_formats = output_pixel_formats;
    return *this;
}

read_features& read_features::with_preferred_output_pixel_format(SailPixelFormat preferred_output_pixel_format)
{
    d->preferred_output_pixel_format = preferred_output_pixel_format;
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
