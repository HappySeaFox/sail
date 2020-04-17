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
#include "string_node.h"

#include "plugin_info-c++.h"
#include "sail-c++.h"

namespace sail
{

class context::pimpl
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
        plugin_info pi;

        std::vector<std::string> extensions;
        std::vector<std::string> mime_types;

        const sail_string_node *extension_node = plugin_info_node->plugin_info->extension_node;

        while (extension_node != nullptr) {
            extensions.push_back(extension_node->value);
            extension_node = extension_node->next;
        }

        const sail_string_node *mime_type_node = plugin_info_node->plugin_info->mime_type_node;

        while (mime_type_node != nullptr) {
            mime_types.push_back(mime_type_node->value);
            mime_type_node = mime_type_node->next;
        }

        pi
            .with_version(plugin_info_node->plugin_info->version)
            .with_name(plugin_info_node->plugin_info->name)
            .with_description(plugin_info_node->plugin_info->description)
            .with_extensions(extensions)
            .with_mime_types(mime_types);

        list.push_back(pi);

        plugin_info_node = plugin_info_node->next;
    }

    return list;
}

sail_error_t context::init()
{
    SAIL_TRY(sail_init(&d->context));

    return 0;
}

}
