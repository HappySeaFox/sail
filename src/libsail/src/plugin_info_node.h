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

#ifndef SAIL_PLUGIN_INFO_NODE_H
#define SAIL_PLUGIN_INFO_NODE_H

struct sail_plugin_info;
struct sail_plugin;

/*
 * A structure representing a plugin information linked list.
 */
struct sail_plugin_info_node {

    /* Plugin information. */
    struct sail_plugin_info *plugin_info;

    /* Plugin instance. */
    struct sail_plugin *plugin;

    struct sail_plugin_info_node *next;
};

typedef struct sail_plugin_info_node sail_plugin_info_node_t;

#endif
