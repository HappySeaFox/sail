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

static int pixel_format_from_string(const char *str) {

    return sail_pixel_format_from_string(str);
}

static int compression_from_string(const char *str) {

    return sail_compression_from_string(str);
}

static sail_status_t parse_serialized_ints(const char *value, int **target, unsigned *length, int (*converter)(const char *str)) {

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(target);
    SAIL_CHECK_PTR(length);

    struct sail_string_node *string_node;
    SAIL_TRY(sail_split_into_string_node_chain(value, &string_node));

    *length = 0;

    for (struct sail_string_node *node = string_node; node != NULL; node = node->next) {
        (*length)++;
    }

    if (*length > 0) {
        void *ptr;
        SAIL_TRY_OR_CLEANUP(sail_malloc((size_t)*length * sizeof(int), &ptr),
                            /* cleanup */ sail_destroy_string_node_chain(string_node));
        *target = ptr;

        int i = 0;

        for (struct sail_string_node *node = string_node; node != NULL; node = node->next) {
            *(*target + i++) = converter(node->string);
        }
    }

    sail_destroy_string_node_chain(string_node);

    return SAIL_OK;
}

static int codec_feature_from_string(const char *str) {

    return sail_codec_feature_from_string(str);
}

static sail_status_t parse_flags(const char *value, int *features, int (*converter)(const char *str)) {

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(features);

    struct sail_string_node *string_node;
    SAIL_TRY(sail_split_into_string_node_chain(value, &string_node));

    *features = 0;

    for (struct sail_string_node *node = string_node; node != NULL; node = node->next) {
        *features |= converter(node->string);
    }

    sail_destroy_string_node_chain(string_node);

    return SAIL_OK;
}

struct init_data {
    struct sail_codec_info *codec_info;
};

static sail_status_t codec_priority_from_string(const char *str, enum SailCodecPriority *result) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_ERROR_UNSUPPORTED_CODEC_PRIORITY);

    switch (hash) {
        case UINT64_C(229425771102513): *result = SAIL_CODEC_PRIORITY_HIGHEST; return SAIL_OK;
        case UINT64_C(6384110277):      *result = SAIL_CODEC_PRIORITY_HIGH;    return SAIL_OK;
        case UINT64_C(6952486921094):   *result = SAIL_CODEC_PRIORITY_MEDIUM;  return SAIL_OK;
        case UINT64_C(193462455):       *result = SAIL_CODEC_PRIORITY_LOW;     return SAIL_OK;
        case UINT64_C(6952460323299):   *result = SAIL_CODEC_PRIORITY_LOWEST;  return SAIL_OK;

        default: return SAIL_ERROR_UNSUPPORTED_CODEC_PRIORITY;
    }
}

static sail_status_t inih_handler_sail_error(void *data, const char *section, const char *name, const char *value) {

    /* Silently ignore empty values. */
    if (strlen(value) == 0) {
        return SAIL_OK;
    }

    struct init_data *init_data = (struct init_data *)data;
    struct sail_codec_info *codec_info = init_data->codec_info;

    if (strcmp(section, "codec") == 0) {
        if (strcmp(name, "layout") == 0) {
            codec_info->layout = atoi(value);
        } else if (strcmp(name, "version") == 0) {
            SAIL_TRY(sail_strdup(value, &codec_info->version));
        } else if (strcmp(name, "priority") == 0) {
            SAIL_TRY_OR_CLEANUP(codec_priority_from_string(value, &codec_info->priority),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse codec priority: '%s'", value));
        } else if (strcmp(name, "name") == 0) {
            SAIL_TRY(sail_strdup(value, &codec_info->name));
        } else if (strcmp(name, "description") == 0) {
            SAIL_TRY(sail_strdup(value, &codec_info->description));
        } else if (strcmp(name, "magic-numbers") == 0) {
            SAIL_TRY(sail_split_into_string_node_chain(value, &codec_info->magic_number_node));

            for (struct sail_string_node *node = codec_info->magic_number_node; node != NULL; node = node->next) {
                if (strlen(node->string) > SAIL_MAGIC_BUFFER_SIZE * 3 - 1) {
                    SAIL_LOG_ERROR("Magic number '%s' is too long. Magic numbers for the '%s' codec are disabled",
                                    node->string, codec_info->name);
                    sail_destroy_string_node_chain(codec_info->magic_number_node);
                    codec_info->magic_number_node = NULL;
                    break;
                }

                sail_to_lower(node->string);
            }
        } else if (strcmp(name, "extensions") == 0) {
            SAIL_TRY(sail_split_into_string_node_chain(value, &codec_info->extension_node));

            for (struct sail_string_node *node = codec_info->extension_node; node != NULL; node = node->next) {
                sail_to_lower(node->string);
            }
        } else if (strcmp(name, "mime-types") == 0) {
            SAIL_TRY(sail_split_into_string_node_chain(value, &codec_info->mime_type_node));

            for (struct sail_string_node *node = codec_info->mime_type_node; node != NULL; node = node->next) {
                sail_to_lower(node->string);
            }
        } else {
            SAIL_LOG_ERROR("Unsupported codec info key '%s' in [%s]", name, section);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }
    } else if (strcmp(section, "load-features") == 0) {
        if (strcmp(name, "features") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_flags(value, &codec_info->load_features->features, codec_feature_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse codec features: '%s'", value));
        } else if (strcmp(name, "tuning") == 0) {
            SAIL_TRY_OR_CLEANUP(sail_split_into_string_node_chain(value, &codec_info->load_features->tuning),
                                    /* cleanup */ SAIL_LOG_ERROR("Failed to parse codec tuning: '%s'", value));
        } else {
            SAIL_LOG_ERROR("Unsupported codec info key '%s' in [%s]", name, section);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }
    } else if (strcmp(section, "save-features") == 0) {
        if (strcmp(name, "features") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_flags(value, &codec_info->save_features->features, codec_feature_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse codec features: '%s'", value));
        } else if (strcmp(name, "pixel-formats") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_serialized_ints(value,
                                                        (int **)&codec_info->save_features->pixel_formats,
                                                        &codec_info->save_features->pixel_formats_length,
                                                        pixel_format_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse output pixel formats: '%s'", value));
        } else if (strcmp(name, "compressions") == 0) {
            SAIL_TRY_OR_CLEANUP(parse_serialized_ints(value,
                                                        (int **)&codec_info->save_features->compressions,
                                                        &codec_info->save_features->compressions_length,
                                                        compression_from_string),
                                /* cleanup */ SAIL_LOG_ERROR("Failed to parse compressions: '%s'", value));
        } else if (strcmp(name, "default-compression") == 0) {
            codec_info->save_features->default_compression = sail_compression_from_string(value);
        } else if (strcmp(name, "compression-level-min") == 0) {
            if (codec_info->save_features->compression_level == NULL) {
                SAIL_TRY(sail_alloc_compression_level(&codec_info->save_features->compression_level));
            }

            codec_info->save_features->compression_level->min_level = atof(value);
        } else if (strcmp(name, "compression-level-max") == 0) {
            if (codec_info->save_features->compression_level == NULL) {
                SAIL_TRY(sail_alloc_compression_level(&codec_info->save_features->compression_level));
            }

            codec_info->save_features->compression_level->max_level = atof(value);
        } else if (strcmp(name, "compression-level-default") == 0) {
            if (codec_info->save_features->compression_level == NULL) {
                SAIL_TRY(sail_alloc_compression_level(&codec_info->save_features->compression_level));
            }

            codec_info->save_features->compression_level->default_level = atof(value);
        } else if (strcmp(name, "compression-level-step") == 0) {
            if (codec_info->save_features->compression_level == NULL) {
                SAIL_TRY(sail_alloc_compression_level(&codec_info->save_features->compression_level));
            }

            codec_info->save_features->compression_level->step = atof(value);
        } else if (strcmp(name, "tuning") == 0) {
            SAIL_TRY_OR_CLEANUP(sail_split_into_string_node_chain(value, &codec_info->save_features->tuning),
                                    /* cleanup */ SAIL_LOG_ERROR("Failed to parse codec tuning: '%s'", value));
        } else {
            SAIL_LOG_ERROR("Unsupported codec info key '%s' in [%s]", name, section);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }
    } else {
        SAIL_LOG_ERROR("Unsupported codec info section '%s'", section);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
    }

    return SAIL_OK;
}

/* Returns 1 on success. */
static int inih_handler(void *data, const char *section, const char *name, const char *value) {

    SAIL_TRY_OR_EXECUTE(inih_handler_sail_error(data, section, name, value),
                        /* on error */ return 0);

    return 1;
}

static sail_status_t check_codec_info(const struct sail_codec_info *codec_info) {

    if (codec_info->name == NULL || strlen(codec_info->name) == 0) {
        SAIL_LOG_ERROR("Codec validation error: the codec currently being parsed has empty name");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    for (size_t i = 0; i < strlen(codec_info->name); i++) {
        if (codec_info->name[i] >= 'a' && codec_info->name[i] <= 'z') {
            SAIL_LOG_ERROR("Codec validation error: %s codec has lowercase letters in its name", codec_info->name);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
        }
    }

    if (codec_info->version == NULL || strlen(codec_info->version) == 0) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has empty version", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    if (codec_info->description == NULL || strlen(codec_info->description) == 0) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has empty description", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    if (codec_info->magic_number_node == NULL && codec_info->extension_node == NULL && codec_info->mime_type_node == NULL) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has no identification method (magic number or extension or mime type)", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    const struct sail_save_features *save_features = codec_info->save_features;

    /* Check save features. */
    if ((save_features->features & SAIL_CODEC_FEATURE_STATIC ||
            save_features->features & SAIL_CODEC_FEATURE_ANIMATED ||
            save_features->features & SAIL_CODEC_FEATURE_MULTI_PAGED) &&
            (save_features->pixel_formats == NULL || save_features->pixel_formats_length == 0)) {
        SAIL_LOG_ERROR("Codec validation error: %s codec is able to save images, but output pixel formats are not specified", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    /* Compressions must exist if we're able to save this image format.*/
    if (save_features->features != 0 && (save_features->compressions == NULL || save_features->compressions_length == 0)) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has empty compressions list", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    /* Compression levels and types are mutually exclusive.*/
    if (save_features->compressions_length > 1 && save_features->compression_level != NULL &&
            (save_features->compression_level->min_level != 0 || save_features->compression_level->max_level != 0)) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has more than two compression types and non-zero compression levels which is unsupported", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    for (unsigned i = 0; i < save_features->compressions_length; i++) {
        if (save_features->compressions[i] == SAIL_COMPRESSION_UNKNOWN) {
            SAIL_LOG_ERROR("Codec validation error: %s codec has UNKNOWN compression", codec_info->name);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
        }
    }

    if (save_features->compressions_length > 0 && save_features->default_compression == SAIL_COMPRESSION_UNKNOWN) {
        SAIL_LOG_ERROR("Codec validation error: %s codec has UNKNOWN default compression", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
    }

    if (codec_info->save_features->compression_level != NULL) {
        if (codec_info->save_features->compression_level->min_level > codec_info->save_features->compression_level->max_level) {
            SAIL_LOG_ERROR("Codec validation error: %s codec has incorrect compression levels of min(%.1f), max(%.1f)",
                            codec_info->name, codec_info->save_features->compression_level->min_level,
                            codec_info->save_features->compression_level->max_level);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INCOMPLETE_CODEC_INFO);
        }
    }

    return SAIL_OK;
}

static sail_status_t alloc_codec_info(struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(codec_info);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_codec_info), &ptr));
    *codec_info = ptr;

    (*codec_info)->path              = NULL;
    (*codec_info)->layout            = 0;
    (*codec_info)->version           = NULL;
    (*codec_info)->name              = NULL;
    (*codec_info)->description       = NULL;
    (*codec_info)->magic_number_node = NULL;
    (*codec_info)->extension_node    = NULL;
    (*codec_info)->mime_type_node    = NULL;
    (*codec_info)->load_features     = NULL;
    (*codec_info)->save_features    = NULL;

    return SAIL_OK;
}

static sail_status_t codec_read_info_from_input(const char *input, int (*ini_parser)(const char*, ini_handler, void*), struct sail_codec_info **codec_info) {

    struct sail_codec_info *codec_info_local;
    SAIL_TRY(alloc_codec_info(&codec_info_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_load_features(&codec_info_local->load_features),
                        destroy_codec_info(codec_info_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_save_features(&codec_info_local->save_features),
                        destroy_codec_info(codec_info_local));

    struct init_data init_data;
    init_data.codec_info = codec_info_local;

    /*
     * Returns:
     *  - 0 on success
     *  - line number of first error on parse error
     *  - -1 on file open error
     *  - -2 on memory allocation error (only when INI_USE_STACK is zero).
     */
    const int code = ini_parser(input, inih_handler, &init_data);

    /* Success. */
    if (code == 0) {
        if (codec_info_local->layout != SAIL_CODEC_LAYOUT_V7) {
            SAIL_LOG_ERROR("Unsupported codec layout version %d. Please check your codec info files", codec_info_local->layout);
            destroy_codec_info(codec_info_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_CODEC_LAYOUT);
        }

        /* Paranoid error checks. */
        SAIL_TRY_OR_CLEANUP(check_codec_info(codec_info_local),
                            /* cleanup */ destroy_codec_info(codec_info_local));

        *codec_info = codec_info_local;

        return SAIL_OK;
    } else {
        destroy_codec_info(codec_info_local);

        switch (code) {
            case -1: SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
            case -2: SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);

            default: SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }
    }
}

/*
 * Public functions.
 */

void destroy_codec_info(struct sail_codec_info *codec_info) {

    if (codec_info == NULL) {
        return;
    }

    sail_free(codec_info->path);
    sail_free(codec_info->version);
    sail_free(codec_info->name);
    sail_free(codec_info->description);

    sail_destroy_string_node_chain(codec_info->magic_number_node);
    sail_destroy_string_node_chain(codec_info->extension_node);
    sail_destroy_string_node_chain(codec_info->mime_type_node);

    sail_destroy_load_features(codec_info->load_features);
    sail_destroy_save_features(codec_info->save_features);

    sail_free(codec_info);
}

sail_status_t codec_read_info_from_file(const char *path, struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(path);
    SAIL_CHECK_PTR(codec_info);

    SAIL_LOG_DEBUG("Loading codec info '%s'", path);

    SAIL_TRY(codec_read_info_from_input(path, ini_parse, codec_info));

    return SAIL_OK;
}

sail_status_t codec_read_info_from_string(const char *str, struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(str);
    SAIL_CHECK_PTR(codec_info);

    SAIL_TRY(codec_read_info_from_input(str, ini_parse_string, codec_info));

    return SAIL_OK;
}
