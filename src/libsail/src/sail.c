#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
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
