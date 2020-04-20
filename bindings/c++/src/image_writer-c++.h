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

#ifndef SAIL_IMAGE_WRITER_CPP_H
#define SAIL_IMAGE_WRITER_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

namespace sail
{

class context;
class image;
class plugin_info;

/*
 * A C++ interface to the SAIL image writing functions.
 */
class SAIL_EXPORT image_writer
{
public:
    image_writer(context *ctx);
    ~image_writer();

    bool is_valid() const;

    /*
     * An interface to sail_write(). See sail_write() for more.
     */
    sail_error_t write(const char *path, const image *simage, plugin_info **splugin_info = nullptr);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
