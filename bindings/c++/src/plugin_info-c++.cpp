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
#include "common.h"
#include "error.h"
#include "log.h"

// libsail.
#include "plugin_info.h"
#include "string_node.h"

#include "plugin_info-c++.h"

namespace sail
{

class SAIL_HIDDEN plugin_info::pimpl
{
public:
    pimpl()
        : sail_plugin_info_c(nullptr)
    {}

    const sail_plugin_info *sail_plugin_info_c;

    std::string path;
    std::string version;
    std::string name;
    std::string description;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;
    sail::read_features read_features;
    sail::write_features write_features;
};

plugin_info::plugin_info()
    : d(new pimpl)
{
}

plugin_info::plugin_info(const sail_plugin_info *pi)
    : plugin_info()
{
    if (pi == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::plugin_info()");
        return;
    }

    d->sail_plugin_info_c = pi;

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

    with_path(pi->path)
        .with_version(pi->version)
        .with_name(pi->name)
        .with_description(pi->description)
        .with_extensions(extensions)
        .with_mime_types(mime_types)
        .with_read_features(pi->read_features)
        .with_write_features(pi->write_features);
}

plugin_info::plugin_info(const plugin_info &pi)
    : plugin_info()
{
    *this = pi;
}

plugin_info& plugin_info::operator=(const plugin_info &pi)
{
    d->sail_plugin_info_c = pi.d->sail_plugin_info_c;

    with_path(pi.path())
        .with_version(pi.version())
        .with_name(pi.name())
        .with_description(pi.description())
        .with_extensions(pi.extensions())
        .with_mime_types(pi.mime_types())
        .with_read_features(pi.read_features())
        .with_write_features(pi.write_features());

    return *this;
}

plugin_info::~plugin_info()
{
    delete d;
}

std::string plugin_info::path() const
{
    return d->path;
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

read_features plugin_info::read_features() const
{
    return d->read_features;
}

write_features plugin_info::write_features() const
{
    return d->write_features;
}

plugin_info& plugin_info::with_path(const std::string &path)
{
    d->path = path;
    return *this;
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

plugin_info& plugin_info::with_read_features(const sail::read_features &read_features)
{
    d->read_features = read_features;
    return *this;
}

plugin_info& plugin_info::with_write_features(const sail::write_features &write_features)
{
    d->write_features = write_features;
    return *this;
}

const sail_plugin_info* plugin_info::to_sail_plugin_info() const
{
    return d->sail_plugin_info_c;
}

}
