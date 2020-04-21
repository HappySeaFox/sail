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

// libsail.
#include "context.h"
#include "plugin_info.h"
#include "sail.h"

#include "context-c++.h"
#include "plugin_info-c++.h"

namespace sail
{

class SAIL_HIDDEN context::pimpl
{
public:
    sail_context *context = nullptr;
};

context::context()
    : d(new pimpl)
{
    init();
}

context::~context()
{
    sail_finish(d->context);
    delete d;
}

bool context::is_valid() const
{
    return d->context != nullptr;
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

sail_error_t context::plugin_by_extension(const std::string &suffix, plugin_info **splugin_info, const plugin **splugin) const
{
    SAIL_TRY(plugin_by_extension(suffix.c_str(), splugin_info, splugin));

    return 0;
}

sail_error_t context::plugin_by_extension(const char *suffix, plugin_info **splugin_info, const plugin **splugin) const
{
    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_by_extension(d->context, suffix, &sail_plugin_info, splugin));

    *splugin_info = new plugin_info(sail_plugin_info);

    if (*splugin_info == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    return 0;
}

sail_error_t context::plugin_by_mime_type(const std::string &mime_type, plugin_info **splugin_info, const plugin **splugin) const
{
    SAIL_TRY(plugin_by_extension(mime_type.c_str(), splugin_info, splugin));

    return 0;
}

sail_error_t context::plugin_by_mime_type(const char *mime_type, plugin_info **splugin_info, const plugin **splugin) const
{
    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_by_mime_type(d->context, mime_type, &sail_plugin_info, splugin));

    *splugin_info = new plugin_info(sail_plugin_info);

    if (*splugin_info == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    return 0;
}

sail_context* context::to_sail_context() const
{
    return d->context;
}

sail_error_t context::init()
{
    SAIL_TRY(sail_init(&d->context));

    return 0;
}

}
