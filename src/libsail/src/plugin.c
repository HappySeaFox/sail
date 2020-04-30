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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include "sail-common.h"
#include "sail.h"

sail_error_t alloc_plugin(const struct sail_plugin_info *plugin_info, struct sail_plugin **plugin) {

    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);
    SAIL_CHECK_PATH_PTR(plugin_info->path);

    *plugin = (struct sail_plugin *)malloc(sizeof(struct sail_plugin));

    if (*plugin == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin)->layout = plugin_info->layout;
    (*plugin)->handle = NULL;
    (*plugin)->v2     = NULL;

    SAIL_LOG_DEBUG("Loading plugin '%s'", plugin_info->path);

#ifdef SAIL_WIN32
    HMODULE handle = LoadLibraryEx(plugin_info->path, NULL, LOAD_LIBRARY_SEARCH_USER_DIRS);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s'. Error: %d", plugin_info->path, GetLastError());
        destroy_plugin(*plugin);
        return SAIL_PLUGIN_LOAD_ERROR;
    }

    (*plugin)->handle = handle;
#else
    void *handle = dlopen(plugin_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", plugin_info->path, dlerror());
        destroy_plugin(*plugin);
        return SAIL_PLUGIN_LOAD_ERROR;
    }

    (*plugin)->handle = handle;
#endif

#ifdef SAIL_WIN32
    #define SAIL_RESOLVE_FUNC GetProcAddress
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s'. Error: %d", #symbol, plugin_info->path, GetLastError())
#else
    #define SAIL_RESOLVE_FUNC dlsym
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s': %s", #symbol, plugin_info->path, dlerror())
#endif

    #define SAIL_RESOLVE(target, handle, symbol)                 \
    do {                                                         \
        target = (symbol##_t)SAIL_RESOLVE_FUNC(handle, #symbol); \
                                                                 \
        if (target == NULL) {                                    \
            SAIL_RESOLVE_LOG_ERROR(symbol);                      \
            destroy_plugin(*plugin);                             \
            return SAIL_PLUGIN_SYMBOL_RESOLVE_FAILED;            \
        }                                                        \
    }                                                            \
    while(0)

    if ((*plugin)->layout == SAIL_PLUGIN_LAYOUT_V2) {
        (*plugin)->v2 = (struct sail_plugin_layout_v2 *)malloc(sizeof(struct sail_plugin_layout_v2));

        SAIL_RESOLVE((*plugin)->v2->read_init_v2,            handle, sail_plugin_read_init_v2);
        SAIL_RESOLVE((*plugin)->v2->read_seek_next_frame_v2, handle, sail_plugin_read_seek_next_frame_v2);
        SAIL_RESOLVE((*plugin)->v2->read_seek_next_pass_v2,  handle, sail_plugin_read_seek_next_pass_v2);
        SAIL_RESOLVE((*plugin)->v2->read_scan_line_v2,       handle, sail_plugin_read_scan_line_v2);
        SAIL_RESOLVE((*plugin)->v2->read_alloc_scan_line_v2, handle, sail_plugin_read_alloc_scan_line_v2);
        SAIL_RESOLVE((*plugin)->v2->read_finish_v2,          handle, sail_plugin_read_finish_v2);

        SAIL_RESOLVE((*plugin)->v2->write_init_v2,            handle, sail_plugin_write_init_v2);
        SAIL_RESOLVE((*plugin)->v2->write_seek_next_frame_v2, handle, sail_plugin_write_seek_next_frame_v2);
        SAIL_RESOLVE((*plugin)->v2->write_seek_next_pass_v2,  handle, sail_plugin_write_seek_next_pass_v2);
        SAIL_RESOLVE((*plugin)->v2->write_scan_line_v2,       handle, sail_plugin_write_scan_line_v2);
        SAIL_RESOLVE((*plugin)->v2->write_finish_v2,          handle, sail_plugin_write_finish_v2);
    } else {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    return 0;
}

void destroy_plugin(struct sail_plugin *plugin) {

    if (plugin == NULL) {
        return;
    }

    if (plugin->handle != NULL) {
#ifdef SAIL_WIN32
        FreeLibrary((HMODULE)plugin->handle);
#else
        dlclose(plugin->handle);
#endif
    }

    if (plugin->layout == SAIL_PLUGIN_LAYOUT_V2) {
        free(plugin->v2);
    } else {
        SAIL_LOG_WARNING("Don't know how to destroy plugin interface version %d", plugin->layout);
    }

    free(plugin);
}
