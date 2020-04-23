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

#ifndef SAIL_READ_OPTIONS_CPP_H
#define SAIL_READ_OPTIONS_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <vector>

struct sail_read_options;

namespace sail
{

/*
 * A C++ interface to struct sail_read_options.
 */
class SAIL_EXPORT read_options
{
    friend class image_reader;

public:
    read_options();
    /*
     * Makes a deep copy of the specified read options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    read_options(const sail_read_options *ro);
    read_options(const read_options &ro);
    read_options& operator=(const read_options &ro);
    ~read_options();

    int output_pixel_format() const;
    int io_options() const;

    read_options& with_output_pixel_format(int output_pixel_format);
    read_options& with_io_options(int io_options);

private:
    sail_error_t to_sail_read_options(sail_read_options **read_options) const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
