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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_status_t alloc_string_node(struct sail_string_node **string_node) {

    SAIL_CHECK_STRING_NODE_PTR(string_node);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_string_node)));
    *string_node = ptr;

    (*string_node)->value = NULL;
    (*string_node)->next  = NULL;

    return SAIL_OK;
}

static void destroy_string_node(struct sail_string_node *string_node) {

    if (string_node == NULL) {
        return;
    }

    sail_free(string_node->value);
    sail_free(string_node);
}

static void destroy_string_node_chain(struct sail_string_node *string_node) {

    while (string_node != NULL) {
        struct sail_string_node *string_node_next = string_node->next;

        destroy_string_node(string_node);

        string_node = string_node_next;
    }
}

static sail_status_t split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node) {

    struct sail_string_node **last_string_node = target_string_node;

    while (*(value += strspn(value, ";")) != '\0') {
        size_t length = strcspn(value, ";");

        struct sail_string_node *string_node;

        SAIL_TRY(alloc_string_node(&string_node));

        SAIL_TRY_OR_CLEANUP(sail_strdup_length(value, length, &string_node->value),
                            /* cleanup */ destroy_string_node(string_node));

        *last_string_node = string_node;
        last_string_node = &string_node->next;

        value += length;
    }

    return SAIL_OK;
}

static sail_status_t pixel_format_from_string(const char *str, int *result) {

    SAIL_TRY(sail_pixel_format_from_string(str, (enum SailPixelFormat*)result));

    return SAIL_OK;
}

static sail_status_t compression_type_from_string(const char *str, int *result) {

    SAIL_TRY(sail_compression_type_from_string(str, (enum SailCompressionType*)result));

    return SAIL_OK;
}

static sail_status_t parse_serialized_ints(const char *value, int **target, int *length, sail_status_t (*converter)(const char *str, int *result)) {

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(target);
    SAIL_CHECK_PTR(length);

    struct sail_string_node *string_node = NULL;

    SAIL_TRY_OR_CLEANUP(split_into_string_node_chain(value, &string_node),
                        /* cleanup */ destroy_string_node_chain(string_node));

    *length = 0;
    struct sail_string_node *node = string_node;

    while (node != NULL) {
        (*length)++;
        node = node->next;
    }

    if (*length > 0) {
        *target = (int *)malloc((size_t)*length * sizeof(int));

        if (*target == NULL) {
            SAIL_LOG_ERROR("Failed to allocate %d integers", *length);
            destroy_string_node_chain(string_node);
            return SAIL_ERROR_MEMORY_ALLOCATION;
        }

        node = string_node;
        int i = 0;

        while (node != NULL) {

            SAIL_TRY_OR_CLEANUP(converter(node->value, *target + i),
                                /* cleanup */ SAIL_LOG_ERROR("Conversion of '%s' failed", node->value),
                                              destroy_string_node_chain(string_node));
            i++;
            node = node->next;
        }
    }

    destroy_string_node_chain(string_node);

    return SAIL_OK;
}

static sail_status_t plugin_feature_from_string(const char *str, int *result) {

    SAIL_TRY(sail_plugin_feature_from_string(str, (enum SailPluginFeature *)result));

    return SAIL_OK;
}

static sail_status_t image_property_from_string(const char *str, int *result) {

    SAIL_TRY(sail_image_property_from_string(str, (enum SailImageProperty *)result));

    return SAIL_OK;
}

static sail_status_t parse_flags(const char *value, int *features, sail_status_t (*converter)(const char *str, int *result)) {

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(features);

    struct sail_string_node *string_node = NULL;

    SAIL_TRY_OR_CLEANUP(split_into_string_node_chain(value, &string_node),
                        /* cleanup */ destroy_string_node_chain(string_node));

    *features = 0;

    struct sail_string_node *node = string_node;

    while (node != NULL) {
        int flag;
        SAIL_TRY_OR_CLEANUP(converter(node->value, &flag),
                            /* cleanup */ SAIL_LOG_ERROR("Conversion of '%s' failed", node->value),
                                          destroy_string_node_chain(string_node));
        *features |= flag;
        node = node->next;
    }

    destroy_string_node_chain(string_node);

    return SAIL_OK;
}

struct init_data {
    struct sail_plugin_info *plugin_info;
    struct sail_pixel_formats_mapping_node **last_mapping_node;
};

static sail_status_t inih_handler_sail_error(void *data, const char *section, const char *name, const char *value) {

    /* Silently ignore empty values. */
    if (strlen(value) == 0) {
        return SAIL_OK;
    }

    struct init_data *init_data = (struct init_data *)data;
    struct sail_plugin_info *plugin_info = init_data->plugin_info;

    if (strcmp(section, "plugin") == 0) {
        if (strcmp(name, "layout") == 0) {
            plugin_info->layout = atoi(value);
        } else if (strcmp(name, "version") == 0) {
            SAIL_TRY(sail_strdup(value, &plugin_info->version));
        } else if (strcmp(name, "name") == 0) {
            SAIL_TRY(sail_strdup(value, &plugin_info->name));
        } else if (strcmp(name, "description") == 0) {
            SAIL_TRY(sail_strdup(value, &plugin_info->description));
        } else if (strcmp(name, "magic-numbers") == 0) {
            SAIL_TRY(split_into_string_node_chain(value, &plugin_info->magic_number_node));

            struct sail_string_node *node = plugin_info->magic_number_node;

            while (node != NULL) {
                if (strlen(node->value) > SAIL_MAGIC_BUFFER_SIZE * 3 - 1) {
                    SAIL_LOG_ERROR("Magic number '%s' is too long. Magic numbers for the '%s' codec are disabled",
                                    node->value, plugin_info->name);
                    destroy_string_node_chain(plugin_info->magic_number_node);
                    plugin_info->magic_number_node = NULL;
                    break;
                }

                sail_to_lower(node->value);
                node = node->next;
            }
        } else if (strcmp(name, "extensions") == 0) {
            SAIL_TRY(split_into_string_node_chain(value, &plugin_info->extension_node));

            struct sail_string_node *node = plugin_info->extension_node;

            while (node != NULL) {
                sail_to_lower(node->value);
                node = node->next;
            }
        } else if (strcmp(name, "mime-types") == 0) {
            SAIL_TRY(split_into_string_node_chain(value, &plugin_info->mime_type_node));

            struct sail_string_node *node = plugin_info->mime_type_node;

            while (node != NULL) {
                sail_to_lower(node->value);
                node = node->next;
            }
        } else {
            SAIL_LOG_ERROR("Unsupported plugin info key '%s' in [%s]", name, section);
            return SAIL_ERROR_PARSE_FILE;
        }
    } else if (strcmp(section, "read-features") == 0) {
        if (strcmp(name, "output-pixel-formats") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_serialized_ints(value,
                                                        (int **)&plugin_info->read_features->output_pixel_formats,
                                                        &plugin_info->read_features->output_pixel_formats_length,
                                                        pixel_format_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse output pixel formats: '%s'", value));
        } else if (strcmp(name, "default-output-pixel-format") == 0) {
            SAIL_TRY_OR_CLEANUP(sail_pixel_format_from_string(value, &plugin_info->read_features->default_output_pixel_format),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse preferred output pixel format: '%s'", value));
        } else if (strcmp(name, "features") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_flags(value, &plugin_info->read_features->features, plugin_feature_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse plugin features: '%s'", value));
        } else {
            SAIL_LOG_ERROR("Unsupported plugin info key '%s' in [%s]", name, section);
            return SAIL_ERROR_PARSE_FILE;
        }
    } else if (strcmp(section, "write-features") == 0) {
        if (strcmp(name, "features") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_flags(value, &plugin_info->write_features->features, plugin_feature_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse plugin features: '%s'", value));
        } else if (strcmp(name, "properties") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_flags(value, &plugin_info->write_features->properties, image_property_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse image properties: '%s'", value));
        } else if (strcmp(name, "interlaced-passes") == 0) {
            plugin_info->write_features->interlaced_passes = atoi(value);
        } else if (strcmp(name, "compression-types") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_serialized_ints(value,
                                                        (int **)&plugin_info->write_features->compression_types,
                                                        &plugin_info->write_features->compression_types_length,
                                                        compression_type_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse compression types: '%s'", value));
        } else if (strcmp(name, "default-compression-type") == 0) {
            SAIL_TRY_OR_CLEANUP(sail_compression_type_from_string(value, &plugin_info->write_features->default_compression_type),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse compression type: '%s'", value));
        } else if (strcmp(name, "compression-min") == 0) {
            plugin_info->write_features->compression_min = atoi(value);
        } else if (strcmp(name, "compression-max") == 0) {
            plugin_info->write_features->compression_max = atoi(value);
        } else if (strcmp(name, "compression-default") == 0) {
            plugin_info->write_features->compression_default = atoi(value);
        } else {
            SAIL_LOG_ERROR("Unsupported plugin info key '%s' in [%s]", name, section);
            return SAIL_ERROR_PARSE_FILE;
        }
    } else if (strcmp(section, "write-pixel-formats-mapping") == 0) {
        struct sail_pixel_formats_mapping_node *node;

        SAIL_TRY_OR_CLEANUP(sail_alloc_pixel_formats_mapping_node(&node),
                            /* cleanup */ SAIL_LOG_ERROR("Failed to allocate a new write mapping node"));

        SAIL_TRY_OR_CLEANUP(sail_pixel_format_from_string(name, &node->input_pixel_format),
                            /* cleanup */ SAIL_LOG_ERROR("Failed to parse write pixel format: '%s'", name),
                                          sail_destroy_pixel_formats_mapping_node(node));

        SAIL_TRY_OR_CLEANUP(parse_serialized_ints(value,
                                                    (int **)&node->output_pixel_formats,
                                                    &node->output_pixel_formats_length,
                                                    pixel_format_from_string),
                            /* cleanup */ SAIL_LOG_ERROR("Failed to parse mapped write pixel formats: '%s'", value),
                                          sail_destroy_pixel_formats_mapping_node(node));

        *(init_data->last_mapping_node) = node;
        init_data->last_mapping_node = &node->next;
    } else {
        SAIL_LOG_ERROR("Unsupported plugin info section '%s'", section);
        return SAIL_ERROR_PARSE_FILE;
    }

    return SAIL_OK;
}

/* Returns 1 on success. */
static int inih_handler(void *data, const char *section, const char *name, const char *value) {

    SAIL_TRY_OR_EXECUTE(inih_handler_sail_error(data, section, name, value),
                        /* on error */ return 0);

    return 1;
}

static sail_status_t check_plugin_info(const char *path, const struct sail_plugin_info *plugin_info) {

    const struct sail_write_features *write_features = plugin_info->write_features;

    /* Check write features. */
    if ((write_features->features & SAIL_PLUGIN_FEATURE_STATIC ||
            write_features->features & SAIL_PLUGIN_FEATURE_ANIMATED ||
            write_features->features & SAIL_PLUGIN_FEATURE_MULTI_FRAME) && write_features->pixel_formats_mapping_node == NULL) {
        SAIL_LOG_ERROR("The plugin '%s' is able to write images, but output pixel formats mappings are not specified", path);
        return SAIL_ERROR_INCOMPLETE_PLUGIN_INFO;
    }

    return SAIL_OK;
}

static sail_status_t alloc_plugin_info(struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_plugin_info)));
    *plugin_info = ptr;

    (*plugin_info)->path              = NULL;
    (*plugin_info)->layout            = 0;
    (*plugin_info)->version           = NULL;
    (*plugin_info)->name              = NULL;
    (*plugin_info)->description       = NULL;
    (*plugin_info)->magic_number_node = NULL;
    (*plugin_info)->extension_node    = NULL;
    (*plugin_info)->mime_type_node    = NULL;
    (*plugin_info)->read_features     = NULL;
    (*plugin_info)->write_features    = NULL;

    return SAIL_OK;
}

static void destroy_plugin_info(struct sail_plugin_info *plugin_info) {

    if (plugin_info == NULL) {
        return;
    }

    sail_free(plugin_info->path);
    sail_free(plugin_info->version);
    sail_free(plugin_info->name);
    sail_free(plugin_info->description);

    destroy_string_node_chain(plugin_info->magic_number_node);
    destroy_string_node_chain(plugin_info->extension_node);
    destroy_string_node_chain(plugin_info->mime_type_node);

    sail_destroy_read_features(plugin_info->read_features);
    sail_destroy_write_features(plugin_info->write_features);

    sail_free(plugin_info);
}

/*
 * Public functions.
 */

sail_status_t alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node) {

    SAIL_CHECK_PLUGIN_INFO_NODE_PTR(plugin_info_node);

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct sail_plugin_info_node)));
    *plugin_info_node = ptr;

    (*plugin_info_node)->plugin_info = NULL;
    (*plugin_info_node)->plugin      = NULL;
    (*plugin_info_node)->next        = NULL;

    return SAIL_OK;
}

void destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node) {

    if (plugin_info_node == NULL) {
        return;
    }

    destroy_plugin_info(plugin_info_node->plugin_info);
    destroy_plugin(plugin_info_node->plugin);

    sail_free(plugin_info_node);
}

void destroy_plugin_info_node_chain(struct sail_plugin_info_node *plugin_info_node) {

    while (plugin_info_node != NULL) {
        struct sail_plugin_info_node *plugin_info_node_next = plugin_info_node->next;

        destroy_plugin_info_node(plugin_info_node);

        plugin_info_node = plugin_info_node_next;
    }
}

sail_status_t plugin_read_info(const char *path, struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    SAIL_LOG_DEBUG("Loading plugin info '%s'", path);

    SAIL_TRY(alloc_plugin_info(plugin_info));
    SAIL_TRY_OR_CLEANUP(sail_alloc_read_features(&(*plugin_info)->read_features),
                        destroy_plugin_info(*plugin_info));
    SAIL_TRY_OR_CLEANUP(sail_alloc_write_features(&(*plugin_info)->write_features),
                        destroy_plugin_info(*plugin_info));

    struct init_data init_data;
    init_data.plugin_info = *plugin_info;
    init_data.last_mapping_node = &(*plugin_info)->write_features->pixel_formats_mapping_node;

    /*
     * Returns:
     *  - 0 on success
     *  - line number of first error on parse error
     *  - -1 on file open error
     *  - -2 on memory allocation error (only when INI_USE_STACK is zero).
     */
    const int code = ini_parse(path, inih_handler, &init_data);

    /* Success. */
    if (code == 0) {
        if ((*plugin_info)->layout != SAIL_PLUGIN_LAYOUT_V3) {
            SAIL_LOG_ERROR("Unsupported plugin layout version %d in '%s'", (*plugin_info)->layout, path);
            destroy_plugin_info(*plugin_info);
            return SAIL_ERROR_UNSUPPORTED_PLUGIN_LAYOUT;
        }

        /* Paranoid error checks. */
        SAIL_TRY_OR_CLEANUP(check_plugin_info(path, *plugin_info),
                            /* cleanup */ destroy_plugin_info(*plugin_info));

        return SAIL_OK;
    } else {
        destroy_plugin_info(*plugin_info);

        switch (code) {
            case -1: return SAIL_ERROR_OPEN_FILE;
            case -2: return SAIL_ERROR_MEMORY_ALLOCATION;

            default: return SAIL_ERROR_PARSE_FILE;
        }
    }
}
