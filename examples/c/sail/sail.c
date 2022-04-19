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

#include "config.h"

#include <stdio.h>
#include <stdlib.h> /* atoi */
#include <string.h>

#include "sail.h"
#include "sail-manip.h"

static void print_invalid_argument(void) {
    fprintf(stderr, "Error: Invalid arguments. Run with -h to see command arguments.\n");
}

static sail_status_t convert_impl(const char *input, const char *output, int compression) {

    SAIL_CHECK_PTR(input);
    SAIL_CHECK_PTR(output);

    const struct sail_codec_info *codec_info;
    void *state;

    struct sail_image *image;

    /* Load the image. */
    SAIL_LOG_INFO("Input file: %s", input);

    SAIL_TRY(sail_codec_info_from_path(input, &codec_info));
    SAIL_LOG_INFO("Input codec: %s", codec_info->description);

    SAIL_TRY(sail_start_loading_file(input, codec_info, &state));

    SAIL_TRY(sail_load_next_frame(state, &image));
    SAIL_TRY(sail_stop_loading(state));

    /* Save the image. */
    SAIL_LOG_INFO("Output file: %s", output);

    SAIL_TRY(sail_codec_info_from_path(output, &codec_info));
    SAIL_LOG_INFO("Output codec: %s", codec_info->description);

    /* Convert to the best pixel format for saving. */
    {
        struct sail_image *image_converted;
        SAIL_TRY(sail_convert_image_for_saving(image, codec_info->save_features, &image_converted));

        sail_destroy_image(image);
        image = image_converted;
    }

    struct sail_save_options *save_options;
    SAIL_TRY(sail_alloc_save_options_from_features(codec_info->save_features, &save_options));

    /* Apply our tuning. */
    SAIL_LOG_INFO("Compression: %d%s", compression, compression == -1 ? " (default)" : "");
    save_options->compression_level = compression;

    SAIL_TRY(sail_start_saving_file_with_options(output, codec_info, save_options, &state));
    SAIL_TRY(sail_write_next_frame(state, image));
    SAIL_TRY(sail_stop_saving(state));

    /* Clean up. */
    sail_destroy_save_options(save_options);

    sail_destroy_image(image);

    SAIL_LOG_INFO("\n*** Success ***\n");

    return SAIL_OK;
}

static sail_status_t convert(int argc, char *argv[]) {

    if (argc < 4 || argc > 6) {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* -1: default compression will be selected. */
    int compression = -1;

    /* Start parsing CLI options from the third argument. */
    int i = 4;

    while (i < argc) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compression") == 0) {
            if (i == argc-1) {
                fprintf(stderr, "Error: Missing compression value.\n");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }

            compression = atoi(argv[i+1]);
            i += 2;
            continue;
        }

        fprintf(stderr, "Error: Unrecognized option '%s'.\n", argv[i]);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(convert_impl(argv[2], argv[3], compression));

    return SAIL_OK;
}

static sail_status_t probe_impl(const char *path) {

    SAIL_CHECK_PTR(path);

    /* Time counter. */
    uint64_t start_time = sail_now();

    struct sail_image *image;
    const struct sail_codec_info *codec_info;

    SAIL_TRY(sail_probe_file(path, &image, &codec_info));

    printf("File          : %s\n", path);
    printf("Probe time    : %lu ms.\n", (unsigned long)(sail_now() - start_time));
    printf("Codec         : %s [%s]\n", codec_info->name, codec_info->description);
    printf("Codec version : %s\n", codec_info->version);
    printf("Size          : %ux%u\n", image->width, image->height);
    if (image->resolution == NULL) {
        printf("Resolution:   : -\n");
    } else {
        printf("Resolution:   : %.1fx%.1f\n", image->resolution->x, image->resolution->y);
    }
    printf("Color         : %s\n", sail_pixel_format_to_string(image->source_image->pixel_format));
    printf("ICC profile   : %s\n", image->iccp == NULL ? "no" : "yes");
    printf("Interlaced    : %s\n", image->source_image->interlaced ? "yes" : "no");
    printf("Orientation   : %s\n", sail_orientation_to_string(image->orientation));

    for (const struct sail_meta_data_node *meta_data_node = image->meta_data_node; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
        const struct sail_meta_data *meta_data = meta_data_node->meta_data;
        const char *meta_data_str = NULL;

        if (meta_data->key == SAIL_META_DATA_UNKNOWN) {
            meta_data_str = meta_data->key_unknown;
        } else {
            meta_data_str = sail_meta_data_to_string(meta_data->key);
        }

        printf("%-14s: ", meta_data_str);

        switch (meta_data->value->type) {
            case SAIL_VARIANT_TYPE_BOOL:           printf("%s\n",  sail_variant_to_bool(meta_data->value) ? "<set>" : "<unset>");   break;
            case SAIL_VARIANT_TYPE_CHAR:           printf("%d\n",  sail_variant_to_char(meta_data->value));                         break;
            case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  printf("%u\n",  sail_variant_to_unsigned_char(meta_data->value));                break;
            case SAIL_VARIANT_TYPE_SHORT:          printf("%d\n",  sail_variant_to_short(meta_data->value));                        break;
            case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: printf("%u\n",  sail_variant_to_unsigned_short(meta_data->value));               break;
            case SAIL_VARIANT_TYPE_INT:            printf("%d\n",  sail_variant_to_int(meta_data->value));                          break;
            case SAIL_VARIANT_TYPE_UNSIGNED_INT:   printf("%u\n",  sail_variant_to_unsigned_int(meta_data->value));                 break;
            case SAIL_VARIANT_TYPE_LONG:           printf("%ld\n", sail_variant_to_long(meta_data->value));                         break;
            case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  printf("%lu\n", sail_variant_to_unsigned_long(meta_data->value));                break;
            case SAIL_VARIANT_TYPE_FLOAT:          printf("%.1f\n", sail_variant_to_float(meta_data->value));                       break;
            case SAIL_VARIANT_TYPE_DOUBLE:         printf("%.1f\n", sail_variant_to_double(meta_data->value));                      break;
            case SAIL_VARIANT_TYPE_STRING:         printf("%s\n", sail_variant_to_string(meta_data->value));                        break;
            case SAIL_VARIANT_TYPE_DATA:           printf("<binary data, length: %u byte(s)>\n", (unsigned)meta_data->value->size); break;
            case SAIL_VARIANT_TYPE_INVALID:        printf("<invalid value>\n");                                                     break;
        }
    }

    sail_destroy_image(image);

    return SAIL_OK;
}

static sail_status_t probe(int argc, char *argv[]) {

    if (argc != 3) {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(probe_impl(argv[2]));

    return SAIL_OK;
}

static sail_status_t list_impl(bool verbose) {

    const struct sail_codec_bundle_node *codec_bundle_node = sail_codec_bundle_list();

    for (int counter = 1; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next, counter++) {
        const struct sail_codec_info *codec_info = codec_bundle_node->codec_bundle->codec_info;

        printf("%2d. [p%d] %s [%s] %s\n", counter, codec_info->priority, codec_info->name, codec_info->description, codec_info->version);

        if (verbose) {
            if (codec_info->load_features->tuning != NULL) {
                printf("         Tuning: ");

                for (const struct sail_string_node *node = codec_info->load_features->tuning, *prev = NULL;
                        node != NULL;
                        prev = node, node = node->next) {
                    if (prev != NULL) {
                        printf(", ");
                    }

                    printf("%s", node->string);
                }

                printf("\n");
            }
        }
    }

    return SAIL_OK;
}

static sail_status_t list(int argc, char *argv[]) {

    (void)argv;

    if (argc < 2 || argc > 3) {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    bool verbose;

    if (argc == 2) {
        verbose = false;
    } else {
        verbose = strcmp(argv[2], "-v") == 0;
    }

    SAIL_TRY(list_impl(verbose));

    return SAIL_OK;
}

static void help(const char *app) {

    fprintf(stderr, "SAIL command-line utility.\n\n");
    fprintf(stderr, "Usage: %s <command> <command arguments>\n", app);
    fprintf(stderr, "       %s [-v | --version]\n", app);
    fprintf(stderr, "       %s [-h | --help]\n", app);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "    convert <INPUT PATH> <OUTPUT PATH> [-c | --compression <value>] - Convert one image format to another.\n");
    fprintf(stderr, "    list [-v] - List supported codecs.\n");
    fprintf(stderr, "    probe <PATH> - Retrieve image information.\n");
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        help(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        help(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        fprintf(stderr, "SAIL command-line utility 1.3.0\n");
        fprintf(stderr, "SAIL library %s\n", SAIL_VERSION_STRING);
        return 0;
    }

    sail_set_log_barrier(SAIL_LOG_LEVEL_WARNING);

    if (strcmp(argv[1], "convert") == 0) {
        SAIL_TRY(convert(argc, argv));
    } else if (strcmp(argv[1], "list") == 0) {
        SAIL_TRY(list(argc, argv));
    } else if (strcmp(argv[1], "probe") == 0) {
        SAIL_TRY(probe(argc, argv));
    } else {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    printf("Success\n");

    sail_finish();

    return 0;
}
