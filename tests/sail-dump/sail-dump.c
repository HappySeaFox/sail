/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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
#include <stdint.h>
#include <string.h>

#ifdef SAIL_WIN32
    /* _fsopen() */
    #include <share.h>
#endif

#include "sail-common.h"

#include "sail-dump.h"

/*
 * Private functions.
 */

static void skip_whitespaces(FILE *fptr) {

#ifdef SAIL_WIN32
    fscanf_s(fptr, "%*[ \r\n]");
#else
    fscanf(fptr, "%*[ \r\n]");
#endif
}

static sail_status_t read_hex(FILE *fptr, size_t data_length, uint8_t **value) {

    void *ptr;
    SAIL_TRY(sail_malloc(data_length, &ptr));
    *value = ptr;

    for (unsigned i = 0; i < data_length; i++) {
        skip_whitespaces(fptr);

        int v;
#ifdef SAIL_WIN32
        if (fscanf_s(fptr, "%2x%*[ \r\n]", &v) != 1) {
#else
        if (fscanf(fptr, "%2x%*[ \r\n]", &v) != 1) {
#endif
            SAIL_LOG_ERROR("DUMP: Failed to read hex element at index %u", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
        }

        *(*value + i) = (uint8_t)v;
    }

    return SAIL_OK;
}

static sail_status_t read_image(FILE *fptr, struct sail_image *image) {

    /*
     * 124 124 62(bpl) BPP4-INDEXED 0(properties)
     */
    char pixel_format[64];

#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%u %u %u %s %d", &image->width, &image->height, &image->bytes_per_line, pixel_format, (unsigned)sizeof(pixel_format), &image->properties) != 5) {
#else
    if (fscanf(fptr, "%u %u %u %s %d", &image->width, &image->height, &image->bytes_per_line, pixel_format, &image->properties) != 5) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read IMAGE properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    SAIL_TRY(sail_pixel_format_from_string(pixel_format, &image->pixel_format));
    const char *pixel_format_str = NULL;
    SAIL_TRY(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));

    SAIL_LOG_DEBUG("DUMP: Image properties: %ux%u bytes_per_line(%u), pixel_format(%s), properties(%d)",
                    image->width, image->height, image->bytes_per_line, pixel_format_str, image->properties);

    return SAIL_OK;
}

static sail_status_t read_source_image(FILE *fptr, struct sail_image *image) {

    /*
     * BPP4-INDEXED 0(properties) NONE(compression)
     */
    SAIL_TRY(sail_alloc_source_image(&image->source_image));

    char pixel_format[64];
    char compression[64];

#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%s %d %s", pixel_format, (unsigned)sizeof(pixel_format), &image->source_image->properties, compression, (unsigned)sizeof(compression)) != 3) {
#else
    if (fscanf(fptr, "%s %d %s", pixel_format, &image->source_image->properties, compression) != 3) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read SOURCE-IMAGE properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    SAIL_TRY(sail_pixel_format_from_string(pixel_format, &image->source_image->pixel_format));
    const char *pixel_format_str = NULL;
    SAIL_TRY(sail_pixel_format_to_string(image->source_image->pixel_format, &pixel_format_str));

    SAIL_TRY(sail_compression_from_string(compression, &image->source_image->compression));
    const char *compression_str = NULL;
    SAIL_TRY(sail_compression_to_string(image->source_image->compression, &compression_str));

    SAIL_LOG_DEBUG("DUMP: Source image properties: pixel_format(%s), properties(%d), compression(%s)",
                    pixel_format_str, image->source_image->properties, compression_str);

    return SAIL_OK;
}

static sail_status_t read_resolution(FILE *fptr, struct sail_image *image) {

    /*
     * 1 1 CENTIMETER
     */
    SAIL_TRY(sail_alloc_resolution(&image->resolution));

    char unit[32];

#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%lf %lf %s", &image->resolution->x, &image->resolution->y, unit, (unsigned)sizeof(unit)) != 3) {
#else
    if (fscanf(fptr, "%lf %lf %s", &image->resolution->x, &image->resolution->y, unit) != 3) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read RESOLUTION properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    if (strcmp(unit, "UNKNOWN") == 0) {
        image->resolution->unit = SAIL_RESOLUTION_UNIT_UNKNOWN;
    } else if (strcmp(unit, "MICROMETER") == 0) {
        image->resolution->unit = SAIL_RESOLUTION_UNIT_MICROMETER;
    } else if (strcmp(unit, "CENTIMETER") == 0) {
        image->resolution->unit = SAIL_RESOLUTION_UNIT_CENTIMETER;
    } else if (strcmp(unit, "METER") == 0) {
        image->resolution->unit = SAIL_RESOLUTION_UNIT_METER;
    } else if (strcmp(unit, "INCH") == 0) {
        image->resolution->unit = SAIL_RESOLUTION_UNIT_INCH;
    } else {
        SAIL_LOG_ERROR("DUMP: Unknown resolution unit '%s'", unit);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
    }

    SAIL_LOG_DEBUG("DUMP: Resolution properties: %.1fx%.1f unit(%s)",
                    image->resolution->x, image->resolution->y, unit);

    return SAIL_OK;
}

static sail_status_t read_animation(FILE *fptr, struct sail_image *image) {

    /*
     * false 0(delay)
     */
    char animated[16];

#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%s %d", animated, (unsigned)sizeof(animated), &image->delay) != 2) {
#else
    if (fscanf(fptr, "%s %d", animated, &image->delay) != 2) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read ANIMATED properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    image->animated = (strcmp(animated, "true") == 0);

    SAIL_LOG_DEBUG("DUMP: Animated properties: animated(%s) delay(%d)", (image->animated ? "true" : "false"), image->delay);

    return SAIL_OK;
}

static sail_status_t read_meta_data(FILE *fptr, struct sail_image *image) {

    /*
     * 2(number of entries)
     * Artist
     * noop (ignored)
     * STRING 144(data length)
     * 00 11 22...
     * UNKNOWN
     * Some Unknown Key
     * DATA 144(data length)
     * 00 11 22...
     */
    struct sail_meta_data_node **last_meta_data_node = &image->meta_data_node;

    unsigned n_of_entries;
#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%u%*[\r\n]", &n_of_entries) != 1) {
#else
    if (fscanf_s(fptr, "%u%*[\r\n]", &n_of_entries) != 1) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read META-DATA number of entries");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    for (unsigned e = 0; e < n_of_entries; e++) {
        char key[32];
        char key_unknown[256];
        char type[16];
        unsigned data_length;

#ifdef SAIL_WIN32
        if (fscanf_s(fptr, "%[^\n]%*[\r\n]%[^\n]%*[\r\n]%s%u%*[\r\n]", key, (unsigned)sizeof(key), key_unknown, (unsigned)sizeof(key_unknown), type, (unsigned)sizeof(type), &data_length) != 4) {
#else
        if (fscanf_s(fptr, "%[^\n]%*[\r\n]%[^\n]%*[\r\n]%s%u%*[\r\n]", key, key_unknown, type, &data_length) != 4) {
#endif
            SAIL_LOG_ERROR("DUMP: Failed to read META-DATA properties");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
        }

        uint8_t *value;
        SAIL_TRY(read_hex(fptr, data_length, &value));

        enum SailMetaData meta_data;
        SAIL_TRY(sail_meta_data_from_string(key, &meta_data));

        enum SailMetaDataType value_type;
        if (strcmp(type, "STRING") == 0) {
            value_type = SAIL_META_DATA_TYPE_STRING;
        } else if (strcmp(type, "DATA") == 0) {
            value_type = SAIL_META_DATA_TYPE_DATA;
        } else {
            SAIL_LOG_ERROR("DUMP: Unknown meta data type '%s'", type);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }

        struct sail_meta_data_node *meta_data_node;

        if (meta_data == SAIL_META_DATA_UNKNOWN) {
            if (value_type == SAIL_META_DATA_TYPE_STRING) {
                SAIL_TRY(sail_alloc_meta_data_node_from_unknown_string(key_unknown, (char *)value, &meta_data_node));
            } else {
                SAIL_TRY(sail_alloc_meta_data_node_from_unknown_data(key_unknown, value, data_length, &meta_data_node));
            }
        } else {
            if (value_type == SAIL_META_DATA_TYPE_STRING) {
                SAIL_TRY(sail_alloc_meta_data_node_from_known_string(meta_data, (char *)value, &meta_data_node));
            } else {
                SAIL_TRY(sail_alloc_meta_data_node_from_known_data(meta_data, value, data_length, &meta_data_node));
            }
        }

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;

        const char *key_str = NULL;
        SAIL_TRY(sail_meta_data_to_string(meta_data_node->key, &key_str));

        SAIL_LOG_DEBUG("DUMP: Meta data properties: key(%s) key_unknown(%s), type(%s), value_length(%u)",
                        key_str, meta_data_node->key_unknown, type, meta_data_node->value_length);
    }

    return SAIL_OK;
}

static sail_status_t read_iccp(FILE *fptr, struct sail_image *image) {

    /*
     * 126(data length)
     * 00 11 22...
     * 00 11 22...
     */
    unsigned data_length;
#ifdef SAIL_WIN32
    if (fscanf_s(fptr, "%u%*[\r\n]", &data_length) != 1) {
#else
    if (fscanf_s(fptr, "%u%*[\r\n]", &data_length) != 1) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read ICCP data length");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    uint8_t *value;
    SAIL_TRY(read_hex(fptr, data_length, &value));

    SAIL_TRY(sail_alloc_iccp_from_data(value, data_length, &image->iccp));

    SAIL_LOG_DEBUG("DUMP: ICCP properties: data_length(%u)", data_length);

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_read_dump(const char *path, struct sail_image **image) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(image);

    char *path_dump;
    SAIL_TRY(sail_concat(&path_dump, 2, path, ".dump"));

#ifdef SAIL_WIN32
    FILE *fptr = _fsopen(path_dump, "r", _SH_DENYWR);
#else
    FILE *fptr = fopen(path_dump, "r");
#endif

    if (fptr == NULL) {
        SAIL_LOG_ERROR("DUMP: Failed to open '%s'", path_dump);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    /* For reading categories: IMAGE, PALETTE etc. */
    char buffer[32];
#ifdef SAIL_WIN32
    while (fscanf_s(fptr, "%[^\n]%*[\r\n]", buffer, (unsigned)sizeof(buffer)) == 1) {
#else
    while (fscanf(fptr, "%[^\n]%*[\r\n]", buffer) == 1) {
#endif

        SAIL_LOG_DEBUG("DUMP: Found category '%s'", buffer);

        if (strcmp(buffer, "IMAGE") == 0) {
            SAIL_TRY(read_image(fptr, image_local));
        } else if (strcmp(buffer, "SOURCE-IMAGE") == 0) {
            SAIL_TRY(read_source_image(fptr, image_local));
        } else if (strcmp(buffer, "RESOLUTION") == 0) {
            SAIL_TRY(read_resolution(fptr, image_local));
        } else if (strcmp(buffer, "ANIMATION") == 0) {
            SAIL_TRY(read_animation(fptr, image_local));
        } else if (strcmp(buffer, "META-DATA") == 0) {
            SAIL_TRY(read_meta_data(fptr, image_local));
        } else if (strcmp(buffer, "ICCP") == 0) {
            SAIL_TRY(read_iccp(fptr, image_local));
        } else {
            SAIL_LOG_ERROR("DUMP: Unknown category '%s'", buffer);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }

        /* Skip two line breaks. */
        skip_whitespaces(fptr);
    }

SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);

    fclose(fptr);

    *image = image_local;

    return SAIL_OK;
}
