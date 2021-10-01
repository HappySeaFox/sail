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

class SAIL_HIDDEN read_features::pimpl
{
public:
    pimpl()
        : sail_read_features_c(nullptr)
    {}

    const sail_read_features *sail_read_features_c;
};

read_features::read_features(const read_features &rf)
    : read_features()
{
    *this = rf;
}

read_features& read_features::operator=(const sail::read_features &read_features)
{
    d->sail_read_features_c = read_features.d->sail_read_features_c;

    return *this;
}

read_features::read_features(sail::read_features &&read_features) noexcept
{
    d = read_features.d;
    read_features.d = nullptr;
}

read_features& read_features::operator=(sail::read_features &&read_features) noexcept
{
    delete d;
    d = read_features.d;
    read_features.d = nullptr;

    return *this;
}

read_features::~read_features()
{
    delete d;
}

int read_features::features() const
{
    return d->sail_read_features_c->features;
}

sail_status_t read_features::to_read_options(sail::read_options *read_options) const
{
    SAIL_CHECK_PTR(d->sail_read_features_c);
    SAIL_CHECK_PTR(read_options);

    sail_read_options *sail_read_options;

    SAIL_TRY(sail_alloc_read_options_from_features(d->sail_read_features_c, &sail_read_options));

    *read_options = sail::read_options(sail_read_options);

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
}

const sail_read_features* read_features::sail_read_features_c() const
{
    return d->sail_read_features_c;
}

}
