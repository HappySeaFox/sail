/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libsail-common */
#include "error.h"
#include "log.h"
#include "utils.h"

#include "ini.h"
#include "plugin_info.h"
#include "plugin.h"
#include "string_node.h"

/*
 * Private functions.
 */

static sail_error_t alloc_string_node(struct sail_string_node **string_node) {

    *string_node = (struct sail_string_node *)malloc(sizeof(struct sail_string_node));

    if (*string_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*string_node)->value = NULL;
    (*string_node)->next  = NULL;

    return 0;
}

static void destroy_string_node(struct sail_string_node *string_node) {

    if (string_node == NULL) {
        return;
    }

    free(string_node->value);
    free(string_node);
}

static void destroy_string_node_chain(struct sail_string_node *string_node) {

    while (string_node != NULL) {
        struct sail_string_node *string_node_next = string_node->next;

        destroy_string_node(string_node);

        string_node = string_node_next;
    }
}

static sail_error_t split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node) {

    struct sail_string_node *last_string_node = NULL;

    while (*(value += strspn(value, ";")) != '\0') {
        size_t length = strcspn(value, ";");

        struct sail_string_node *extension_node;

        SAIL_TRY(alloc_string_node(&extension_node));

        SAIL_TRY_OR_CLEANUP(sail_strdup_length(value, length, &extension_node->value),
                            /* cleanup */ destroy_string_node(extension_node));

        sail_to_lower(extension_node->value);

        if (*target_string_node == NULL) {
            *target_string_node = last_string_node = extension_node;
        } else {
            last_string_node->next = extension_node;
            last_string_node = extension_node;
        }

        value += length;
    }

    return 0;
}

static int inih_handler(void *data, const char *section, const char *name, const char *value) {

    (void)section;

    struct sail_plugin_info *plugin_info = (struct sail_plugin_info *)data;

    int res;

    if (strcmp(name, "layout") == 0) {
        plugin_info->layout = atoi(value);

        if (plugin_info->layout == 1) {
            SAIL_LOG_ERROR("Failed to convert '%s' to a plugin layout version", value);
            return 0;
        }

        if (plugin_info->layout != SAIL_PLUGIN_LAYOUT_V2) {
            SAIL_LOG_ERROR("Unsupported plugin layout version %d", plugin_info->layout);
            return 0;
        }

        return 1;
    }

    if (plugin_info->layout == 0) {
        SAIL_LOG_ERROR("Plugin layout version is unset. Make sure a plugin layout version is the very first key in the plugin info file");
        return 0;
    }

    if (plugin_info->layout == SAIL_PLUGIN_LAYOUT_V2) {
        if (strcmp(name, "version") == 0) {
            if ((res = sail_strdup(value, &plugin_info->version)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "name") == 0) {
            if ((res = sail_strdup(value, &plugin_info->name)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "description") == 0) {
            if ((res = sail_strdup(value, &plugin_info->description)) != 0) {
                return 0;
            }
        } else if (strcmp(name, "extensions") == 0) {
            if (split_into_string_node_chain(value, &plugin_info->extension_node) != 0) {
                return 0;
            }
        } else if (strcmp(name, "mime-types") == 0) {
            if (split_into_string_node_chain(value, &plugin_info->mime_type_node) != 0) {
                return 0;
            }
        } else {
            SAIL_LOG_ERROR("Unsupported plugin configuraton key '%s'", name);
            return 0;  /* error */
        }
    } else {
        SAIL_LOG_ERROR("Unsupported plugin layout version %d", plugin_info->layout);
        return 0;
    }

    return 1;
}

/*
 * Public functions.
 */

int sail_alloc_plugin_info(struct sail_plugin_info **plugin_info) {

    *plugin_info = (struct sail_plugin_info *)malloc(sizeof(struct sail_plugin_info));

    if (*plugin_info == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin_info)->layout         = 0;
    (*plugin_info)->version        = NULL;
    (*plugin_info)->name           = NULL;
    (*plugin_info)->description    = NULL;
    (*plugin_info)->extension_node = NULL;
    (*plugin_info)->mime_type_node = NULL;
    (*plugin_info)->path           = NULL;

    return 0;
}

void sail_destroy_plugin_info(struct sail_plugin_info *plugin_info) {

    if (plugin_info == NULL) {
        return;
    }

    free(plugin_info->version);
    free(plugin_info->name);
    free(plugin_info->description);

    destroy_string_node_chain(plugin_info->extension_node);
    destroy_string_node_chain(plugin_info->mime_type_node);

    free(plugin_info->path);
    free(plugin_info);
}

sail_error_t sail_alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node) {

    *plugin_info_node = (struct sail_plugin_info_node *)malloc(sizeof(struct sail_plugin_info_node));

    if (*plugin_info_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin_info_node)->plugin_info = NULL;
    (*plugin_info_node)->plugin      = NULL;
    (*plugin_info_node)->next        = NULL;

    return 0;
}

void sail_destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node) {

    if (plugin_info_node == NULL) {
        return;
    }

    sail_destroy_plugin_info(plugin_info_node->plugin_info);
    sail_destroy_plugin(plugin_info_node->plugin);

    free(plugin_info_node);
}

void sail_destroy_plugin_info_node_chain(struct sail_plugin_info_node *plugin_info_node) {

    while (plugin_info_node != NULL) {
        struct sail_plugin_info_node *plugin_info_node_next = plugin_info_node->next;

        sail_destroy_plugin_info_node(plugin_info_node);

        plugin_info_node = plugin_info_node_next;
    }
}

int sail_plugin_read_info(const char *path, struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_alloc_plugin_info(plugin_info));

    /*
     * Returns:
     *  - 0 on success
     *  - line number of first error on parse error
     *  - -1 on file open error
     *  - -2 on memory allocation error (only when INI_USE_STACK is zero).
     */
    const int code = ini_parse(path, inih_handler, *plugin_info);

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
