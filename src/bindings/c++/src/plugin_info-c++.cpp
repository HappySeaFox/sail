/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

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
    std::vector<std::string> magic_numbers;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;
    sail::read_features read_features;
    sail::write_features write_features;
};

plugin_info::plugin_info()
    : d(new pimpl)
{
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
        .with_magic_numbers(pi.magic_numbers())
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

const std::string& plugin_info::path() const
{
    return d->path;
}

const std::string& plugin_info::version() const
{
    return d->version;
}

const std::string& plugin_info::name() const
{
    return d->name;
}

const std::string& plugin_info::description() const
{
    return d->description;
}

const std::vector<std::string>& plugin_info::magic_numbers() const
{
    return d->magic_numbers;
}

const std::vector<std::string>& plugin_info::extensions() const
{
    return d->extensions;
}

const std::vector<std::string>& plugin_info::mime_types() const
{
    return d->mime_types;
}

const read_features& plugin_info::read_features() const
{
    return d->read_features;
}

const write_features& plugin_info::write_features() const
{
    return d->write_features;
}

sail_error_t plugin_info::plugin_feature_to_string(SailPluginFeature plugin_feature, const char **result)
{
    SAIL_TRY(sail_plugin_feature_to_string(plugin_feature, result));

    return 0;
}

sail_error_t plugin_info::plugin_feature_from_string(const char *str, SailPluginFeature *result)
{
    SAIL_TRY(sail_plugin_feature_from_string(str, result));

    return 0;
}

sail_error_t plugin_info::by_magic_number_from_path(const std::string &path, plugin_info *splugin_info)
{
    SAIL_TRY(by_magic_number_from_path(path.c_str(), splugin_info));

    return 0;
}

sail_error_t plugin_info::by_magic_number_from_path(const char *path, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_by_magic_number_from_path(path, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t plugin_info::by_magic_number_from_mem(const void *buffer, size_t buffer_length, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_by_magic_number_from_mem(buffer, buffer_length, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t plugin_info::by_magic_number_from_io(const sail::io &io, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);
    SAIL_TRY(io.verify_valid());

    struct sail_io sail_io;
    SAIL_TRY(io.to_sail_io(&sail_io));

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_by_magic_number_from_io(&sail_io, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t plugin_info::from_path(const std::string &path, plugin_info *splugin_info)
{
    SAIL_TRY(from_path(path.c_str(), splugin_info));

    return 0;
}

sail_error_t plugin_info::from_path(const char *path, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_path(path, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t plugin_info::from_extension(const std::string &suffix, plugin_info *splugin_info)
{
    SAIL_TRY(from_extension(suffix.c_str(), splugin_info));

    return 0;
}

sail_error_t plugin_info::from_extension(const char *suffix, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_extension(suffix, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t plugin_info::from_mime_type(const std::string &mime_type, plugin_info *splugin_info)
{
    SAIL_TRY(from_mime_type(mime_type.c_str(), splugin_info));

    return 0;
}

sail_error_t plugin_info::from_mime_type(const char *mime_type, plugin_info *splugin_info)
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_mime_type(mime_type, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

std::vector<plugin_info> plugin_info::list()
{
    std::vector<plugin_info> plugin_info_list;
    const sail_plugin_info_node *plugin_info_node = sail_plugin_info_list();

    while (plugin_info_node != nullptr) {
        plugin_info_list.push_back(plugin_info(plugin_info_node->plugin_info));
        plugin_info_node = plugin_info_node->next;
    }

    return plugin_info_list;
}

plugin_info::plugin_info(const sail_plugin_info *pi)
    : plugin_info()
{
    if (pi == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::plugin_info(). The object is untouched");
        return;
    }

    d->sail_plugin_info_c = pi;

    std::vector<std::string> magic_numbers;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;

    // magic numbers
    const sail_string_node *magic_number_node = pi->magic_number_node;

    while (magic_number_node != nullptr) {
        magic_numbers.push_back(magic_number_node->value);
        magic_number_node = magic_number_node->next;
    }

    // extensions
    const sail_string_node *extension_node = pi->extension_node;

    while (extension_node != nullptr) {
        extensions.push_back(extension_node->value);
        extension_node = extension_node->next;
    }

    // mime types
    const sail_string_node *mime_type_node = pi->mime_type_node;

    while (mime_type_node != nullptr) {
        mime_types.push_back(mime_type_node->value);
        mime_type_node = mime_type_node->next;
    }

    with_path(pi->path)
        .with_version(pi->version)
        .with_name(pi->name)
        .with_description(pi->description)
        .with_magic_numbers(magic_numbers)
        .with_extensions(extensions)
        .with_mime_types(mime_types)
        .with_read_features(pi->read_features)
        .with_write_features(pi->write_features);
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

plugin_info& plugin_info::with_magic_numbers(const std::vector<std::string> &magic_numbers)
{
    d->magic_numbers = magic_numbers;
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

const sail_plugin_info* plugin_info::sail_plugin_info_c() const
{
    return d->sail_plugin_info_c;
}

}
