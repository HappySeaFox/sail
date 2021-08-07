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

class SAIL_HIDDEN write_features::pimpl
{
public:
    pimpl()
        : sail_write_features_c(nullptr)
    {}

    const sail_write_features *sail_write_features_c;

    std::vector<SailPixelFormat> output_pixel_formats;
    std::vector<SailCompression> compressions;
};

write_features::write_features(const write_features &wf)
    : write_features()
{
    *this = wf;
}

write_features& write_features::operator=(const sail::write_features &write_features)
{
    d->sail_write_features_c = write_features.d->sail_write_features_c;
    d->output_pixel_formats  = write_features.d->output_pixel_formats;
    d->compressions          = write_features.d->compressions;

    return *this;
}

write_features::write_features(sail::write_features &&write_features) noexcept
{
    d = write_features.d;
    write_features.d = nullptr;
}

write_features& write_features::operator=(sail::write_features &&write_features)
{
    delete d;
    d = write_features.d;
    write_features.d = nullptr;

    return *this;
}

write_features::~write_features()
{
    delete d;
}

const std::vector<SailPixelFormat>& write_features::output_pixel_formats() const
{
    return d->output_pixel_formats;
}

int write_features::features() const
{
    return d->sail_write_features_c->features;
}

int write_features::properties() const
{
    return d->sail_write_features_c->properties;
}

const std::vector<SailCompression>& write_features::compressions() const
{
    return d->compressions;
}

SailCompression write_features::default_compression() const
{
    return d->sail_write_features_c->default_compression;
}

double write_features::compression_level_min() const
{
    return d->sail_write_features_c->compression_level_min;
}

double write_features::compression_level_max() const
{
    return d->sail_write_features_c->compression_level_max;
}

double write_features::compression_level_default() const
{
    return d->sail_write_features_c->compression_level_default;
}

double write_features::compression_level_step() const
{
    return d->sail_write_features_c->compression_level_step;
}

sail_status_t write_features::to_write_options(sail::write_options *write_options) const
{
    SAIL_CHECK_WRITE_FEATURES_PTR(d->sail_write_features_c);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    sail_write_options *sail_write_options;

    SAIL_TRY(sail_alloc_write_options_from_features(d->sail_write_features_c, &sail_write_options));

    *write_options = sail::write_options(sail_write_options);

    sail_destroy_write_options(sail_write_options);

    return SAIL_OK;
}

write_features::write_features()
    : d(new pimpl)
{
}

write_features::write_features(const sail_write_features *write_features)
    : write_features()
{
    if (write_features == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::write_features(). The object is untouched");
        return;
    }

    d->sail_write_features_c = write_features;

    // Output pixel formats
    std::vector<SailPixelFormat> output_pixel_formats;

    if (write_features->output_pixel_formats != nullptr && write_features->output_pixel_formats_length > 0) {
        output_pixel_formats.reserve(write_features->output_pixel_formats_length);

        for (unsigned i = 0; i < write_features->output_pixel_formats_length; i++) {
            output_pixel_formats.push_back(write_features->output_pixel_formats[i]);
        }
    }

    d->output_pixel_formats = output_pixel_formats;

    // Compressions
    std::vector<SailCompression> compressions;

    if (write_features->compressions != nullptr && write_features->compressions_length > 0) {
        compressions.reserve(write_features->compressions_length);

        for (unsigned i = 0; i < write_features->compressions_length; i++) {
            compressions.push_back(write_features->compressions[i]);
        }
    }

    d->compressions = compressions;
}

const sail_write_features* write_features::sail_write_features_c() const
{
    return d->sail_write_features_c;
}

}
