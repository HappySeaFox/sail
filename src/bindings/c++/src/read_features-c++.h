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

#ifndef SAIL_READ_FEATURES_CPP_H
#define SAIL_READ_FEATURES_CPP_H

#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_read_features;

namespace sail
{

class read_options;

/*
 * A C++ interface to struct sail_read_features.
 */
class SAIL_EXPORT read_features
{
    friend class plugin_info;

public:
    read_features(const read_features &rf);
    read_features& operator=(const read_features &rf);
    ~read_features();

    std::vector<int> output_pixel_formats() const;
    int preferred_output_pixel_format() const;
    int features() const;

    sail_error_t to_read_options(read_options *sread_options) const;

private:
    read_features();
    /*
     * Makes a deep copy of the specified read features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    read_features(const sail_read_features *rf);

    read_features& with_output_pixel_formats(const std::vector<int> &output_pixel_formats);
    read_features& with_preferred_output_pixel_format(int preferred_output_pixel_format);
    read_features& with_features(int features);

    const sail_read_features* sail_read_features_c() const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
