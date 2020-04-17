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

#ifndef SAIL_PLUGIN_INFO_PRIVATE_H
#define SAIL_PLUGIN_INFO_PRIVATE_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_plugin_info_node;
struct sail_plugin_info;

/*
 * Private plugin info functions.
 */

/*
 * Allocates a new plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_plugin_info(struct sail_plugin_info **plugin_info);

/*
 * Destroys the specified plugin_info and all its internal allocated memory buffers.
 * The "plugin_info" pointer MUST NOT be used after calling this function.
 */
SAIL_EXPORT void sail_destroy_plugin_info(struct sail_plugin_info *plugin_info);

/*
 * Allocates a new plugin info node. The assigned node MUST be destroyed later with sail_destroy_plugin_info_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node);

/*
 * Destroys the specified plugin info node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node);

/*
 * Destroys the specified plugin info node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_plugin_info_node_chain(struct sail_plugin_info_node *plugin_info_node);

/*
 * Reads SAIL plugin info from the specified file and stores the parsed information into
 * the specified plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_info(const char *path, struct sail_plugin_info **plugin_info);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
