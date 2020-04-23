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

#ifndef SAIL_WRITE_FEATURES_CPP_H
#define SAIL_WRITE_FEATURES_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <vector>

struct sail_write_features;

namespace sail
{

class write_options;

/*
 * A C++ interface to struct sail_write_features.
 */
class SAIL_EXPORT write_features
{
    friend class plugin_info;

public:
    write_features(const write_features &wf);
    write_features& operator=(const write_features &wf);
    ~write_features();

    std::vector<int> input_pixel_formats() const;
    std::vector<int> output_pixel_formats() const;
    int preferred_output_pixel_format() const;
    int features() const;
    int properties() const;
    int passes() const;
    std::vector<int> compression_types() const;
    int preferred_compression_type() const;
    int compression_min() const;
    int compression_max() const;
    int compression_default() const;

    sail_error_t to_write_options(write_options **swrite_options) const;

private:
    write_features();
    /*
     * Makes a deep copy of the specified write features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    write_features(const sail_write_features *wf);

    write_features& with_input_pixel_formats(const std::vector<int> &input_pixel_formats);
    write_features& with_output_pixel_formats(const std::vector<int> &output_pixel_formats);
    write_features& with_preferred_output_pixel_format(int preferred_output_pixel_format);
    write_features& with_features(int features);
    write_features& with_properties(int properties);
    write_features& with_passes(int passes);
    write_features& with_compression_types(const std::vector<int> &compression_types);
    write_features& with_preferred_compression_type(int preferred_compression_type);
    write_features& with_compression_min(int compression_min);
    write_features& with_compression_max(int compression_max);
    write_features& with_compression_default(int compression_default);

    const sail_write_features* sail_write_features_c() const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
