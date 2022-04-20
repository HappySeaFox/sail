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

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN save_features::pimpl
{
public:
    pimpl()
        : sail_save_features_c(nullptr)
    {}

    const sail_save_features *sail_save_features_c;

    std::vector<SailPixelFormat> pixel_formats;
    std::vector<SailCompression> compressions;
    sail::compression_level compression_level;
    sail::supported_tuning supported_tuning;
};

save_features::save_features(const save_features &wf)
    : save_features()
{
    *this = wf;
}

save_features& save_features::operator=(const sail::save_features &save_features)
{
    d->sail_save_features_c = save_features.d->sail_save_features_c;
    d->pixel_formats        = save_features.d->pixel_formats;
    d->compressions         = save_features.d->compressions;
    d->compression_level    = save_features.d->compression_level;
    d->supported_tuning     = save_features.d->supported_tuning;

    return *this;
}

save_features::save_features(sail::save_features &&save_features) noexcept
{
    *this = std::move(save_features);
}

save_features& save_features::operator=(sail::save_features &&save_features) noexcept
{
    d = std::move(save_features.d);

    return *this;
}

save_features::~save_features()
{
}

const std::vector<SailPixelFormat>& save_features::pixel_formats() const
{
    return d->pixel_formats;
}

int save_features::features() const
{
    return d->sail_save_features_c->features;
}

const std::vector<SailCompression>& save_features::compressions() const
{
    return d->compressions;
}

SailCompression save_features::default_compression() const
{
    return d->sail_save_features_c->default_compression;
}

const sail::compression_level& save_features::compression_level() const
{
    return d->compression_level;
}

const sail::supported_tuning& save_features::supported_tuning() const
{
    return d->supported_tuning;
}

sail_status_t save_features::to_options(sail::save_options *save_options) const
{
    SAIL_CHECK_PTR(d->sail_save_features_c);
    SAIL_CHECK_PTR(save_options);

    sail_save_options *sail_save_options;

    SAIL_TRY(sail_alloc_save_options_from_features(d->sail_save_features_c, &sail_save_options));

    *save_options = sail::save_options(sail_save_options);

    sail_destroy_save_options(sail_save_options);

    return SAIL_OK;
}

save_features::save_features()
    : d(new pimpl)
{
}

save_features::save_features(const sail_save_features *wf)
    : save_features()
{
    if (wf == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::save_features(). The object is untouched");
        return;
    }

    d->sail_save_features_c = wf;

    // Output pixel formats
    std::vector<SailPixelFormat> pixel_formats;

    if (d->sail_save_features_c->pixel_formats != nullptr && d->sail_save_features_c->pixel_formats_length > 0) {
        pixel_formats.reserve(d->sail_save_features_c->pixel_formats_length);

        for (unsigned i = 0; i < d->sail_save_features_c->pixel_formats_length; i++) {
            pixel_formats.push_back(d->sail_save_features_c->pixel_formats[i]);
        }
    }

    d->pixel_formats = pixel_formats;

    // Compressions
    std::vector<SailCompression> compressions;

    if (d->sail_save_features_c->compressions != nullptr && d->sail_save_features_c->compressions_length > 0) {
        compressions.reserve(d->sail_save_features_c->compressions_length);

        for (unsigned i = 0; i < d->sail_save_features_c->compressions_length; i++) {
            compressions.push_back(d->sail_save_features_c->compressions[i]);
        }
    }

    d->compressions = compressions;

    // Compression level
    if (wf->compression_level != nullptr) {
        d->compression_level = sail::compression_level(wf->compression_level);
    }

    // Supported tuning
    for (const sail_string_node *node = wf->tuning; node != nullptr; node = node->next) {
        d->supported_tuning.push_back(node->string);
    }
}

const sail_save_features* save_features::sail_save_features_c() const
{
    return d->sail_save_features_c;
}

}
