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

int sail_alloc_plugin(const struct sail_plugin_info *plugin_info, struct sail_plugin **plugin) {

    if (plugin_info == NULL || plugin_info->path == NULL) {
        return SAIL_INVALID_ARGUMENT;
    }

    *plugin = (struct sail_plugin *)malloc(sizeof(struct sail_plugin));

    if (*plugin == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin)->layout        = plugin_info->layout;
    (*plugin)->handle        = NULL;
    (*plugin)->inter_face.v1 = NULL;

#ifdef SAIL_WIN32
#else
    SAIL_LOG_DEBUG("Loading plugin '%s'", plugin_info->path);

    void *handle = dlopen(plugin_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", plugin_info->path, dlerror());
        sail_destroy_plugin(*plugin);
        return SAIL_PLUGIN_LOAD_ERROR;
    }

    (*plugin)->handle = handle;

    if ((*plugin)->layout == 1) {
        (*plugin)->inter_face.v1 = (struct sail_plugin_layout_v1 *)malloc(sizeof(struct sail_plugin_layout_v1));

        (*plugin)->inter_face.v1->read_features_v1        = (sail_plugin_read_features_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_features_v1");
        (*plugin)->inter_face.v1->read_init_v1            = (sail_plugin_read_init_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_init_v1");
        (*plugin)->inter_face.v1->read_seek_next_frame_v1 = (sail_plugin_read_seek_next_frame_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_seek_next_frame_v1");
        (*plugin)->inter_face.v1->read_seek_next_pass_v1  = (sail_plugin_read_seek_next_pass_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_seek_next_pass_v1");
        (*plugin)->inter_face.v1->read_scan_line_v1       = (sail_plugin_read_scan_line_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_scan_line_v1");
        (*plugin)->inter_face.v1->read_finish_v1          = (sail_plugin_read_finish_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_finish_v1");

        (*plugin)->inter_face.v1->write_features_v1        = (sail_plugin_write_features_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_features_v1");
        (*plugin)->inter_face.v1->write_init_v1            = (sail_plugin_write_init_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_init_v1");
        (*plugin)->inter_face.v1->write_seek_next_frame_v1 = (sail_plugin_write_seek_next_frame_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_seek_next_frame_v1");
        (*plugin)->inter_face.v1->write_seek_next_pass_v1  = (sail_plugin_write_seek_next_pass_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_seek_next_pass_v1");
        (*plugin)->inter_face.v1->write_scan_line_v1       = (sail_plugin_write_scan_line_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_scan_line_v1");
        (*plugin)->inter_face.v1->write_finish_v1          = (sail_plugin_write_finish_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_finish_v1");
    } else if ((*plugin)->layout == 2) {
        (*plugin)->inter_face.v2 = (struct sail_plugin_layout_v2 *)malloc(sizeof(struct sail_plugin_layout_v2));

        (*plugin)->inter_face.v2->read_features_v1        = (sail_plugin_read_features_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_features_v1");
        (*plugin)->inter_face.v2->read_init_v1            = (sail_plugin_read_init_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_init_v1");
        (*plugin)->inter_face.v2->read_seek_next_frame_v1 = (sail_plugin_read_seek_next_frame_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_seek_next_frame_v1");
        (*plugin)->inter_face.v2->read_seek_next_pass_v1  = (sail_plugin_read_seek_next_pass_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_seek_next_pass_v1");
        (*plugin)->inter_face.v2->read_scan_line_v1       = (sail_plugin_read_scan_line_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_scan_line_v1");
        (*plugin)->inter_face.v2->read_finish_v1          = (sail_plugin_read_finish_v1_t)dlsym((*plugin)->handle, "sail_plugin_read_finish_v1");

        (*plugin)->inter_face.v2->read_scan_line_v2       = (sail_plugin_read_scan_line_v2_t)dlsym((*plugin)->handle, "sail_plugin_read_scan_line_v2");

        (*plugin)->inter_face.v2->write_features_v1        = (sail_plugin_write_features_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_features_v1");
        (*plugin)->inter_face.v2->write_init_v1            = (sail_plugin_write_init_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_init_v1");
        (*plugin)->inter_face.v2->write_seek_next_frame_v1 = (sail_plugin_write_seek_next_frame_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_seek_next_frame_v1");
        (*plugin)->inter_face.v2->write_seek_next_pass_v1  = (sail_plugin_write_seek_next_pass_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_seek_next_pass_v1");
        (*plugin)->inter_face.v2->write_scan_line_v1       = (sail_plugin_write_scan_line_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_scan_line_v1");
        (*plugin)->inter_face.v2->write_finish_v1          = (sail_plugin_write_finish_v1_t)dlsym((*plugin)->handle, "sail_plugin_write_finish_v1");
    } else {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }
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
        case 1: free(plugin->inter_face.v1); break;
        case 2: free(plugin->inter_face.v2); break;

        default:
            SAIL_LOG_WARNING("Don't know how to destroy plugin interface version %d", plugin->layout);
    }

    free(plugin);
}
