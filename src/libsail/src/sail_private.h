/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
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

SAIL_HIDDEN sail_error_t load_plugin(struct sail_plugin_info_node *node);

SAIL_HIDDEN sail_error_t load_plugin_by_plugin_info(struct sail_context *context,
                                                    const struct sail_plugin_info *plugin_info,
                                                    const struct sail_plugin **plugin);

SAIL_HIDDEN void destroy_hidden_state(struct hidden_state *state);

SAIL_HIDDEN sail_error_t stop_writing(void *state, size_t *written);

SAIL_HIDDEN sail_error_t allowed_read_output_pixel_format(const struct sail_read_features *read_features,
                                                            enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_error_t allowed_write_output_pixel_format(const struct sail_write_features *write_features,
                                                            enum SailPixelFormat input_pixel_format,
                                                            enum SailPixelFormat output_pixel_format);

#endif
