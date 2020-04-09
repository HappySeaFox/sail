#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libsail-common */
#include "error.h"
#include "log.h"
#include "utils.h"

#include "ini.h"
#include "plugin.h"

int sail_alloc_plugin_info(struct sail_plugin_info **plugin_info) {

    *plugin_info = (struct sail_plugin_info *)malloc(sizeof(struct sail_plugin_info));

    if (*plugin_info == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin_info)->layout      = 0;
    (*plugin_info)->version     = NULL;
    (*plugin_info)->description = NULL;
    (*plugin_info)->extensions  = NULL;
    (*plugin_info)->mime_types  = NULL;
    (*plugin_info)->magic       = NULL;

    return 0;
}

void sail_destroy_plugin_info(struct sail_plugin_info *plugin_info) {

    if (plugin_info == NULL) {
        return;
    }

    if (plugin_info->version != NULL) {
        free(plugin_info->version);
    }
    if (plugin_info->description != NULL) {
        free(plugin_info->description);
    }
    if (plugin_info->extensions != NULL) {
        free(plugin_info->extensions);
    }
    if (plugin_info->mime_types != NULL) {
        free(plugin_info->mime_types);
    }
    if (plugin_info->magic != NULL) {
        free(plugin_info->magic);
    }

    free(plugin_info);
}

static int inih_handler(void *data, const char *section, const char *name, const char *value) {

    (void)section;

    struct sail_plugin_info *plugin_info = (struct sail_plugin_info *)data;

    int res;

    if (strcmp(name, "layout") == 0) {
        plugin_info->layout = atoi(value);
        return 1;
    }

    if (plugin_info->layout == 0) {
        SAIL_LOG_ERROR("Plugin layout version is unknown\n");
        return 0;
    }

    if (plugin_info->layout >= 1 && plugin_info->layout <= 2) {
        if (strcmp(name, "version") == 0) {
            if ((res = sail_strdup(value, &plugin_info->version)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "description") == 0) {
            if ((res = sail_strdup(value, &plugin_info->description)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "extensions") == 0) {
            if ((res = sail_strdup(value, &plugin_info->extensions)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "mime-types") == 0) {
            if ((res = sail_strdup(value, &plugin_info->mime_types)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "magic") == 0) {
            if ((res = sail_strdup(value, &plugin_info->magic)) != 0) {
                return 0;
            }
        } else {
            SAIL_LOG_ERROR("Unsupported plugin configuraton key '%s'\n", name);
            return 0;  /* error */
        }
    } else {
        SAIL_LOG_ERROR("Unsupported plugin layout version %d\n", plugin_info->layout);
        return 0;
    }

    return 1;
}

sail_error_t sail_alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node) {

    *plugin_info_node = (struct sail_plugin_info_node *)malloc(sizeof(struct sail_plugin_info_node));

    if (*plugin_info_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin_info_node)->plugin_info = NULL;
    (*plugin_info_node)->next        = NULL;

    return 0;
}

void sail_destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node) {

    if (plugin_info_node == NULL) {
        return;
    }

    if (plugin_info_node->plugin_info != NULL) {
        free(plugin_info_node->plugin_info);
    }

    free(plugin_info_node);
}

void sail_destroy_plugin_info_node_chain(struct sail_plugin_info_node *plugin_info_node) {

    while (plugin_info_node != NULL) {
        struct sail_plugin_info_node *plugin_info_node_next = plugin_info_node->next;

        sail_destroy_plugin_info_node(plugin_info_node);

        plugin_info_node = plugin_info_node_next;
    }
}

int sail_plugin_read_info(const char *file, struct sail_plugin_info **plugin_info) {

    if (file == NULL) {
        return SAIL_INVALID_ARGUMENT;
    }

    SAIL_TRY(sail_alloc_plugin_info(plugin_info));

    /*
     * Returns 0 on success, line number of first error on parse error (doesn't
     * stop on first error), -1 on file open error, or -2 on memory allocation
     * error (only when INI_USE_STACK is zero).
     */
    const int code = ini_parse(file, inih_handler, *plugin_info);

    if (code == 0) {
        return 0;
    }

    sail_destroy_plugin_info(*plugin_info);

    switch (code) {
        case -1: return SAIL_FILE_OPEN_ERROR;
        case -2: return SAIL_MEMORY_ALLOCATION_FAILED;

        default: return SAIL_FILE_PARSE_ERROR;
    }
}
