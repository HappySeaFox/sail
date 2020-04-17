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

#include "config.h"

// libsail-common.
#include "error.h"
#include "log.h"

// libsail.
#include "plugin_info.h"
#include "string_node.h"

#include "plugin_info-c++.h"

namespace sail
{

class plugin_info::pimpl
{
public:
    std::string version;
    std::string name;
    std::string description;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;
};

plugin_info::plugin_info()
    : d(new pimpl)
{
}

plugin_info::plugin_info(const plugin_info &pi)
    : plugin_info()
{
    with_version(pi.version())
    .with_name(pi.name())
    .with_description(pi.description())
    .with_extensions(pi.extensions())
    .with_mime_types(pi.mime_types());
}

plugin_info::plugin_info(const sail_plugin_info *pi)
    : plugin_info()
{
    if (pi == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to plugin_info()");
        return;
    }

    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;

    const sail_string_node *extension_node = pi->extension_node;

    while (extension_node != nullptr) {
        extensions.push_back(extension_node->value);
        extension_node = extension_node->next;
    }

    const sail_string_node *mime_type_node = pi->mime_type_node;

    while (mime_type_node != nullptr) {
        mime_types.push_back(mime_type_node->value);
        mime_type_node = mime_type_node->next;
    }

    with_version(pi->version)
    .with_name(pi->name)
    .with_description(pi->description)
    .with_extensions(extensions)
    .with_mime_types(mime_types);
}

plugin_info::~plugin_info()
{
}

std::string plugin_info::version() const
{
    return d->version;
}

std::string plugin_info::name() const
{
    return d->name;
}

std::string plugin_info::description() const
{
    return d->description;
}

std::vector<std::string> plugin_info::extensions() const
{
    return d->extensions;
}

std::vector<std::string> plugin_info::mime_types() const
{
    return d->mime_types;
}

plugin_info& plugin_info::with_version(const std::string &version)
{
    d->version = version;
    return *this;
}

plugin_info& plugin_info::with_name(const std::string &name)
{
    d->name = name;
    return *this;
}

plugin_info& plugin_info::with_description(const std::string &description)
{
    d->description = description;
    return *this;
}

plugin_info& plugin_info::with_extensions(const std::vector<std::string> &extensions)
{
    d->extensions = extensions;
    return *this;
}

plugin_info& plugin_info::with_mime_types(const std::vector<std::string> &mime_types)
{
    d->mime_types = mime_types;
    return *this;
}

}
