/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN context::pimpl
{
public:
    sail_context *context = nullptr;
    sail_error_t init_result = SAIL_CONTEXT_UNINITIALIZED;
};

context::context()
    : d(new pimpl)
{
    d->init_result = init(0);

    if (d->init_result != 0) {
        SAIL_LOG_ERROR("Failed to initialize SAIL. Error: %d", d->init_result);
    }
}

context::context(int flags)
    : d(new pimpl)
{
    d->init_result = init(flags);

    if (d->init_result != 0) {
        SAIL_LOG_ERROR("Failed to initialize SAIL. Error: %d", d->init_result);
    }
}

context::~context()
{
    sail_finish(d->context);
    delete d;
}

sail_error_t context::status() const
{
    return d->init_result;
}

std::vector<plugin_info> context::plugin_info_list() const
{
    std::vector<plugin_info> list;

    const sail_plugin_info_node *plugin_info_node = sail_plugin_info_list(d->context);

    while (plugin_info_node != nullptr) {
        list.push_back(plugin_info(plugin_info_node->plugin_info));
        plugin_info_node = plugin_info_node->next;
    }

    return list;
}

sail_error_t context::unload_plugins()
{
    SAIL_TRY(sail_unload_plugins(d->context));

    return 0;
}

sail_error_t context::plugin_info_from_path(const std::string &path, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_path(path.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_path(const char *path, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_path(path, d->context, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::plugin_info_from_extension(const std::string &suffix, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_extension(suffix.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_extension(const char *suffix, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_extension(suffix, d->context, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::plugin_info_from_mime_type(const std::string &mime_type, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_mime_type(mime_type.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_mime_type(const char *mime_type, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_mime_type(d->context, mime_type, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::init(int flags)
{
    SAIL_TRY(sail_init_with_flags(&d->context, flags));

    return 0;
}

sail_context* context::sail_context_c() const
{
    return d->context;
}

}
