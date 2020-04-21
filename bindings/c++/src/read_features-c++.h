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

#ifndef SAIL_READ_FEATURES_CPP_H
#define SAIL_READ_FEATURES_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <vector>

struct sail_read_features;

namespace sail
{

class read_options;

/*
 * A C++ interface to struct sail_read_features.
 */
class SAIL_EXPORT read_features
{
public:
    read_features();
    // Makes a deep copy of the specified read features
    //
    read_features(const sail_read_features *rf);
    read_features(const read_features &rf);
    read_features& operator=(const read_features &rf);
    ~read_features();

    std::vector<int> input_pixel_formats() const;
    std::vector<int> output_pixel_formats() const;
    int preferred_output_pixel_format() const;
    int features() const;

    sail_error_t to_read_options(read_options **sread_options) const;

private:
    read_features& with_input_pixel_formats(const std::vector<int> &input_pixel_formats);
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
