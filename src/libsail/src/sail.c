#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <dirent.h>
#endif

/* sail-common */
#include "log.h"

#include "sail.h"

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

    if (d) {
        struct dirent *dir;

        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }

        closedir(d);
    } else {
        SAIL_LOG_ERROR("Failed to open '%s': %s", SAIL_PLUGINS_PATH, strerror(errno));
    }
#endif

    return 0;
}

void sail_finish(struct sail_context *context) {

    if (context == NULL) {
        return;
    }

    free(context);
}
