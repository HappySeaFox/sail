#include "config.h"

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

/* sail-common */
#include "log.h"
#include "utils.h"

#include "plugin.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_error_t build_full_path(const char *name, char **full_path) {

    /* +2 : NULL and '/' characters. */
    size_t full_path_length = strlen(SAIL_PLUGINS_PATH) + strlen(name) + 2;

    *full_path = (char *)malloc(full_path_length);

    if (*full_path == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

#ifdef SAIL_WIN32
    strcpy_s(*full_path, full_path_length, SAIL_PLUGINS_PATH);
    strcat_s(*full_path, full_path_length, "\\");
    strcat_s(*full_path, full_path_length, name);
#else
    strcpy(*full_path, SAIL_PLUGINS_PATH);
    strcat(*full_path, "/");
    strcat(*full_path, name);
#endif

    return 0;
}

static sail_error_t build_plugin_full_path(struct sail_context *context,
                                            struct sail_plugin_info_node **last_plugin_info_node,
                                            char *plugin_info_full_path) {

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
    plugin_info_node->path = plugin_full_path;

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

    *context = (struct sail_context *)malloc(sizeof(struct sail_context));

    if (*context == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*context)->plugin_info_node = NULL;

    struct sail_plugin_info_node *last_plugin_info_node;

#ifdef SAIL_WIN32
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(SAIL_PLUGINS_PATH "\\*.plugin.info", &data);

    if (hFind == INVALID_HANDLE_VALUE) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d", SAIL_PLUGINS_PATH, GetLastError());
        return SAIL_DIR_OPEN_ERROR;
    }

    do {
        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        if (build_full_path(data.cFileName, &full_path) != 0) {
            continue;
        }

        build_plugin_full_path(*context, &last_plugin_info_node, full_path);

        free(full_path);
    } while (FindNextFile(hFind, &data));

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d. Some plugins may be ignored", SAIL_PLUGINS_PATH, GetLastError());
    }

    FindClose(hFind);
#else
    DIR *d = opendir(SAIL_PLUGINS_PATH);

    if (d == NULL) {
        SAIL_LOG_ERROR("Failed to list files in '%s': %s", SAIL_PLUGINS_PATH, strerror(errno));
        return SAIL_DIR_OPEN_ERROR;
    }

    struct dirent *dir;

    while ((dir = readdir(d)) != NULL) {

        /* Build a full path. */
        char *full_path;

        /* Ignore errors and try to load as much as possible. */
        if (build_full_path(dir->d_name, &full_path) != 0) {
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

    return 0;
}

void sail_finish(struct sail_context *context) {

    if (context == NULL) {
        return;
    }

    sail_destroy_plugin_info_node_chain(context->plugin_info_node);

    free(context);
}
