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

#include "sail.h"

static char* build_full_path(const char *name) {

    /* +2 : NULL and '/' characters. */
    char *full_path = (char *)(malloc(strlen(SAIL_PLUGINS_PATH) + strlen(name) + 2));

    if (full_path == NULL) {
        return NULL;
    }

    strcpy(full_path, SAIL_PLUGINS_PATH);
    strcat(full_path, "/");
    strcat(full_path, name);

    return full_path;
}

sail_error_t sail_init(struct sail_context **context) {

    *context = (struct sail_context *)malloc(sizeof(struct sail_context));

    if (*context == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

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
        if (sail_strdup_length(plugin_info_full_path, plugin_full_path_length+3, &plugin_full_path) != 0) {
            free(plugin_info_full_path);
            continue;
        }

        /* Overwrite the end of the path with "so". */
        strcpy(plugin_full_path+plugin_full_path_length+1, "so");

        SAIL_LOG_INFO("PLUGIN %s %s\n", plugin_info_full_path, plugin_full_path);

        free(plugin_info_full_path);
        free(plugin_full_path);
    }

    closedir(d);
#endif

    return 0;
}

void sail_finish(struct sail_context *context) {

    if (context == NULL) {
        return;
    }

    free(context);
}
