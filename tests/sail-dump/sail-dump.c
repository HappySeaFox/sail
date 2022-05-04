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
#include <locale.h>

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

#ifdef _MSC_VER
    int ret = fscanf_s(fptr, "%*[ \r\n]");
#else
    int ret = fscanf(fptr, "%*[ \r\n]");
#endif

    (void)ret;
}

static sail_status_t read_hex(FILE *fptr, size_t data_length, uint8_t **value) {

    void *ptr;
    SAIL_TRY(sail_malloc(data_length, &ptr));
    uint8_t *value_local = ptr;

    for (unsigned i = 0; i < data_length; i++) {
        skip_whitespaces(fptr);

        unsigned v;
#ifdef _MSC_VER
        if (fscanf_s(fptr, "%2x%*[ \r\n]", &v) != 1) {
#else
        if (fscanf(fptr, "%2x%*[ \r\n]", &v) != 1) {
#endif
            sail_free(value_local);
            SAIL_LOG_ERROR("DUMP: Failed to read hex element at index %u", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
        }

        *(value_local + i) = (uint8_t)v;
    }

    *value = value_local;

    return SAIL_OK;
}

static sail_status_t print_hex(uint8_t *data, size_t data_length) {

    if (data_length == 0) {
        return SAIL_OK;
    }

    if (data == NULL) {
        SAIL_LOG_ERROR("DUMP: Data length is %u but data is NULL", data_length);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    for (unsigned i = 0; i < data_length; i++) {
        printf("%02x ", *(data + i));
    }

    printf("\n");

    return SAIL_OK;
}

static sail_status_t read_image(FILE *fptr, struct sail_image *image) {

    /*
     * 124 124 62(bpl) BPP4-INDEXED NORMAL(orientation)
     */
    char pixel_format[64];
    char orientation[64];

#ifdef _MSC_VER
    if (fscanf_s(fptr, "%u %u %u %s %s", &image->width, &image->height, &image->bytes_per_line, pixel_format, (unsigned)sizeof(pixel_format), orientation, (unsigned)sizeof(orientation)) != 5) {
#else
    if (fscanf(fptr, "%u %u %u %s %s", &image->width, &image->height, &image->bytes_per_line, pixel_format, orientation) != 5) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read IMAGE properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    image->pixel_format = sail_pixel_format_from_string(pixel_format);
    image->orientation = sail_orientation_from_string(orientation);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("DUMP: Read image with unknown pixel format: '%s'", pixel_format);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SAIL_LOG_DEBUG("DUMP: Image properties: %ux%u bytes_per_line(%u), pixel_format(%s), orientation(%s)",
                    image->width, image->height, image->bytes_per_line, sail_pixel_format_to_string(image->pixel_format), sail_orientation_to_string(image->orientation));

    return SAIL_OK;
}

static sail_status_t read_source_image(FILE *fptr, struct sail_image *image) {

    /*
     * BPP4-INDEXED UNKNOWN(chroma subsampling) NORMAL(orientation) NONE(compression) 1(interlaced)
     */
    SAIL_TRY(sail_alloc_source_image(&image->source_image));

    char pixel_format[64];
    char chroma_subsampling[16];
    char orientation[64];
    char compression[64];
    int interlaced;

#ifdef _MSC_VER
    if (fscanf_s(fptr, "%s %s %s %s %d", pixel_format, (unsigned)sizeof(pixel_format), chroma_subsampling, (unsigned)sizeof(chroma_subsampling), orientation, (unsigned)sizeof(orientation), compression, (unsigned)sizeof(compression), &interlaced) != 5) {
#else
    if (fscanf(fptr, "%s %s %s %s %d", pixel_format, chroma_subsampling, orientation, compression, &interlaced) != 5) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read SOURCE-IMAGE properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    image->source_image->pixel_format       = sail_pixel_format_from_string(pixel_format);
    image->source_image->chroma_subsampling = sail_chroma_subsampling_from_string(chroma_subsampling);
    image->source_image->orientation        = sail_orientation_from_string(orientation);
    image->source_image->compression        = sail_compression_from_string(compression);
    image->source_image->interlaced         = interlaced;

    SAIL_LOG_DEBUG("DUMP: Source image properties: pixel_format(%s), chroma_subsampling(%s), orientation(%s), compression(%s), interlaced(%s)",
                    sail_pixel_format_to_string(image->source_image->pixel_format),
                    sail_chroma_subsampling_to_string(image->source_image->chroma_subsampling),
                    sail_orientation_to_string(image->source_image->orientation),
                    sail_compression_to_string(image->source_image->compression),
                    image->source_image->interlaced ? "yes" : "no");

    return SAIL_OK;
}

static sail_status_t read_resolution(FILE *fptr, struct sail_image *image) {

    /*
     * 1 1 CENTIMETER
     */
    SAIL_TRY(sail_alloc_resolution(&image->resolution));

    char unit[32];

#ifdef _MSC_VER
    if (fscanf_s(fptr, "%lf %lf %s", &image->resolution->x, &image->resolution->y, unit, (unsigned)sizeof(unit)) != 3) {
#else
    if (fscanf(fptr, "%lf %lf %s", &image->resolution->x, &image->resolution->y, unit) != 3) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read RESOLUTION properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    image->resolution->unit = sail_resolution_unit_from_string(unit);

    SAIL_LOG_DEBUG("DUMP: Resolution properties: %.1fx%.1f unit(%s)",
                    image->resolution->x, image->resolution->y, unit);

    return SAIL_OK;
}

static sail_status_t read_animation(FILE *fptr, struct sail_image *image) {

    /*
     * 0(delay)
     */
#ifdef _MSC_VER
    if (fscanf_s(fptr, "%d", &image->delay) != 1) {
#else
    if (fscanf(fptr, "%d", &image->delay) != 1) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read ANIMATED properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    SAIL_LOG_DEBUG("DUMP: Animated properties: delay(%d)", image->delay);

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
#ifdef _MSC_VER
    if (fscanf_s(fptr, "%u%*[\r\n]", &n_of_entries) != 1) {
#else
    if (fscanf(fptr, "%u%*[\r\n]", &n_of_entries) != 1) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read META-DATA number of entries");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    for (unsigned e = 0; e < n_of_entries; e++) {
        char key[32];
        char key_unknown[256];
        char type[16];
        unsigned data_length;

#ifdef _MSC_VER
        if (fscanf_s(fptr, "%[^\n]%*[\r\n]%[^\n]%*[\r\n]%s%u%*[\r\n]", key, (unsigned)sizeof(key), key_unknown, (unsigned)sizeof(key_unknown), type, (unsigned)sizeof(type), &data_length) != 4) {
#else
        if (fscanf(fptr, "%[^\n]%*[\r\n]%[^\n]%*[\r\n]%s%u%*[\r\n]", key, key_unknown, type, &data_length) != 4) {
#endif
            SAIL_LOG_ERROR("DUMP: Failed to read META-DATA properties");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
        }

        uint8_t *value;
        SAIL_TRY(read_hex(fptr, data_length, &value));

        enum SailMetaData meta_data = sail_meta_data_from_string(key);
        enum SailVariantType variant_type;

        if (strcmp(type, "STRING") == 0) {
            variant_type = SAIL_VARIANT_TYPE_STRING;
        } else if (strcmp(type, "DATA") == 0) {
            variant_type = SAIL_VARIANT_TYPE_DATA;
        } else {
            SAIL_LOG_ERROR("DUMP: Unknown meta data type '%s'", type);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }

        struct sail_meta_data_node *meta_data_node;

        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node),
                            /* on error */ sail_free(value));

        if (meta_data == SAIL_META_DATA_UNKNOWN) {
            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_unknown_key(key_unknown, &meta_data_node->meta_data),
                                /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                               sail_free(value));

            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                               sail_free(value));

            if (variant_type == SAIL_VARIANT_TYPE_STRING) {
                SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, (const char *)value),
                                    /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                                   sail_free(value));
            } else {
                SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value, value, data_length),
                                    /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                                   sail_free(value));
            }
        } else {
            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(meta_data, &meta_data_node->meta_data),
                                /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                               sail_free(value));
            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                               sail_free(value));

            if (variant_type == SAIL_VARIANT_TYPE_STRING) {
                SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, (const char *)value),
                                    /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                                   sail_free(value));
            } else {
                SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value, value, data_length),
                                    /* on error */ sail_destroy_meta_data_node(meta_data_node),
                                                   sail_free(value));
            }
        }

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;

        sail_free(value);

        SAIL_LOG_DEBUG("DUMP: Meta data properties: key(%s) key_unknown(%s), type(%s), size(%u)",
                        sail_meta_data_to_string(meta_data_node->meta_data->key), meta_data_node->meta_data->key_unknown, type,
                        meta_data_node->meta_data->value->size);
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
#ifdef _MSC_VER
    if (fscanf_s(fptr, "%u%*[\r\n]", &data_length) != 1) {
#else
    if (fscanf(fptr, "%u%*[\r\n]", &data_length) != 1) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read ICCP data length");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    uint8_t *value;
    SAIL_TRY(read_hex(fptr, data_length, &value));

    SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_from_data(value, data_length, &image->iccp),
                        /* on error */ sail_free(value));

    sail_free(value);

    SAIL_LOG_DEBUG("DUMP: ICCP properties: data_length(%u)", data_length);

    return SAIL_OK;
}

static sail_status_t read_palette(FILE *fptr, struct sail_image *image) {

    /*
     * BPP24-RGB 3(color count) 144(data length)
     * 00 11 22...
     */
    unsigned data_length;
    unsigned color_count;
    char pixel_format[64];

#ifdef _MSC_VER
    if (fscanf_s(fptr, "%s%u%u%*[ \r\n]", pixel_format, (unsigned)sizeof(pixel_format), &color_count, &data_length) != 3) {
#else
    if (fscanf(fptr, "%s%u%u%*[ \r\n]", pixel_format, &color_count, &data_length) != 3) {
#endif
        SAIL_LOG_ERROR("DUMP: Failed to read PALETTE properties");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    uint8_t *value;
    SAIL_TRY(read_hex(fptr, data_length, &value));

    enum SailPixelFormat pixel_format_enum = sail_pixel_format_from_string(pixel_format);

    if (pixel_format_enum == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_free(value);
        SAIL_LOG_ERROR("DUMP: Read palette with unknown pixel format: '%s'", pixel_format);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SAIL_TRY_OR_CLEANUP(sail_alloc_palette_from_data(pixel_format_enum, value, color_count, &image->palette),
                        /* on error */ sail_free(value));

    sail_free(value);

    SAIL_LOG_DEBUG("DUMP: Palette properties: pixel_format(%s), color_count(%u), data_length(%u)",
                    sail_pixel_format_to_string(pixel_format_enum), color_count, data_length);

    return SAIL_OK;
}

static sail_status_t read_pixels(FILE *fptr, struct sail_image *image) {

    /*
     * 00 11 22...
     */
    const unsigned data_length = image->bytes_per_line * image->height;

    uint8_t *value;
    SAIL_TRY(read_hex(fptr, data_length, &value));

    image->pixels = value;

    SAIL_LOG_DEBUG("DUMP: Pixels properties: data_length(%u)", data_length);

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_read_dump(const char *path, struct sail_image *images[]) {

    SAIL_CHECK_PTR(path);

    /*  To scanf dots in floats. */
    setlocale(LC_NUMERIC, "C");

    char *path_dump;
    SAIL_TRY(sail_concat(&path_dump, 2, path, ".dump"));

    SAIL_LOG_DEBUG("DUMP: Opening file '%s'", path_dump);

#ifdef _MSC_VER
    FILE *fptr = _fsopen(path_dump, "r", _SH_DENYWR);
#else
    FILE *fptr = fopen(path_dump, "r");
#endif

    if (fptr == NULL) {
        SAIL_LOG_ERROR("DUMP: Failed to open '%s'", path_dump);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    struct sail_image *image_local = NULL;
    size_t current_image = 0;

    /* For reading categories: IMAGE, PALETTE etc. */
    char buffer[32];
#ifdef _MSC_VER
    while (fscanf_s(fptr, "%[^\n]%*[\r\n]", buffer, (unsigned)sizeof(buffer)) == 1) {
#else
    while (fscanf(fptr, "%[^\n]%*[\r\n]", buffer) == 1) {
#endif
        SAIL_LOG_DEBUG("DUMP: Found category '%s'", buffer);

        if (strcmp(buffer, "IMAGE") == 0) {
            if (image_local != NULL) {
                images[current_image]   = image_local;
                images[current_image+1] = NULL;
                current_image++;
            }

            SAIL_TRY(sail_alloc_image(&image_local));
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
        } else if (strcmp(buffer, "PALETTE") == 0) {
            SAIL_TRY(read_palette(fptr, image_local));
        } else if (strcmp(buffer, "PIXELS") == 0) {
            SAIL_TRY(read_pixels(fptr, image_local));
        } else {
            SAIL_LOG_ERROR("DUMP: Unknown category '%s'", buffer);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_PARSE_FILE);
        }

        /* Skip two line breaks. */
        skip_whitespaces(fptr);
    }

    /* Save the last loaded image. */
    images[current_image]   = image_local;
    images[current_image+1] = NULL;

    fclose(fptr);

    return SAIL_OK;
}

sail_status_t sail_dump(const struct sail_image *image) {

    SAIL_TRY(sail_check_image_valid(image));

    /*  To print dots in floats. */
    setlocale(LC_NUMERIC, "C");

    printf("IMAGE\n%u %u %u %s %s\n\n", image->width, image->height, image->bytes_per_line, sail_pixel_format_to_string(image->pixel_format), sail_orientation_to_string(image->orientation));

    if (image->source_image != NULL) {
        printf("SOURCE-IMAGE\n%s %s %s %s %d\n\n",
                sail_pixel_format_to_string(image->source_image->pixel_format),
                sail_chroma_subsampling_to_string(image->source_image->chroma_subsampling),
                sail_orientation_to_string(image->source_image->orientation),
                sail_compression_to_string(image->source_image->compression),
                image->source_image->interlaced);
    }

    if (image->resolution != NULL) {
        printf("RESOLUTION\n%f %f %s\n\n", image->resolution->x, image->resolution->y, sail_resolution_unit_to_string(image->resolution->unit));
    }

    {
        printf("ANIMATION\n%d\n\n", image->delay);
    }

    {
        unsigned meta_data_count = 0;

        for (const struct sail_meta_data_node *meta_data_node = image->meta_data_node; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
            meta_data_count++;
        }

        if (meta_data_count > 0) {
            printf("META-DATA\n%d\n", meta_data_count);

            for (const struct sail_meta_data_node *meta_data_node = image->meta_data_node; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
                const struct sail_meta_data *meta_data = meta_data_node->meta_data;

                printf("%s\n", sail_meta_data_to_string(meta_data->key));
                printf("%s\n", meta_data->key_unknown == NULL ? "noop" : meta_data->key_unknown);

                const char *type_str = NULL;
                switch (meta_data->value->type) {
                    case SAIL_VARIANT_TYPE_STRING: type_str = "STRING"; break;
                    case SAIL_VARIANT_TYPE_DATA:   type_str = "DATA";   break;
                    default: {
                        SAIL_LOG_ERROR("DUMP: Unknown meta data value type #%d", meta_data->value->type);
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                    }
                }
                printf("%s %u\n", type_str, (unsigned)meta_data->value->size);
                SAIL_TRY(print_hex(meta_data->value->value, meta_data->value->size));
            }

            printf("\n");
        }
    }

    if (image->iccp != NULL) {
        printf("ICCP\n%u\n", image->iccp->data_length);
        SAIL_TRY(print_hex(image->iccp->data, image->iccp->data_length));
        printf("\n");
    }

    if (image->palette != NULL) {
        unsigned palette_size;
        SAIL_TRY(sail_bytes_per_line(image->palette->color_count, image->palette->pixel_format, &palette_size));

        printf("PALETTE\n%s %u %u\n", sail_pixel_format_to_string(image->palette->pixel_format), image->palette->color_count, palette_size);
        SAIL_TRY(print_hex(image->palette->data, palette_size));
        printf("\n");
    }

    {
        printf("PIXELS\n");
        const unsigned pixels_size = image->bytes_per_line * image->height;
        SAIL_TRY(print_hex(image->pixels, pixels_size));
        printf("\n");
    }

    return SAIL_OK;
}
