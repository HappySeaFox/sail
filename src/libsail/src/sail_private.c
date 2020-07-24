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
#endif

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static const char* plugins_path(void) {

    SAIL_THREAD_LOCAL static bool plugins_path_called = false;
    SAIL_THREAD_LOCAL static const char *env = NULL;

    if (plugins_path_called) {
        return env;
    }

    plugins_path_called = true;

#ifdef SAIL_WIN32
    _dupenv_s((char **)&env, NULL, "SAIL_PLUGINS_PATH");

    /* Construct "\bin\..\lib\sail\plugins" from "\bin\sail.dll". */
    if (env == NULL) {
        char path[MAX_PATH];
        HMODULE thisModule;

        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCSTR)&plugins_path, &thisModule) == 0) {
            SAIL_LOG_ERROR("GetModuleHandleEx() failed with an error code %d. Falling back to loading plugins from '%s'",
                            GetLastError(), SAIL_PLUGINS_PATH);
            env = SAIL_PLUGINS_PATH;
        } else if (GetModuleFileName(thisModule, path, sizeof(path)) == 0) {
            SAIL_LOG_ERROR("GetModuleFileName() failed with an error code %d. Falling back to loading plugins from '%s'",
                            GetLastError(), SAIL_PLUGINS_PATH);
            env = SAIL_PLUGINS_PATH;
        } else {
            char *lib_sail_plugins_path;

            /* "\bin\sail.dll" -> "\bin". */
            char *last_sep = strrchr(path, '\\');

            if (last_sep == NULL) {
                SAIL_LOG_ERROR("Failed to find a path separator in '%s'. Falling back to loading plugins from '%s'",
                                path, SAIL_PLUGINS_PATH);
                env = SAIL_PLUGINS_PATH;
            } else {
                *last_sep = '\0';

                /* "\bin" -> "\bin\..\lib\sail\plugins". */
                SAIL_TRY_OR_EXECUTE(sail_concat(&lib_sail_plugins_path, 2, path, "\\..\\lib\\sail\\plugins"),
                                    /* on error */ SAIL_LOG_ERROR("Failed to concat strings. Falling back to loading plugins from '%s'",
                                                                    SAIL_PLUGINS_PATH),
                                    env = SAIL_PLUGINS_PATH);
                env = lib_sail_plugins_path;
                SAIL_LOG_DEBUG("Optional SAIL_PLUGINS_PATH environment variable is not set");
            }
        }
    } else {
        SAIL_LOG_DEBUG("SAIL_PLUGINS_PATH environment variable is set. Loading plugins from '%s'", env);
    }
#else
    env = getenv("SAIL_PLUGINS_PATH");

    if (env == NULL) {
        SAIL_LOG_DEBUG("SAIL_PLUGINS_PATH environment variable is not set. Loading plugins from '%s'", SAIL_PLUGINS_PATH);
        env = SAIL_PLUGINS_PATH;
    } else {
        SAIL_LOG_DEBUG("SAIL_PLUGINS_PATH environment variable is set. Loading plugins from '%s'", env);
    }
#endif

    return env;
}

/* Add "sail/plugins/lib" to the DLL/SO search path. */
static sail_status_t update_lib_path(void) {

    SAIL_THREAD_LOCAL static bool update_lib_path_called = false;

    if (update_lib_path_called) {
        return SAIL_OK;
    }

    update_lib_path_called = true;

    /* Build a full path to the SAIL plugins path + "/lib". */
    const char *plugs_path = plugins_path();

#ifdef SAIL_WIN32
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, plugs_path, "\\lib"));

    if (!sail_is_dir(full_path_to_lib)) {
        SAIL_LOG_DEBUG("Optional DLL directory '%s' doesn't exist, so not loading DLLs from it", full_path_to_lib);
        sail_free(full_path_to_lib);
        return SAIL_OK;
    }

    SAIL_LOG_DEBUG("Set DLL directory to '%s'", full_path_to_lib);

    wchar_t *full_path_to_lib_w;
    SAIL_TRY_OR_CLEANUP(sail_to_wchar(full_path_to_lib, &full_path_to_lib_w),
                        sail_free(full_path_to_lib));

    if (!AddDllDirectory(full_path_to_lib_w)) {
        SAIL_LOG_ERROR("Failed to update library search path with '%s'. Error: %d", full_path_to_lib, GetLastError());
        sail_free(full_path_to_lib_w);
        sail_free(full_path_to_lib);
        return SAIL_ERROR_ENV_UPDATE;
    }

    sail_free(full_path_to_lib_w);
    sail_free(full_path_to_lib);
#else
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, plugs_path, "/lib"));

    if (!sail_is_dir(full_path_to_lib)) {
        SAIL_LOG_DEBUG("Optional LIB directory '%s' doesn't exist, so not updating LD_LIBRARY_PATH with it", full_path_to_lib);
        sail_free(full_path_to_lib);
        return SAIL_OK;
    }

    char *combined_ld_library_path;
    char *env = getenv("LD_LIBRARY_PATH");

    if (env == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_strdup(full_path_to_lib, &combined_ld_library_path),
                            sail_free(full_path_to_lib));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_concat(&combined_ld_library_path, 3, env, ":", full_path_to_lib),
                            sail_free(full_path_to_lib));
    }

    sail_free(full_path_to_lib);
    SAIL_LOG_DEBUG("Set LD_LIBRARY_PATH to '%s'", combined_ld_library_path);

    if (setenv("LD_LIBRARY_PATH", combined_ld_library_path, true) != 0) {
        SAIL_LOG_ERROR("Failed to update library search path: %s", strerror(errno));
        sail_free(combined_ld_library_path);
        return SAIL_ERROR_ENV_UPDATE;
    }

    sail_free(combined_ld_library_path);
#endif

    return SAIL_OK;
}

static sail_status_t build_full_path(const char *sail_plugins_path, const char *name, char **full_path) {

#ifdef SAIL_WIN32
    SAIL_TRY(sail_concat(full_path, 3, sail_plugins_path, "\\", name));
#else
    SAIL_TRY(sail_concat(full_path, 3, sail_plugins_path, "/", name));
#endif

    return SAIL_OK;
}

static sail_status_t build_plugin_from_plugin_info(const char *plugin_info_full_path,
                                                    struct sail_plugin_info_node **plugin_info_node) {

    SAIL_CHECK_PATH_PTR(plugin_info_full_path);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info_node);

    /* Build "/path/jpeg.so" from "/path/jpeg.plugin.info". */
    char *plugin_info_part = strstr(plugin_info_full_path, ".plugin.info");

    if (plugin_info_part == NULL) {
        return SAIL_ERROR_MEMORY_ALLOCATION;
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
    SAIL_TRY_OR_CLEANUP(alloc_plugin_info_node(plugin_info_node),
                        sail_free(plugin_full_path));

    struct sail_plugin_info *plugin_info;
    SAIL_TRY_OR_CLEANUP(plugin_read_info(plugin_info_full_path, &plugin_info),
                        destroy_plugin_info_node(*plugin_info_node),
                        sail_free(plugin_full_path));

    /* Save the parsed plugin info into the SAIL context. */
    (*plugin_info_node)->plugin_info = plugin_info;
    plugin_info->path = plugin_full_path;

    return SAIL_OK;
}

/* Initializes the context and loads all the plugin info files if the context is not initialized. */
static sail_status_t init_context(struct sail_context *context, int flags) {

    SAIL_CHECK_CONTEXT_PTR(context);

    if (context->initialized) {
        return SAIL_OK;
    }

    context->initialized = true;

    /* Time counter. */
    uint64_t start_time = sail_now();

    SAIL_LOG_INFO("Version %s", SAIL_VERSION_STRING);

    SAIL_TRY(update_lib_path());

    const char *plugs_path = plugins_path();

    SAIL_LOG_DEBUG("Loading plugins from '%s'", plugs_path);

    /* Used to load and store plugin info objects. */
    struct sail_plugin_info_node **last_plugin_info_node = &context->plugin_info_node;
    struct sail_plugin_info_node *plugin_info_node;

#ifdef SAIL_WIN32
    const char *plugs_info_mask = "\\*.plugin.info";

    size_t plugs_path_with_mask_length = strlen(plugs_path) + strlen(plugs_info_mask) + 1;

    char *plugs_path_with_mask;
    SAIL_TRY(sail_malloc(&plugs_path_with_mask, plugs_path_with_mask_length));

    strcpy_s(plugs_path_with_mask, plugs_path_with_mask_length, plugs_path);
    strcat_s(plugs_path_with_mask, plugs_path_with_mask_length, plugs_info_mask);

    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(plugs_path_with_mask, &data);

    if (hFind == INVALID_HANDLE_VALUE) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d", plugs_path, GetLastError());
        sail_free(plugs_path_with_mask);
        return SAIL_ERROR_LIST_DIR;
    }

    do {
        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        SAIL_TRY_OR_EXECUTE(build_full_path(plugs_path, data.cFileName, &full_path),
                            /* on error */ continue);

        SAIL_LOG_DEBUG("Found plugin info '%s'", data.cFileName);

        if (build_plugin_from_plugin_info(full_path, &plugin_info_node) == SAIL_OK) {
            *last_plugin_info_node = plugin_info_node;
            last_plugin_info_node = &plugin_info_node->next;
        }

        sail_free(full_path);
    } while (FindNextFile(hFind, &data));

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d. Some plugins may be ignored", plugs_path, GetLastError());
    }

    sail_free(plugs_path_with_mask);
    FindClose(hFind);
#else
    DIR *d = opendir(plugs_path);

    if (d == NULL) {
        SAIL_LOG_ERROR("Failed to list files in '%s': %s", plugs_path, strerror(errno));
        return SAIL_ERROR_LIST_DIR;
    }

    struct dirent *dir;

    while ((dir = readdir(d)) != NULL) {
        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        SAIL_TRY_OR_EXECUTE(build_full_path(plugs_path, dir->d_name, &full_path),
                            /* on error */ continue);

        /* Handle files only. */
        if (sail_is_file(full_path)) {
            bool is_plugin_info = strstr(full_path, ".plugin.info") != NULL;

            if (is_plugin_info) {
                SAIL_LOG_DEBUG("Found plugin info '%s'", dir->d_name);

                if (build_plugin_from_plugin_info(full_path, &plugin_info_node) == SAIL_OK) {
                    *last_plugin_info_node = plugin_info_node;
                    last_plugin_info_node = &plugin_info_node->next;
                }
            }
        }

        sail_free(full_path);
    }

    closedir(d);
#endif

    if (flags & SAIL_FLAG_PRELOAD_PLUGINS) {
        SAIL_LOG_DEBUG("Preloading plugins");

        plugin_info_node = context->plugin_info_node;

        while (plugin_info_node != NULL) {
            const struct sail_plugin *plugin;

            /* Ignore loading errors on purpose. */
            load_plugin_by_plugin_info(plugin_info_node->plugin_info, &plugin);

            plugin_info_node = plugin_info_node->next;
        }
    }

    SAIL_LOG_DEBUG("Enumerated plugins:");

    /* Print the found plugin infos. */
    struct sail_plugin_info_node *node = context->plugin_info_node;
    int counter = 1;

    while (node != NULL) {
        SAIL_LOG_DEBUG("%d. %s [%s] %s", counter++, node->plugin_info->name, node->plugin_info->description, node->plugin_info->version);
        node = node->next;
    }

    SAIL_LOG_DEBUG("Initialized in %lld ms.", (unsigned long)(sail_now() - start_time));

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t alloc_context(struct sail_context **context) {

    SAIL_CHECK_CONTEXT_PTR(context);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_context)));

    *context = ptr;

    (*context)->initialized      = false;
    (*context)->plugin_info_node = NULL;

    return SAIL_OK;
}

sail_status_t destroy_context(struct sail_context *context) {

    if (context == NULL) {
        return SAIL_OK;
    }

    destroy_plugin_info_node_chain(context->plugin_info_node);
    sail_free(context);

    return SAIL_OK;
}

sail_status_t control_tls_context(struct sail_context **context, enum SailContextAction action) {

    SAIL_THREAD_LOCAL static struct sail_context *tls_context = NULL;

    switch (action) {
        case SAIL_CONTEXT_ALLOCATE: {
            SAIL_CHECK_CONTEXT_PTR(context);

            if (tls_context == NULL) {
                SAIL_TRY(alloc_context(&tls_context));
                SAIL_LOG_DEBUG("Allocated a new thread-local context %p", tls_context);
            }

            *context = tls_context;
            break;
        }
        case SAIL_CONTEXT_FETCH: {
            *context = tls_context;
            break;
        }
        case SAIL_CONTEXT_DESTROY: {
            destroy_context(tls_context);
            SAIL_LOG_DEBUG("Destroyed the thread-local context %p", tls_context);
            tls_context = NULL;
            break;
        }
    }

    return SAIL_OK;
}

sail_status_t current_tls_context(struct sail_context **context) {

    SAIL_TRY(current_tls_context_with_flags(context, /* flags */ 0));

    return SAIL_OK;
}

sail_status_t current_tls_context_with_flags(struct sail_context **context, int flags) {

    SAIL_CHECK_CONTEXT_PTR(context);

    SAIL_TRY(control_tls_context(context, SAIL_CONTEXT_ALLOCATE));
    SAIL_TRY(init_context(*context, flags));

    return SAIL_OK;
}

sail_status_t load_plugin(struct sail_plugin_info_node *node) {

    SAIL_CHECK_PTR(node);

    /* Already loaded. */
    if (node->plugin != NULL) {
        return SAIL_OK;
    }

    /* Plugin is not loaded. Let's load it. */
    SAIL_TRY(alloc_and_load_plugin(node->plugin_info, &node->plugin));

    return SAIL_OK;
}

sail_status_t load_plugin_by_plugin_info(const struct sail_plugin_info *plugin_info, const struct sail_plugin **plugin) {

    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);
    SAIL_CHECK_PLUGIN_PTR(plugin);

    struct sail_context *context;
    SAIL_TRY(current_tls_context(&context));

    /* Find the plugin in the cache. */
    struct sail_plugin_info_node *node = context->plugin_info_node;
    struct sail_plugin_info_node *found_node = NULL;

    while (node != NULL) {
        if (node->plugin_info == plugin_info) {
            if (node->plugin != NULL) {
                *plugin = node->plugin;
                return SAIL_OK;
            }

            found_node = node;
            break;
        }

        node = node->next;
    }

    /* Something weird. The pointer to the plugin info is not found the cache. */
    if (found_node == NULL) {
        return SAIL_ERROR_PLUGIN_NOT_FOUND;
    }

    SAIL_TRY(load_plugin(found_node));

    *plugin = found_node->plugin;

    return SAIL_OK;
}

void destroy_hidden_state(struct hidden_state *state) {

    if (state == NULL) {
        return;
    }

    if (state->own_io) {
        sail_destroy_io(state->io);
    }

    sail_destroy_write_options(state->write_options);

    /* This state must be freed and zeroed by plugins. We free it just in case to avoid memory leaks. */
    sail_free(state->state);

    sail_free(state);
}

sail_status_t stop_writing(void *state, size_t *written) {

    if (written != NULL) {
        *written = 0;
    }

    /* Not an error. */
    if (state == NULL) {
        return SAIL_OK;
    }

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    /* Not an error. */
    if (state_of_mind->plugin == NULL) {
        destroy_hidden_state(state_of_mind);
        return SAIL_OK;
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->write_finish(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (written != NULL) {
        state_of_mind->io->tell(state_of_mind->io->stream, written);
    }

    destroy_hidden_state(state_of_mind);

    return SAIL_OK;
}

sail_status_t allowed_read_output_pixel_format(const struct sail_read_features *read_features,
                                                enum SailPixelFormat pixel_format) {

    SAIL_CHECK_READ_FEATURES_PTR(read_features);

    for (int i = 0; i < read_features->output_pixel_formats_length; i++) {
        if (read_features->output_pixel_formats[i] == pixel_format) {
            return SAIL_OK;
        }
    }

    return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
}

sail_status_t allowed_write_output_pixel_format(const struct sail_write_features *write_features,
                                                enum SailPixelFormat input_pixel_format,
                                                enum SailPixelFormat output_pixel_format) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);

    /* Plugins will compute output pixel format automatically. */
    if (output_pixel_format == SAIL_PIXEL_FORMAT_AUTO) {
        return SAIL_OK;
    }

    /*
     * For example:
     *
     * [write-pixel-formats-mapping]
     * BPP8-GRAYSCALE=SOURCE
     * BPP24-RGB=SOURCE;BPP24-YCBCR;BPP8-GRAYSCALE
     *
     * When input_pixel_format is BPP24-RGB and output_pixel_format is BPP24-YCBCR, success is returned.
     * When input_pixel_format is BPP24-RGB and output_pixel_format is BPP32-CMYK, error is returned.
     */
    const struct sail_pixel_formats_mapping_node *node = write_features->pixel_formats_mapping_node;

    while (node != NULL) {
        if (node->input_pixel_format == input_pixel_format) {
            for (int i = 0; i < node->output_pixel_formats_length; i++) {
                if (node->output_pixel_formats[i] == output_pixel_format) {
                    return SAIL_OK;
                }
            }

            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }

        node = node->next;
    }

    return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
}
