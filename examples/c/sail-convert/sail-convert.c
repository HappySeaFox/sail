/*
    Copyright (c) 2020 Dmitry Baryshev

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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libsail-common. */
#include "config.h"
#include "common.h"
#include "log.h"
#include "meta_entry_node.h"
#include "utils.h"

#include "plugin_info.h"
#include "plugin.h"
#include "sail.h"

static sail_error_t convert(const char *input, const char *output, struct sail_context *context, int compression) {

    SAIL_CHECK_PATH_PTR(input);
    SAIL_CHECK_PATH_PTR(output);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info;
    void *pimpl;

    struct sail_image *image;
    void *image_bits;

    /* Read the image. */
    SAIL_LOG_INFO("Input file: %s", input);

    SAIL_TRY(sail_plugin_info_from_path(input, context, &plugin_info));
    SAIL_LOG_INFO("Input codec: %s", plugin_info->description);

    SAIL_TRY(sail_start_reading(input, context, plugin_info, &pimpl));

    SAIL_TRY(sail_read_next_frame(pimpl, &image, &image_bits));
    SAIL_TRY(sail_stop_reading(pimpl));

    /* Write the image. */
    SAIL_LOG_INFO("Output file: %s", output);

    SAIL_TRY(sail_plugin_info_from_path(output, context, &plugin_info));
    SAIL_LOG_INFO("Outpu codec: %s", plugin_info->description);

    struct sail_write_options *write_options;
    SAIL_TRY(sail_alloc_write_options_from_features(plugin_info->write_features, &write_options));

    /* Apply our tuning. */
    SAIL_LOG_INFO("Compression: %d%s", compression, compression == -1 ? " (default)" : "");
    write_options->compression = compression;

    SAIL_TRY(sail_start_writing_with_options(output, context, plugin_info, write_options, &pimpl));
    SAIL_TRY(sail_write_next_frame(pimpl, image, image_bits));
    SAIL_TRY(sail_stop_writing(pimpl));

    /* Clean up. */
    free(image_bits);
    sail_destroy_write_options(write_options);
    sail_destroy_image(image);

    SAIL_LOG_INFO("Success");

    return 0;
}

static void help(char *app) {

    fprintf(stderr, "sail-convert: Convert one image format to another.\n\n");
    fprintf(stderr, "Usage: %s <PATH TO INPUT IMAGE> <PATH TO OUTPUT IMAGE> [-c | --compression <value>]\n", app);
    fprintf(stderr, "       %s [-v | --version]\n", app);
    fprintf(stderr, "       %s [-h | --help]\n", app);
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
        fprintf(stderr, "sail-convert 1.0\n");
        return 0;
    }

    if (argc < 3) {
        help(argv[0]);
        return 1;
    }

    /* -1: default compression will be selected. */
    int compression = -1;

    /* Start parsing CLI options from the third argument. */
    int i = 3;

    while (i < argc) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compression") == 0) {
            if (i == argc-1) {
                fprintf(stderr, "Error: Missing compression value.\n");
                return 1;
            }

            compression = atoi(argv[i+1]);
            i += 2;
            continue;
        }

        fprintf(stderr, "Error: Unrecognized option '%s'.\n", argv[i]);
        return 1;
    }

    struct sail_context *context;

    SAIL_TRY(sail_init(&context));

    SAIL_TRY(convert(argv[1], argv[2], context, compression));

    sail_finish(context);

    return 0;
}
