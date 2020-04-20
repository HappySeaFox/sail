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
#include <stdint.h>
#include <string.h>

/* libsail-common. */
#include "config.h"
#include "common.h"
#include "meta_entry_node.h"
#include "plugin_info.h"
#include "utils.h"

#include "sail.h"

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <errno.h>
    #include <sys/time.h>
    #include <string.h>
#endif

static uint64_t now() {

#ifdef SAIL_WIN32
    SAIL_THREAD_LOCAL static bool initialized = false;
    SAIL_THREAD_LOCAL static double frequency = 0;

    LARGE_INTEGER li;

    if (!initialized) {
        initialized = true;

        if (!QueryPerformanceFrequency(&li)) {
            fprintf(stderr, "Failed to get the current time. Error: %d\n", GetLastError());
            return 0;
        }

        frequency = (double)li.QuadPart / 1000;
    }

    if (!QueryPerformanceCounter(&li)) {
        fprintf(stderr, "Failed to get the current time. Error: %d\n", GetLastError());
        return 0;
    }

    return (uint64_t)((double)li.QuadPart / frequency);
#else
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        fprintf(stderr, "Failed to get the current time: %s\n", strerror(errno));
        return 0;
    }

    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

static sail_error_t probe(const char *path, struct sail_context *context) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    /* Time counter. */
    const uint64_t start_time = now();

    struct sail_image *image;
    const struct sail_plugin_info *plugin_info;

    SAIL_TRY(sail_probe(path, context, &plugin_info, &image));

    printf("File          : %s\n", path);
    printf("Probe time    : %ld ms.\n", (unsigned long)(now() - start_time));
    printf("Codec         : %s\n", plugin_info->description);
    printf("Codec version : %s\n", plugin_info->version);

    printf("\n");

    printf("Size          : %dx%d\n", image->width, image->height);
    printf("Color         : %s\n", sail_pixel_format_to_string(image->source_pixel_format));

    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != NULL) {
        printf("%-14s: %s\n", node->key, node->value);
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
