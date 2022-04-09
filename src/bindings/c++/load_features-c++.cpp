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

class SAIL_HIDDEN load_features::pimpl
{
public:
    pimpl()
        : sail_load_features_c(nullptr)
    {}

    const sail_load_features *sail_load_features_c;
    sail::supported_tuning supported_tuning;
};

load_features::load_features(const load_features &rf)
    : load_features()
{
    *this = rf;
}

load_features& load_features::operator=(const sail::load_features &load_features)
{
    d->sail_load_features_c = load_features.d->sail_load_features_c;
    d->supported_tuning     = load_features.d->supported_tuning;

    return *this;
}

load_features::load_features(sail::load_features &&load_features) noexcept
{
    *this = std::move(load_features);
}

load_features& load_features::operator=(sail::load_features &&load_features) noexcept
{
    d = std::move(load_features.d);

    return *this;
}

load_features::~load_features()
{
}

int load_features::features() const
{
    return d->sail_load_features_c->features;
}

const sail::supported_tuning& load_features::supported_tuning() const
{
    return d->supported_tuning;
}

sail_status_t load_features::to_options(sail::load_options *load_options) const
{
    SAIL_CHECK_PTR(d->sail_load_features_c);
    SAIL_CHECK_PTR(load_options);

    sail_load_options *sail_load_options;

    SAIL_TRY(sail_alloc_load_options_from_features(d->sail_load_features_c, &sail_load_options));

    *load_options = sail::load_options(sail_load_options);

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

load_features::load_features()
    : d(new pimpl)
{
}

load_features::load_features(const sail_load_features *rf)
    : load_features()
{
    if (rf == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::load_features(). The object is untouched");
        return;
    }

    d->sail_load_features_c = rf;

    for (const sail_string_node *node = rf->tuning; node != nullptr; node = node->next) {
        d->supported_tuning.push_back(node->string);
    }
}

const sail_load_features* load_features::sail_load_features_c() const
{
    return d->sail_load_features_c;
}

}
