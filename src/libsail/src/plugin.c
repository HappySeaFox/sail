#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
#else
    #include <dlfcn.h>
#endif

/* libsail-common */
#include "error.h"
#include "log.h"

#include "plugin_info.h"
#include "plugin.h"

int sail_alloc_plugin(struct sail_plugin_info *plugin_info, struct sail_plugin **plugin) {

    if (plugin_info == NULL || plugin_info->path == NULL) {
        return SAIL_INVALID_ARGUMENT;
    }

    *plugin = (struct sail_plugin *)malloc(sizeof(struct sail_plugin));

    if (*plugin == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin)->layout       = plugin_info->layout;
    (*plugin)->handle       = NULL;
    (*plugin)->interface.v1 = NULL;

#ifdef SAIL_WIN32
#else
    void *handle = dlopen(plugin_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", plugin_info->path, dlerror());
        sail_destroy_plugin(*plugin);
        return SAIL_PLUGIN_LOAD_ERROR;
    }

    (*plugin)->handle = handle;
#endif

    return 0;
}

void sail_destroy_plugin(struct sail_plugin *plugin) {

    if (plugin == NULL) {
        return;
    }

    if (plugin->handle != NULL) {
#ifdef SAIL_WIN32
#else
        dlclose(plugin->handle);
#endif
    }

    switch (plugin->layout) {
        case 1: free(plugin->interface.v1); break;
        case 2: free(plugin->interface.v2); break;

        default:
            SAIL_LOG_WARNING("Don't know how to destroy plugin interface version %d", plugin->layout);
    }

    free(plugin);
}
