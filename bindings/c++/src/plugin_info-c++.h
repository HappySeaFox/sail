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

#ifndef SAIL_PLUGIN_INFO_CPP_H
#define SAIL_PLUGIN_INFO_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
    #include "read_features-c++.h"
    #include "write_features-c++.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/read_features-c++.h>
    #include <sail/write_features-c++.h>
#endif

#include <string>
#include <vector>

struct sail_plugin_info;

namespace sail
{

/*
 * A C++ interface to struct sail_plugin_info.
 */
class SAIL_EXPORT plugin_info
{
public:
    plugin_info();
    // Makes a deep copy of the specified plugin info
    //
    plugin_info(const sail_plugin_info *pi);
    plugin_info(const plugin_info &pi);
    plugin_info& operator=(const plugin_info &pi);
    ~plugin_info();

    std::string version() const;
    std::string name() const;
    std::string description() const;
    std::vector<std::string> extensions() const;
    std::vector<std::string> mime_types() const;
    sail::read_features read_features() const;
    sail::write_features write_features() const;

private:
    plugin_info& with_version(const std::string &version);
    plugin_info& with_name(const std::string &name);
    plugin_info& with_description(const std::string &description);
    plugin_info& with_extensions(const std::vector<std::string> &extensions);
    plugin_info& with_mime_types(const std::vector<std::string> &mime_types);
    plugin_info& with_read_features(const sail::read_features &read_features);
    plugin_info& with_write_features(const sail::write_features &write_features);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
