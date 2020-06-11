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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sail-common.h"
#include "sail.h"

static sail_error_t probe(const char *path, struct sail_context *context) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    /* Time counter. */
    uint64_t start_time;
    SAIL_TRY(sail_now(&start_time));

    struct sail_image *image;
    const struct sail_plugin_info *plugin_info;

    SAIL_TRY(sail_probe(path, context, &image, &plugin_info));

    uint64_t end_time;
    SAIL_TRY(sail_now(&end_time));

    printf("File          : %s\n", path);
    printf("Probe time    : %lu ms.\n", (unsigned long)(end_time - start_time));
    printf("Codec         : %s [%s]\n", plugin_info->name, plugin_info->description);
    printf("Codec version : %s\n", plugin_info->version);
    printf("Size          : %ux%u\n", image->width, image->height);
    const char *pixel_format_str;
    SAIL_TRY(sail_pixel_format_to_string(image->source_pixel_format, &pixel_format_str));
    printf("Color         : %s\n", pixel_format_str);
    printf("ICC profile   : %s\n", image->iccp == NULL ? "no" : "yes");
    printf("Interlaced    : %s\n", (image->source_properties & SAIL_IMAGE_PROPERTY_INTERLACED) ? "yes" : "no");
    printf("Flipped Vert. : %s\n", (image->source_properties & SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY) ? "yes" : "no");

    struct sail_meta_entry_node *node = image->meta_entry_node;

    while (node != NULL) {
        printf("%-14s: %s\n", node->key, node->value);
        node = node->next;
    }

    sail_destroy_image(image);

    return 0;
}

static void help(char *app) {

    fprintf(stderr, "sail-probe: Quickly retrieve image info.\n\n");
    fprintf(stderr, "Usage: %s <PATH TO IMAGE>\n", app);
    fprintf(stderr, "       %s [-v | --version]\n", app);
    fprintf(stderr, "       %s [-h | --help]\n", app);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        help(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        help(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        fprintf(stderr, "sail-probe 1.0\n");
        return 0;
    }

    struct sail_context *context;

    SAIL_TRY(sail_init(&context));

    SAIL_TRY(probe(argv[1], context));

    sail_finish(context);

    return 0;
}
