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

#ifndef SAIL_SAIL_PRIVATE_H
#define SAIL_SAIL_PRIVATE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_context;
struct sail_io;
struct sail_plugin_info;
struct sail_plugin_info_node;
struct sail_plugin;
struct sail_read_features;
struct sail_write_features;

struct hidden_state {

    struct sail_io *io;
    bool own_io;

    /*
     * Write operations save write options to check if the interlaced mode was requested on later stages.
     * It's also used to check if the supplied pixel format is supported.
     */
    struct sail_write_options *write_options;

    /* Local state passed to plugin reading and writing functions. */
    void *state;

    /* Pointers to internal data structures so no need to free these. */
    const struct sail_plugin_info *plugin_info;
    const struct sail_plugin *plugin;
};

SAIL_HIDDEN sail_status_t alloc_context(struct sail_context **context);

SAIL_HIDDEN sail_status_t destroy_context(struct sail_context *context);

enum SailContextAction {
    /* Allocates a new TLS context if it's not allocated yet. */
    SAIL_CONTEXT_ALLOCATE,

    /* fetches the current TLS context or NULL if it's not allocated yet. */
    SAIL_CONTEXT_FETCH,

    /* Destroyes the currently existing TLS context. */
    SAIL_CONTEXT_DESTROY,
};

/*
 * Allocates or destroyes the current SAIL TLS context.
 * If it's already allocated, just returns it.
 */
SAIL_HIDDEN sail_status_t control_tls_context(struct sail_context **context, enum SailContextAction action);

/* Initializes the context and loads all the plugin info files if the context is not initialized. */
SAIL_HIDDEN sail_status_t init_context(struct sail_context *context, int flags);

/* Returns the allocated and initialized TLS context. */
SAIL_HIDDEN sail_status_t current_tls_context(struct sail_context **context);

SAIL_HIDDEN sail_status_t load_plugin(struct sail_plugin_info_node *node);

SAIL_HIDDEN sail_status_t load_plugin_by_plugin_info(const struct sail_plugin_info *plugin_info,
                                                    const struct sail_plugin **plugin);

SAIL_HIDDEN void destroy_hidden_state(struct hidden_state *state);

SAIL_HIDDEN sail_status_t stop_writing(void *state, size_t *written);

SAIL_HIDDEN sail_status_t allowed_read_output_pixel_format(const struct sail_read_features *read_features,
                                                            enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_status_t allowed_write_output_pixel_format(const struct sail_write_features *write_features,
                                                            enum SailPixelFormat input_pixel_format,
                                                            enum SailPixelFormat output_pixel_format);

#endif
