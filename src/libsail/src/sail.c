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

#define _POSIX_C_SOURCE 200112L /* setenv */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static const char* plugins_path(void) {

    SAIL_THREAD_LOCAL static bool plugins_path_called = false;
    SAIL_THREAD_LOCAL static char *env = NULL;

    if (plugins_path_called) {
        return env;
    }

    plugins_path_called = true;

#ifdef SAIL_WIN32
    _dupenv_s(&env, NULL, "SAIL_PLUGINS_PATH");
#else
    env = getenv("SAIL_PLUGINS_PATH");
#endif

    if (env == NULL) {
        SAIL_LOG_DEBUG("SAIL_PLUGINS_PATH environment variable is not set. Loading plugins from %s", SAIL_PLUGINS_PATH);
        env = SAIL_PLUGINS_PATH;
    } else {
        SAIL_LOG_DEBUG("SAIL_PLUGINS_PATH environment variable is set. Loading plugins from %s", env);
    }

    return env;
}

/* Add "sail/plugins/lib" to the DLL/SO search path. */
static sail_error_t update_lib_path(void) {

    SAIL_THREAD_LOCAL static bool update_lib_path_called = false;

    if (update_lib_path_called) {
        return 0;
    }

    update_lib_path_called = true;

    /* Build a full path to the SAIL plugins path + "/lib". */
    const char *plugs_path = plugins_path();

#ifdef SAIL_WIN32
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, plugs_path, "\\lib"));

    SAIL_LOG_DEBUG("Set DLL directory to '%s'", full_path_to_lib);

    wchar_t *full_path_to_lib_w;
    SAIL_TRY_OR_CLEANUP(sail_to_wchar(full_path_to_lib, &full_path_to_lib_w),
                        free(full_path_to_lib));

    if (!AddDllDirectory(full_path_to_lib_w)) {
        SAIL_LOG_ERROR("Failed to update library search path. Error: %d", GetLastError());
        free(full_path_to_lib_w);
        free(full_path_to_lib);
        return SAIL_ENV_UPDATE_FAILED;
    }

    free(full_path_to_lib_w);
    free(full_path_to_lib);
#else
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, plugs_path, "/lib"));

    char *combined_ld_library_path;
    char *env = getenv("LD_LIBRARY_PATH");

    if (env == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_strdup(full_path_to_lib, &combined_ld_library_path),
                            free(full_path_to_lib));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_concat(&combined_ld_library_path, 3, env, ":", full_path_to_lib),
                            free(full_path_to_lib));
    }

    free(full_path_to_lib);
    SAIL_LOG_DEBUG("Set LD_LIBRARY_PATH to '%s'", combined_ld_library_path);

    if (setenv("LD_LIBRARY_PATH", combined_ld_library_path, true) != 0) {
        SAIL_LOG_ERROR("Failed to update library search path: %s", strerror(errno));
        free(combined_ld_library_path);
        return SAIL_ENV_UPDATE_FAILED;
    }

    free(combined_ld_library_path);
#endif

    return 0;
}

static sail_error_t build_full_path(const char *sail_plugins_path, const char *name, char **full_path) {

#ifdef SAIL_WIN32
    SAIL_TRY(sail_concat(full_path, 3, sail_plugins_path, "\\", name));
#else
    SAIL_TRY(sail_concat(full_path, 3, sail_plugins_path, "/", name));
#endif

    return 0;
}

static sail_error_t build_plugin_full_path(struct sail_context *context,
                                            struct sail_plugin_info_node **last_plugin_info_node,
                                            const char *plugin_info_full_path) {

    /* Build "/path/jpeg.so" from "/path/jpeg.plugin.info". */
    char *plugin_info_part = strstr(plugin_info_full_path, ".plugin.info");

    if (plugin_info_part == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    /* The length of "/path/jpeg". */
    size_t plugin_full_path_length = strlen(plugin_info_full_path) - strlen(plugin_info_part);
    char *plugin_full_path;

#ifdef SAIL_WIN32
    static const char * const LIB_SUFFIX = "dll";
#else
    static const char * const LIB_SUFFIX = "so";
#endif

    /* The resulting string will be "/path/jpeg.plu" (on Windows) or "/path/jpeg.pl". */
    SAIL_TRY(sail_strdup_length(plugin_info_full_path,
                                plugin_full_path_length + strlen(LIB_SUFFIX) + 1, &plugin_full_path));

#ifdef SAIL_WIN32
    /* Overwrite the end of the path with "dll". */
    strcpy_s(plugin_full_path + plugin_full_path_length + 1, strlen(LIB_SUFFIX) + 1, LIB_SUFFIX);
#else
    /* Overwrite the end of the path with "so". */
    strcpy(plugin_full_path + plugin_full_path_length + 1, LIB_SUFFIX);
#endif

    /* Parse plugin info. */
    struct sail_plugin_info_node *plugin_info_node;
    SAIL_TRY_OR_CLEANUP(sail_alloc_plugin_info_node(&plugin_info_node),
                        free(plugin_full_path));

    struct sail_plugin_info *plugin_info;
    SAIL_TRY_OR_CLEANUP(sail_plugin_read_info(plugin_info_full_path, &plugin_info),
                        sail_destroy_plugin_info_node(plugin_info_node),
                        free(plugin_full_path));

    /* Save the parsed plugin info into the SAIL context. */
    plugin_info_node->plugin_info = plugin_info;
    plugin_info->path = plugin_full_path;

    *last_plugin_info_node = NULL;

    if (context->plugin_info_node == NULL) {
        context->plugin_info_node = *last_plugin_info_node = plugin_info_node;
    } else {
        (*last_plugin_info_node)->next = plugin_info_node;
        *last_plugin_info_node = plugin_info_node;
    }

    return 0;
}

/*
 * Public functions.
 */

sail_error_t sail_init(struct sail_context **context) {

    /* Time counter. */
    uint64_t start_time;
    SAIL_TRY(sail_now(&start_time));

    SAIL_LOG_INFO("Version %s-%s", SAIL_VERSION_STRING, SAIL_GIT_HASH);

    SAIL_CHECK_CONTEXT_PTR(context);

    *context = (struct sail_context *)malloc(sizeof(struct sail_context));

    if (*context == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*context)->plugin_info_node = NULL;

    SAIL_TRY(update_lib_path());

    struct sail_plugin_info_node *last_plugin_info_node;

#ifdef SAIL_WIN32
    const char *plugs_path = plugins_path();
    const char *plugs_info_mask = "\\*.plugin.info";

    size_t plugs_path_with_mask_length = strlen(plugs_path) + strlen(plugs_info_mask) + 1;
    char *plugs_path_with_mask = (char *)malloc(plugs_path_with_mask_length);

    if (plugs_path_with_mask == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    strcpy_s(plugs_path_with_mask, plugs_path_with_mask_length, plugs_path);
    strcat_s(plugs_path_with_mask, plugs_path_with_mask_length, plugs_info_mask);

    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(plugs_path_with_mask, &data);

    if (hFind == INVALID_HANDLE_VALUE) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d", plugs_path, GetLastError());
        free(plugs_path_with_mask);
        return SAIL_DIR_OPEN_ERROR;
    }

    do {
        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        if (build_full_path(plugs_path, data.cFileName, &full_path) != 0) {
            continue;
        }

        build_plugin_full_path(*context, &last_plugin_info_node, full_path);

        free(full_path);
    } while (FindNextFile(hFind, &data));

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d. Some plugins may be ignored", plugs_path, GetLastError());
    }

    free(plugs_path_with_mask);
    FindClose(hFind);
#else
    const char *plugs_path = plugins_path();
    DIR *d = opendir(plugs_path);

    if (d == NULL) {
        SAIL_LOG_ERROR("Failed to list files in '%s': %s", plugs_path, strerror(errno));
        return SAIL_DIR_OPEN_ERROR;
    }

    struct dirent *dir;

    while ((dir = readdir(d)) != NULL) {

        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        if (build_full_path(plugs_path, dir->d_name, &full_path) != 0) {
            continue;
        }

        /* Handle files only. */
        struct stat full_path_stat;
        stat(full_path, &full_path_stat);
        bool is_file = S_ISREG(full_path_stat.st_mode);

        if (is_file) {
            build_plugin_full_path(*context, &last_plugin_info_node, full_path);
        }

        free(full_path);
    }

    closedir(d);
#endif

    uint64_t end_time;
    SAIL_TRY(sail_now(&end_time));

    SAIL_LOG_DEBUG("Initialized in %lld ms.", (unsigned long)(end_time - start_time));

    return 0;
}

void sail_finish(struct sail_context *context) {

    SAIL_LOG_INFO("Finish");

    if (context == NULL) {
        return;
    }

    sail_destroy_plugin_info_node_chain(context->plugin_info_node);

    free(context);
}

const struct sail_plugin_info_node* sail_plugin_info_list(const struct sail_context *context) {

    return context->plugin_info_node;
}

sail_error_t sail_plugin_info_from_path(const char *path, const struct sail_context *context, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    const char *dot = strrchr(path, '.');

    if (dot == NULL || *(dot+1) == '\0') {
        return SAIL_INVALID_ARGUMENT;
    }

    SAIL_TRY(sail_plugin_info_from_extension(dot+1, context, plugin_info));

    return 0;
}

sail_error_t sail_plugin_info_from_extension(const char *extension, const struct sail_context *context, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_EXTENSION_PTR(extension);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    char *extension_copy;
    SAIL_TRY(sail_strdup(extension, &extension_copy));

    /* Will compare in lower case. */
    sail_to_lower(extension_copy);

    struct sail_plugin_info_node *node = context->plugin_info_node;

    while (node != NULL) {
        struct sail_string_node *string_node = node->plugin_info->extension_node;

        while (string_node != NULL) {
            if (strcmp(string_node->value, extension_copy) == 0) {
                free(extension_copy);
                *plugin_info = node->plugin_info;
                return 0;
            }

            string_node = string_node->next;
        }

        node = node->next;
    }

    free(extension_copy);
    return SAIL_PLUGIN_NOT_FOUND;
}

sail_error_t sail_plugin_info_from_mime_type(const struct sail_context *context, const char *mime_type, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PTR(mime_type);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    char *mime_type_copy;
    SAIL_TRY(sail_strdup(mime_type, &mime_type_copy));

    /* Will compare in lower case. */
    sail_to_lower(mime_type_copy);

    struct sail_plugin_info_node *node = context->plugin_info_node;

    while (node != NULL) {
        struct sail_string_node *string_node = node->plugin_info->mime_type_node;

        while (string_node != NULL) {
            if (strcmp(string_node->value, mime_type_copy) == 0) {
                free(mime_type_copy);
                *plugin_info = node->plugin_info;
                return 0;
            }

            string_node = string_node->next;
        }

        node = node->next;
    }

    free(mime_type_copy);
    return SAIL_PLUGIN_NOT_FOUND;
}

sail_error_t sail_unload_plugins(struct sail_context *context) {

    SAIL_CHECK_CONTEXT_PTR(context);

    SAIL_LOG_DEBUG("Unloading cached plugins");

    struct sail_plugin_info_node *node = context->plugin_info_node;
    int counter = 0;

    while (node != NULL) {
        if (node->plugin != NULL) {
            sail_destroy_plugin(node->plugin);
            counter++;
        }

        node->plugin = NULL;

        node = node->next;
    }

    SAIL_LOG_DEBUG("Unloaded plugins: %d", counter);

    return 0;
}

SAIL_EXPORT sail_error_t sail_start_reading_io_with_options(struct sail_io *io, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                            const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_CHECK_STATE_PTR(state_of_mind);

    state_of_mind->io          = io;
    state_of_mind->state       = NULL;
    state_of_mind->plugin_info = plugin_info;
    state_of_mind->plugin      = NULL;

    *state = state_of_mind;

    SAIL_TRY(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin));

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    if (read_options == NULL) {
        struct sail_read_options *read_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(state_of_mind->plugin_info->read_features, &read_options_local),
                            /* cleanup */ sail_destroy_read_options(read_options_local));
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_read_options(read_options_local));
        sail_destroy_read_options(read_options_local);
    } else {
        SAIL_TRY(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options, &state_of_mind->state));
    }

    return 0;
}

sail_error_t sail_start_reading_file_with_options(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                  const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_file(path, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options(io, context, plugin_info_local, read_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_reading_io(struct sail_io *io, struct sail_context *context,
                                   const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_writing_io_with_options(struct sail_io *io, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                  const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_CHECK_STATE_PTR(state_of_mind);

    state_of_mind->io          = io;
    state_of_mind->state       = NULL;
    state_of_mind->plugin_info = plugin_info;
    state_of_mind->plugin      = NULL;

    *state = state_of_mind;

    SAIL_TRY(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin));

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    if (write_options == NULL) {
        struct sail_write_options *write_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_write_options_from_features(state_of_mind->plugin_info->write_features, &write_options_local),
                            /* cleanup */ sail_destroy_write_options(write_options_local));
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->write_init_v2(state_of_mind->io, write_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_write_options(write_options_local));
        sail_destroy_write_options(write_options_local);
    } else {
        SAIL_TRY(state_of_mind->plugin->v2->write_init_v2(state_of_mind->io, write_options, &state_of_mind->state));
    }

    return 0;
}

sail_error_t sail_start_writing_file_with_options(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                  const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_write_file(path, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_writing_io_with_options(io, context, plugin_info_local, write_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_writing_io(struct sail_io *io, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_writing_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}
