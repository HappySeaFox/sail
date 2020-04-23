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

#ifndef SAIL_WRITE_OPTIONS_CPP_H
#define SAIL_WRITE_OPTIONS_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <vector>

struct sail_write_options;

namespace sail
{

/*
 * A C++ interface to struct sail_write_options.
 */
class SAIL_EXPORT write_options
{
    friend class image_writer;

public:
    write_options();
    /*
     * Makes a deep copy of the specified write options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    write_options(const sail_write_options *ro);
    write_options(const write_options &ro);
    write_options& operator=(const write_options &ro);
    ~write_options();

    int output_pixel_format() const;
    int io_options() const;
    int compression_type() const;
    int compression() const;

    write_options& with_output_pixel_format(int output_pixel_format);
    write_options& with_io_options(int io_options);
    write_options& with_compression_type(int compression_type);
    write_options& with_compression(int compression);

private:
    sail_error_t to_sail_write_options(sail_write_options **write_options) const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
