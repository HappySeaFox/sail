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

#include "config.h"

#include "sail-common.h"
#include "sail.h"

sail_error_t load_plugin(struct sail_plugin_info_node *node) {

    SAIL_CHECK_PTR(node);

    /* Already loaded. */
    if (node->plugin != NULL) {
        return 0;
    }

    struct sail_plugin *plugin;

    /* Plugin is not loaded. Let's load it. */
    SAIL_TRY(sail_alloc_plugin(node->plugin_info, &plugin));

    node->plugin = plugin;

    return 0;
}

sail_error_t load_plugin_by_plugin_info(struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                        const struct sail_plugin **plugin) {

    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);
    SAIL_CHECK_PLUGIN_PTR(plugin);

    /* Find the plugin in the cache. */
    struct sail_plugin_info_node *node = context->plugin_info_node;
    struct sail_plugin_info_node *found_node = NULL;

    while (node != NULL) {
        if (node->plugin_info == plugin_info) {
            if (node->plugin != NULL) {
                *plugin = node->plugin;
                return 0;
            }

            found_node = node;
            break;
        }

        node = node->next;
    }

    /* Something weird. The pointer to the plugin info is not found the cache. */
    if (found_node == NULL) {
        return SAIL_PLUGIN_NOT_FOUND;
    }

    SAIL_TRY(load_plugin(found_node));

    *plugin = found_node->plugin;

    return 0;
}
