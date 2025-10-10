/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <stdlib.h> /* atoi */
#include <string.h>

#include <sail/sail.h>

#include <sail-manip/sail-manip.h>

static void print_invalid_argument(void) {
    fprintf(stderr, "Error: Invalid arguments. Run with -h to see command arguments.\n");
}

static sail_status_t convert_impl(const char *input, const char *output, enum SailPixelFormat pixel_format, int compression, int max_frames) {

    SAIL_CHECK_PTR(input);
    SAIL_CHECK_PTR(output);

    const struct sail_codec_info *input_codec_info;
    const struct sail_codec_info *output_codec_info;
    void *load_state;
    void *save_state = NULL;

    struct sail_image *image;

    /* Load the image. */
    SAIL_LOG_INFO("Input file: %s", input);

    SAIL_TRY(sail_codec_info_from_path(input, &input_codec_info));
    SAIL_LOG_INFO("Input codec: %s", input_codec_info->description);

    /* Use SOURCE_IMAGE option to preserve original pixel format when possible. */
    struct sail_load_options *load_options;
    SAIL_TRY(sail_alloc_load_options_from_features(input_codec_info->load_features, &load_options));
    load_options->options |= SAIL_OPTION_SOURCE_IMAGE;

    SAIL_TRY_OR_CLEANUP(sail_start_loading_from_file_with_options(input, input_codec_info, load_options, &load_state),
                        /* cleanup */ sail_destroy_load_options(load_options));
    sail_destroy_load_options(load_options);

    /* Setup output. */
    SAIL_LOG_INFO("Output file: %s", output);

    SAIL_TRY(sail_codec_info_from_path(output, &output_codec_info));
    SAIL_LOG_INFO("Output codec: %s", output_codec_info->description);

    struct sail_save_options *save_options;
    SAIL_TRY(sail_alloc_save_options_from_features(output_codec_info->save_features, &save_options));

    /* Apply our tuning. */
    SAIL_LOG_INFO("Compression: %d%s", compression, compression == -1 ? " (default)" : "");
    save_options->compression_level = compression;

    /* Check if output format supports animation or multi-paged. If not, force limit to 1 frame. */
    if (!(output_codec_info->save_features->features & SAIL_CODEC_FEATURE_ANIMATED) &&
        !(output_codec_info->save_features->features & SAIL_CODEC_FEATURE_MULTI_PAGED)) {
        if (max_frames > 0) {
            SAIL_LOG_WARNING("Output format doesn't support animation/multi-page, forcing to 1 frame");
        }
        max_frames = 1;
    }

    /* Convert all frames (or limited number). */
    int frame_count = 0;
    sail_status_t load_status;

    while ((load_status = sail_load_next_frame(load_state, &image)) == SAIL_OK) {
        /* Check max frames limit. */
        if (max_frames > 0 && frame_count >= max_frames) {
            SAIL_LOG_INFO("Reached max frames limit (%d), stopping", max_frames);
            sail_destroy_image(image);
            break;
        }

        SAIL_LOG_INFO("Processing frame #%d", frame_count);

        /* Convert to the specified or best pixel format for saving. */
        struct sail_image *image_converted;
        if (pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN) {
            SAIL_LOG_INFO("Converting to specified pixel format: %s", sail_pixel_format_to_string(pixel_format));
            SAIL_TRY_OR_CLEANUP(sail_convert_image(image, pixel_format, &image_converted),
                                /* cleanup */ sail_destroy_image(image);
                                              if (save_state != NULL) sail_stop_saving(save_state);
                                              sail_destroy_save_options(save_options);
                                              sail_stop_loading(load_state));
        } else {
            SAIL_TRY_OR_CLEANUP(sail_convert_image_for_saving(image, output_codec_info->save_features, &image_converted),
                                /* cleanup */ sail_destroy_image(image);
                                              if (save_state != NULL) sail_stop_saving(save_state);
                                              sail_destroy_save_options(save_options);
                                              sail_stop_loading(load_state));
        }

        sail_destroy_image(image);
        image = image_converted;

        /* Start saving on first frame. */
        if (frame_count == 0) {
            SAIL_TRY_OR_CLEANUP(sail_start_saving_into_file_with_options(output, output_codec_info, save_options, &save_state),
                                /* cleanup */ sail_destroy_image(image);
                                              sail_destroy_save_options(save_options);
                                              sail_stop_loading(load_state));
        }

        /* Write frame. */
        SAIL_TRY_OR_CLEANUP(sail_write_next_frame(save_state, image),
                            /* cleanup */ sail_destroy_image(image);
                                          sail_stop_saving(save_state);
                                          sail_destroy_save_options(save_options);
                                          sail_stop_loading(load_state));

        sail_destroy_image(image);
        frame_count++;
    }

    /* Check if we processed at least one frame. */
    if (frame_count == 0) {
        SAIL_LOG_ERROR("No frames found in input file");
        sail_destroy_save_options(save_options);
        sail_stop_loading(load_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    SAIL_LOG_INFO("Converted %d frame(s)", frame_count);

    /* Clean up. */
    if (save_state != NULL) {
        SAIL_TRY(sail_stop_saving(save_state));
    }
    sail_destroy_save_options(save_options);
    SAIL_TRY(sail_stop_loading(load_state));

    return SAIL_OK;
}

static sail_status_t convert(int argc, char *argv[]) {

    if (argc < 4 || argc > 10) {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* -1: default compression will be selected. */
    int compression = -1;
    /* 0: convert all frames. */
    int max_frames = 0;
    /* UNKNOWN: auto-select best format. */
    enum SailPixelFormat pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;

    /* Start parsing CLI options from the 4th argument. */
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

        if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max-frames") == 0) {
            if (i == argc-1) {
                fprintf(stderr, "Error: Missing max-frames value.\n");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }

            max_frames = atoi(argv[i+1]);
            i += 2;
            continue;
        }

        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pixel-format") == 0) {
            if (i == argc-1) {
                fprintf(stderr, "Error: Missing pixel-format value.\n");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }

            pixel_format = sail_pixel_format_from_string(argv[i+1]);
            if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
                fprintf(stderr, "Error: Unknown pixel format '%s'.\n", argv[i+1]);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }
            i += 2;
            continue;
        }

        fprintf(stderr, "Error: Unrecognized option '%s'.\n", argv[i]);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(convert_impl(argv[2], argv[3], pixel_format, compression, max_frames));

    return SAIL_OK;
}

static bool special_properties_printf_callback(const char *key, const struct sail_variant *value) {

    printf("  %s : ", key);
    sail_printf_variant(value);
    printf("\n");

    return true;
}

static void print_aligned_image_info(const struct sail_image *image) {

    printf("Size          : %ux%u\n", image->width, image->height);

    if (image->resolution == NULL) {
        printf("Resolution:   : -\n");
    } else {
        printf("Resolution:   : %.1fx%.1f\n", image->resolution->x, image->resolution->y);
    }

    printf("Pixel format  : %s\n", sail_pixel_format_to_string(image->source_image->pixel_format));
    printf("ICC profile   : %s\n", image->iccp == NULL ? "no" : "yes");
    printf("Interlaced    : %s\n", image->source_image->interlaced ? "yes" : "no");
    printf("Delay         : %d ms.\n", image->delay);

    if (image->meta_data_node != NULL) {
        printf("Meta data     :\n");

        for (const struct sail_meta_data_node *meta_data_node = image->meta_data_node; meta_data_node != NULL; meta_data_node = meta_data_node->next) {
            const struct sail_meta_data *meta_data = meta_data_node->meta_data;
            const char *meta_data_str = NULL;

            if (meta_data->key == SAIL_META_DATA_UNKNOWN) {
                meta_data_str = meta_data->key_unknown;
            } else {
                meta_data_str = sail_meta_data_to_string(meta_data->key);
            }

            printf("  %-12s: ", meta_data_str);
            sail_printf_variant(meta_data->value);
            printf("\n");
        }
    }

    if (image->special_properties != NULL) {
        printf("Special properties :\n");
        sail_traverse_hash_map(image->special_properties,
                                special_properties_printf_callback);
    }
}

static sail_status_t probe_impl(const char *path) {

    SAIL_CHECK_PTR(path);

    /* Time counter. */
    uint64_t start_time = sail_now();

    struct sail_image *image;
    const struct sail_codec_info *codec_info;

    SAIL_TRY(sail_probe_file(path, &image, &codec_info));

    uint64_t elapsed_time = sail_now() - start_time;

    printf("File          : %s\n", path);
    printf("Codec         : %s [%s]\n", codec_info->name, codec_info->description);
    printf("Codec version : %s\n", codec_info->version);
    printf("Probe time    : %lu ms.\n", (unsigned long)elapsed_time);

    print_aligned_image_info(image);

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

static sail_status_t decode_impl(const char *path) {

    SAIL_CHECK_PTR(path);

    const struct sail_codec_info *codec_info;
    SAIL_TRY(sail_codec_info_from_path(path, &codec_info));

    printf("File          : %s\n", path);
    printf("Codec         : %s [%s]\n", codec_info->name, codec_info->description);
    printf("Codec version : %s\n", codec_info->version);

    /* Time counter. */
    uint64_t start_time = sail_now();

    /* Decode. */
    void *state;
    SAIL_TRY(sail_start_loading_from_file(path, codec_info, &state));

    struct sail_image *image;
    sail_status_t status;
    unsigned frame = 0;

    while ((status = sail_load_next_frame(state, &image)) == SAIL_OK) {
        printf("Frame #%u\n", frame++);
        print_aligned_image_info(image);
        sail_destroy_image(image);
    }

    if (status != SAIL_ERROR_NO_MORE_FRAMES) {
        sail_stop_loading(state);
        fprintf(stderr, "Error: Decoder error %d.\n", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SAIL_TRY(sail_stop_loading(state));

    uint64_t elapsed_time = sail_now() - start_time;

    printf("Decode time   : %lu ms.\n", (unsigned long)elapsed_time);

    return SAIL_OK;
}

static sail_status_t decode(int argc, char *argv[]) {

    if (argc != 3) {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(decode_impl(argv[2]));

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
    fprintf(stderr, "    list [-v] - List supported codecs.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    convert <INPUT PATH> <OUTPUT PATH> [-p | --pixel-format <value>]\n");
    fprintf(stderr, "                                       [-c | --compression <value>]\n");
    fprintf(stderr, "                                       [-m | --max-frames <value>]\n");
    fprintf(stderr, "            Convert one image format to another.\n");
    fprintf(stderr, "            Supports both static and animated images (all frames are converted by default).\n");
    fprintf(stderr, "            Use -p to specify target pixel format (e.g., BPP24-RGB, BPP8-INDEXED).\n");
    fprintf(stderr, "            Use -m to limit number of frames to convert from animations.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    probe <PATH> - Retrieve information of the very first image frame found in the file.\n");
    fprintf(stderr, "                   In most cases probing doesn't decode the image data.\n");
    fprintf(stderr, "    decode <PATH> - Decode the whole file and print information of all its frames.\n");
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
        fprintf(stderr, "SAIL command-line utility 1.5.0\n");
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
    } else if (strcmp(argv[1], "decode") == 0) {
        SAIL_TRY(decode(argc, argv));
    } else {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    printf("Success\n");

    sail_finish();

    return 0;
}
