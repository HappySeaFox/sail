#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libsail-common */
#include "error.h"
#include "log.h"
#include "utils.h"

#include "ini.h"
#include "plugin_info.h"
#include "string_node.h"

/*
 * Private functions.
 */

static int alloc_string_node(struct sail_string_node **string_node) {

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

    if (string_node->value != NULL) {
        free(string_node->value);
    }

    free(string_node);
}

static void destroy_string_node_chain(struct sail_string_node *string_node) {

    while (string_node != NULL) {
        struct sail_string_node *string_node_next = string_node->next;

        destroy_string_node(string_node);

        string_node = string_node_next;
    }
}

static void to_lower(char *str) {

    if (str == NULL) {
        return;
    }

    size_t length = strlen(str);

    for (size_t i = 0; i < length; i++) {
        str[i] = (char)tolower(str[i]);
    }
}

static int inih_handler(void *data, const char *section, const char *name, const char *value) {

    (void)section;

    struct sail_plugin_info *plugin_info = (struct sail_plugin_info *)data;

    int res;

    if (strcmp(name, "layout") == 0) {
        plugin_info->layout = atoi(value);

        if (plugin_info->layout < 1) {
            SAIL_LOG_ERROR("Failed to convert '%s' to a plugin layout version", value);
            return 0;
        }

        return 1;
    }

    if (plugin_info->layout == 0) {
        SAIL_LOG_ERROR("Plugin layout version is unset. Make sure a plugin layout version is the very first key in the plugin info file");
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
            struct sail_string_node *last_extension_node = NULL;

            while (*(value += strspn(value, ";")) != '\0') {
                size_t length = strcspn(value, ";");

                struct sail_string_node *extension_node;

                if (alloc_string_node(&extension_node) != 0) {
                    return 0;
                }

                if (sail_strdup_length(value, length, &extension_node->value) != 0) {
                    destroy_string_node(extension_node);
                    return 0;
                }

                to_lower(extension_node->value);

                if (plugin_info->extension_node == NULL) {
                    plugin_info->extension_node = last_extension_node = extension_node;
                } else {
                    last_extension_node->next = extension_node;
                    last_extension_node = extension_node;
                }

                value += length;
            }
        } else if (strcmp(name, "mime-types") == 0) {
            struct sail_string_node *last_mime_type_node = NULL;

            while (*(value += strspn(value, ";")) != '\0') {
                size_t length = strcspn(value, ";");

                struct sail_string_node *mime_type_node;

                if (alloc_string_node(&mime_type_node) != 0) {
                    return 0;
                }

                if (sail_strdup_length(value, length, &mime_type_node->value) != 0) {
                    destroy_string_node(mime_type_node);
                    return 0;
                }

                to_lower(mime_type_node->value);

                if (plugin_info->mime_type_node == NULL) {
                    plugin_info->mime_type_node = last_mime_type_node = mime_type_node;
                } else {
                    last_mime_type_node->next = mime_type_node;
                    last_mime_type_node = mime_type_node;
                }

                value += length;
            }
        } else if (strcmp(name, "magic") == 0) {
            if ((res = sail_strdup(value, &plugin_info->magic)) != 0) {
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

    (*plugin_info)->layout          = 0;
    (*plugin_info)->version         = NULL;
    (*plugin_info)->description     = NULL;
    (*plugin_info)->extension_node  = NULL;
    (*plugin_info)->mime_type_node  = NULL;
    (*plugin_info)->magic           = NULL;

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

    destroy_string_node_chain(plugin_info->extension_node);
    destroy_string_node_chain(plugin_info->mime_type_node);

    if (plugin_info->magic != NULL) {
        free(plugin_info->magic);
    }

    free(plugin_info);
}

sail_error_t sail_alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node) {

    *plugin_info_node = (struct sail_plugin_info_node *)malloc(sizeof(struct sail_plugin_info_node));

    if (*plugin_info_node == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*plugin_info_node)->plugin_info = NULL;
    (*plugin_info_node)->path        = NULL;
    (*plugin_info_node)->next        = NULL;

    return 0;
}

void sail_destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node) {

    if (plugin_info_node == NULL) {
        return;
    }

    sail_destroy_plugin_info(plugin_info_node->plugin_info);

    if (plugin_info_node->path != NULL) {
        free(plugin_info_node->path);
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
