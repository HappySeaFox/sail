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

static char* build_full_path(const char *name) {

    /* +2 : NULL and '/' characters. */
    size_t full_path_length = strlen(SAIL_PLUGINS_PATH) + strlen(name) + 2;

    char *full_path = (char *)malloc(full_path_length);

    if (full_path == NULL) {
        return NULL;
    }

#ifdef SAIL_WIN32
    strcpy_s(full_path, full_path_length, SAIL_PLUGINS_PATH);
    strcat_s(full_path, full_path_length, "/");
    strcat_s(full_path, full_path_length, name);
#else
    strcpy(full_path, SAIL_PLUGINS_PATH);
    strcat(full_path, "/");
    strcat(full_path, name);
#endif

    return full_path;
}

sail_error_t sail_init(struct sail_context **context) {

    *context = (struct sail_context *)malloc(sizeof(struct sail_context));

    if (*context == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*context)->plugin_info_node = NULL;

#ifdef SAIL_WIN32
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(SAIL_PLUGINS_PATH "\\*", &data);

    if (hFind == INVALID_HANDLE_VALUE) {
        SAIL_LOG_ERROR("Failed to list files in '%s'. Error: %d", SAIL_PLUGINS_PATH, GetLastError());
        return SAIL_DIR_OPEN_ERROR;
    }

    do {
        printf("%s\n", data.cFileName);
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

    struct sail_plugin_info_node *last_plugin_info_node;

    struct dirent *dir;

    while ((dir = readdir(d)) != NULL) {
        /* Build a full path. */
        char *full_path = build_full_path(dir->d_name);

        if (full_path == NULL) {
            closedir(d);
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        /* Handle files only. */
        struct stat full_path_stat;
        stat(full_path, &full_path_stat);
        bool is_file = S_ISREG(full_path_stat.st_mode);

        if (!is_file) {
            free(full_path);
            continue;
        }

        /* Build "/path/jpeg.so" from "/path/jpeg.plugin.info". */
        char *plugin_info_part = strstr(full_path, ".plugin.info");

        if (plugin_info_part == NULL) {
            free(full_path);
            continue;
        }

        /* Use a different pointer just to make its name more clear. */
        char *plugin_info_full_path = full_path;

        /* The length of "/path/jpeg". */
        int plugin_full_path_length = strlen(plugin_info_full_path) - strlen(plugin_info_part);
        char *plugin_full_path;

        /* +3: to append ".so" later. The resulting string will be "/path/jpeg.pl". */
        SAIL_TRY_OR_CLEANUP(sail_strdup_length(plugin_info_full_path, plugin_full_path_length+3, &plugin_full_path),
                            free(plugin_info_full_path),
                            closedir(d));

        /* Overwrite the end of the path with "so". */
        strcpy(plugin_full_path+plugin_full_path_length+1, "so");

        /* Parse plugin info. */
        struct sail_plugin_info_node *plugin_info_node;
        SAIL_TRY_OR_CLEANUP(sail_alloc_plugin_info_node(&plugin_info_node),
                            free(plugin_full_path),
                            free(plugin_info_full_path),
                            closedir(d));

        struct sail_plugin_info *plugin_info;
        SAIL_TRY_OR_CLEANUP(sail_plugin_read_info(plugin_info_full_path, &plugin_info),
                            sail_destroy_plugin_info_node(plugin_info_node),
                            free(plugin_full_path),
                            free(plugin_info_full_path),
                            closedir(d));

        free(plugin_info_full_path);

        /* Save the parsed plugin info into the SAIL context. */
        plugin_info_node->plugin_info = plugin_info;
        plugin_info_node->path = plugin_full_path;

        if ((*context)->plugin_info_node == NULL) {
            (*context)->plugin_info_node = last_plugin_info_node = plugin_info_node;
        } else {
            last_plugin_info_node->next = plugin_info_node;
            last_plugin_info_node = plugin_info_node;
        }
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
