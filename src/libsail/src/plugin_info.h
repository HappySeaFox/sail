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

#ifndef SAIL_PLUGIN_INFO_H
#define SAIL_PLUGIN_INFO_H

struct sail_string_node;

struct sail_plugin;

/*
 * A structure representing plugin information.
 */
struct sail_plugin_info {

    /*
     * The plugin loader will use the plugin's layout version to correctly handle the plugin. Unsupported
     * plugin layout versions will be reported. This field must be the very first key in a plugin information
     * file.
     *
     * Plugin layout is a list of exported functions. We use plugin layout versions to implement
     * backward compatibility in a simple and maintanable way.
     */
    int layout;

    /* Plugin version. For example: "1.5.2". */
    char *version;

    /* Short plugin name in upper case. For example: "JPEG". */
    char *name;

    /* Plugin description. For example: "Joint Photographic Experts Group". */
    char *description;

    /* A linked list of supported file extensions. For example: "jpg", "jpeg". */
    struct sail_string_node *extension_node;

    /* A linked list of supported mime types. For example: "image/jpeg". */
    struct sail_string_node *mime_type_node;

    /* Full path to the plugin. */
    char *path;
};

typedef struct sail_plugin_info sail_plugin_info_t;

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
