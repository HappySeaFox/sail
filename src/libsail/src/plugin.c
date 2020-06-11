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
