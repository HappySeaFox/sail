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

#include "read_features-c++.h"

namespace sail
{

class SAIL_HIDDEN read_features::pimpl
{
public:
    pimpl()
        : preferred_output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , features(0)
    {}

    std::vector<int> input_pixel_formats;
    std::vector<int> output_pixel_formats;
    int preferred_output_pixel_format;
    int features;
};

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

    std::vector<int> input_pixel_formats;

    if (rf->input_pixel_formats != nullptr && rf->input_pixel_formats_length > 0) {
        input_pixel_formats.reserve(rf->input_pixel_formats_length);

        for (int i = 0; i < rf->input_pixel_formats_length; i++) {
            input_pixel_formats.push_back(rf->input_pixel_formats[i]);
        }
    }

    std::vector<int> output_pixel_formats;

    if (rf->output_pixel_formats != nullptr && rf->output_pixel_formats_length > 0) {
        output_pixel_formats.reserve(rf->output_pixel_formats_length);

        for (int i = 0; i < rf->output_pixel_formats_length; i++) {
            output_pixel_formats.push_back(rf->output_pixel_formats[i]);
        }
    }

    with_input_pixel_formats(input_pixel_formats)
        .with_output_pixel_formats(output_pixel_formats)
        .with_preferred_output_pixel_format(rf->preferred_output_pixel_format)
        .with_features(rf->features);
}

read_features::read_features(const read_features &rf)
    : read_features()
{
    *this = rf;
}

read_features& read_features::operator=(const read_features &rf)
{
    with_input_pixel_formats(rf.input_pixel_formats())
        .with_output_pixel_formats(rf.output_pixel_formats())
        .with_preferred_output_pixel_format(rf.preferred_output_pixel_format())
        .with_features(rf.features());

    return *this;
}

read_features::~read_features()
{
    delete d;
}

std::vector<int> read_features::input_pixel_formats() const
{
    return d->input_pixel_formats;
}

std::vector<int> read_features::output_pixel_formats() const
{
    return d->output_pixel_formats;
}

int read_features::preferred_output_pixel_format() const
{
    return d->preferred_output_pixel_format;
}

int read_features::features() const
{
    return d->features;
}

read_features& read_features::with_input_pixel_formats(const std::vector<int> &input_pixel_formats)
{
    d->input_pixel_formats = input_pixel_formats;
    return *this;
}

read_features& read_features::with_output_pixel_formats(const std::vector<int> &output_pixel_formats)
{
    d->output_pixel_formats = output_pixel_formats;
    return *this;
}

read_features& read_features::with_preferred_output_pixel_format(int preferred_output_pixel_format)
{
    d->preferred_output_pixel_format = preferred_output_pixel_format;
    return *this;
}

read_features& read_features::with_features(int features)
{
    d->features = features;
    return *this;
}

}
