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

#include "plugin.h"

int sail_alloc_plugin(const char *path, struct sail_plugin **plugin) {

    if (path == NULL) {
        return SAIL_INVALID_ARGUMENT;
    }

    *plugin = (struct sail_plugin *)malloc(sizeof(struct sail_plugin));

    if (*plugin == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin)->handle = NULL;

#ifdef SAIL_WIN32
#else
    void *handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", path, dlerror());
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

    free(plugin);
}
