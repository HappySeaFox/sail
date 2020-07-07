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

#ifndef SAIL_PLUGIN_H
#define SAIL_PLUGIN_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

/*
 * Currently supported plugin layout version.
 */
#define SAIL_PLUGIN_LAYOUT_V3 3

struct sail_plugin_info;

struct sail_read_features;
struct sail_read_options;
struct sail_write_features;
struct sail_write_options;
struct sail_image;
struct sail_io;

/* V3 declarations. */
typedef sail_error_t (*sail_plugin_read_init_v3_t)           (struct sail_io *io, const struct sail_read_options *read_options, void **state);
typedef sail_error_t (*sail_plugin_read_seek_next_frame_v3_t)(void *state, struct sail_io *io, struct sail_image **image);
typedef sail_error_t (*sail_plugin_read_seek_next_pass_v3_t) (void *state, struct sail_io *io, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_read_frame_v3_t)          (void *state, struct sail_io *io, const struct sail_image *image, void *bits);
typedef sail_error_t (*sail_plugin_read_finish_v3_t)         (void **state, struct sail_io *io);

typedef sail_error_t (*sail_plugin_write_init_v3_t)           (struct sail_io *io, const struct sail_write_options *write_options, void **state);
typedef sail_error_t (*sail_plugin_write_seek_next_frame_v3_t)(void *state, struct sail_io *io, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_write_seek_next_pass_v3_t) (void *state, struct sail_io *io, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_write_frame_v3_t)          (void *state, struct sail_io *io, const struct sail_image *image, const void *bits);
typedef sail_error_t (*sail_plugin_write_finish_v3_t)         (void **state, struct sail_io *io);

struct sail_plugin_layout_v3 {
    sail_plugin_read_init_v3_t            read_init;
    sail_plugin_read_seek_next_frame_v3_t read_seek_next_frame;
    sail_plugin_read_seek_next_pass_v3_t  read_seek_next_pass;
    sail_plugin_read_frame_v3_t           read_frame;
    sail_plugin_read_finish_v3_t          read_finish;

    sail_plugin_write_init_v3_t            write_init;
    sail_plugin_write_seek_next_frame_v3_t write_seek_next_frame;
    sail_plugin_write_seek_next_pass_v3_t  write_seek_next_pass;
    sail_plugin_write_frame_v3_t           write_frame;
    sail_plugin_write_finish_v3_t          write_finish;
};

/*
 * SAIL plugin.
 */
struct sail_plugin {

    /* Layout version. */
    int layout;

    /* System-specific library handle. */
    void *handle;

    /* Plugin interface. */
    struct sail_plugin_layout_v3 *v3;
};

typedef struct sail_plugin sail_plugin_t;

/*
 * Loads the specified plugin by its info and saves its handle and exported interfaces into
 * the specified plugin instance.
 * The assigned plugin MUST be destroyed later with sail_destroy_plugin().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_HIDDEN sail_error_t alloc_plugin(const struct sail_plugin_info *plugin_info, struct sail_plugin **plugin);

/*
 * Destroys the specified plugin and all its internal memory buffers.
 * Does nothing if the plugin is NULL.
 */
SAIL_HIDDEN void destroy_plugin(struct sail_plugin *plugin);

#endif
