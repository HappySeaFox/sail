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
    #include "image-c++.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/image-c++.h>
#endif

struct sail_plugin_info;

namespace sail
{

class context;

/*
 * A C++ interface to struct sail_image.
 */
class SAIL_EXPORT image_reader
{
public:
    image_reader(context *ctx);
    ~image_reader();

    bool is_valid() const;

    sail_error_t read(const char *path, image **simage, const sail_plugin_info **plugin_info = nullptr);

private:
    class pimpl;
    const std::unique_ptr<pimpl> d;
};

}

#endif
