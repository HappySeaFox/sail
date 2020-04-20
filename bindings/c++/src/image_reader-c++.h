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

#ifndef SAIL_IMAGE_READER_CPP_H
#define SAIL_IMAGE_READER_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <string>

namespace sail
{

class context;
class image;
class plugin_info;

/*
 * A C++ interface to the SAIL image reading functions.
 */
class SAIL_EXPORT image_reader
{
public:
    image_reader(context *ctx);
    ~image_reader();

    bool is_valid() const;

    /*
     * An interface to sail_probe(). See sail_probe() for more.
     */
    sail_error_t probe(const std::string &path, image **simage, plugin_info **splugin_info = nullptr);

    /*
     * An interface to sail_read(). See sail_read() for more.
     */
    sail_error_t read(const std::string &path, image **simage, plugin_info **splugin_info = nullptr);

    /*
     * An interface to sail_start_reading(). See sail_start_reading() for more.
     */
    sail_error_t start_reading(const std::string &path, plugin_info **splugin_info = nullptr);

    /*
     * An interface to sail_read_next_frame(). See sail_read_next_frame() for more.
     */
    sail_error_t read_next_frame(image **simage);

    /*
     * An interface to sail_stop_reading(). See sail_stop_reading() for more.
     */
    sail_error_t stop_reading();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
