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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN write_features::pimpl
{
public:
    pimpl()
        : sail_write_features_c(nullptr)
        , preferred_output_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , features(0)
        , properties(0)
        , passes(0)
        , preferred_compression_type(0)
        , compression_min(0)
        , compression_max(0)
        , compression_default(0)
    {}

    const sail_write_features *sail_write_features_c;

    std::vector<int> input_pixel_formats;
    std::vector<int> output_pixel_formats;
    int preferred_output_pixel_format;
    int features;
    int properties;
    int passes;
    std::vector<int> compression_types;
    int preferred_compression_type;
    int compression_min;
    int compression_max;
    int compression_default;
};

write_features::write_features()
    : d(new pimpl)
{
}

write_features::write_features(const sail_write_features *wf)
    : write_features()
{
    if (wf == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::write_features()");
        return;
    }

    d->sail_write_features_c = wf;

    std::vector<int> input_pixel_formats;

    if (wf->input_pixel_formats != nullptr && wf->input_pixel_formats_length > 0) {
        input_pixel_formats.reserve(wf->input_pixel_formats_length);

        for (int i = 0; i < wf->input_pixel_formats_length; i++) {
            input_pixel_formats.push_back(wf->input_pixel_formats[i]);
        }
    }

    std::vector<int> output_pixel_formats;

    if (wf->output_pixel_formats != nullptr && wf->output_pixel_formats_length > 0) {
        output_pixel_formats.reserve(wf->output_pixel_formats_length);

        for (int i = 0; i < wf->output_pixel_formats_length; i++) {
            output_pixel_formats.push_back(wf->output_pixel_formats[i]);
        }
    }

    std::vector<int> compression_types;

    if (wf->compression_types != nullptr && wf->compression_types_length > 0) {
        compression_types.reserve(wf->compression_types_length);

        for (int i = 0; i < wf->compression_types_length; i++) {
            compression_types.push_back(wf->compression_types[i]);
        }
    }

    with_input_pixel_formats(input_pixel_formats)
        .with_output_pixel_formats(output_pixel_formats)
        .with_preferred_output_pixel_format(wf->preferred_output_pixel_format)
        .with_features(wf->features)
        .with_properties(wf->properties)
        .with_passes(wf->passes)
        .with_compression_types(compression_types)
        .with_preferred_compression_type(wf->preferred_compression_type)
        .with_compression_min(wf->compression_min)
        .with_compression_max(wf->compression_max)
        .with_compression_default(wf->compression_default);
}

write_features::write_features(const write_features &wf)
    : write_features()
{
    *this = wf;
}

write_features& write_features::operator=(const write_features &wf)
{
    d->sail_write_features_c = wf.d->sail_write_features_c;

    with_input_pixel_formats(wf.input_pixel_formats())
        .with_output_pixel_formats(wf.output_pixel_formats())
        .with_preferred_output_pixel_format(wf.preferred_output_pixel_format())
        .with_features(wf.features());

    return *this;
}

write_features::~write_features()
{
    delete d;
}

std::vector<int> write_features::input_pixel_formats() const
{
    return d->input_pixel_formats;
}

std::vector<int> write_features::output_pixel_formats() const
{
    return d->output_pixel_formats;
}

int write_features::preferred_output_pixel_format() const
{
    return d->preferred_output_pixel_format;
}

int write_features::features() const
{
    return d->features;
}

int write_features::properties() const
{
    return d->properties;
}

int write_features::passes() const
{
    return d->passes;
}

std::vector<int> write_features::compression_types() const
{
    return d->compression_types;
}

int write_features::preferred_compression_type() const
{
    return d->preferred_compression_type;
}

int write_features::compression_min() const
{
    return d->compression_min;
}

int write_features::compression_max() const
{
    return d->compression_max;
}

int write_features::compression_default() const
{
    return d->compression_default;
}

sail_error_t write_features::to_write_options(write_options **swrite_options) const
{
    SAIL_CHECK_WRITE_FEATURES_PTR(d->sail_write_features_c);
    SAIL_CHECK_WRITE_OPTIONS_PTR(swrite_options);

    sail_write_options *sail_write_options = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_write_options(sail_write_options);
    );

    SAIL_TRY(sail_alloc_write_options_from_features(d->sail_write_features_c, &sail_write_options));

    *swrite_options = new write_options(sail_write_options);

    if (*swrite_options == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    return 0;
}

write_features& write_features::with_input_pixel_formats(const std::vector<int> &input_pixel_formats)
{
    d->input_pixel_formats = input_pixel_formats;
    return *this;
}

write_features& write_features::with_output_pixel_formats(const std::vector<int> &output_pixel_formats)
{
    d->output_pixel_formats = output_pixel_formats;
    return *this;
}

write_features& write_features::with_preferred_output_pixel_format(int preferred_output_pixel_format)
{
    d->preferred_output_pixel_format = preferred_output_pixel_format;
    return *this;
}

write_features& write_features::with_features(int features)
{
    d->features = features;
    return *this;
}

write_features& write_features::with_properties(int properties)
{
    d->properties = properties;
    return *this;
}

write_features& write_features::with_passes(int passes)
{
    d->passes = passes;
    return *this;
}

write_features& write_features::with_compression_types(const std::vector<int> &compression_types)
{
    d->compression_types = compression_types;
    return *this;
}

write_features& write_features::with_preferred_compression_type(int preferred_compression_type)
{
    d->preferred_compression_type = preferred_compression_type;
    return *this;
}

write_features& write_features::with_compression_min(int compression_min)
{
    d->compression_min = compression_min;
    return *this;
}

write_features& write_features::with_compression_max(int compression_max)
{
    d->compression_max = compression_max;
    return *this;
}

write_features& write_features::with_compression_default(int compression_default)
{
    d->compression_default = compression_default;
    return *this;
}

const sail_write_features* write_features::sail_write_features_c() const
{
    return d->sail_write_features_c;
}

}
