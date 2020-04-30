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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_error_t sail_start_reading_io_with_options_impl(struct sail_io *io, bool own_io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_CHECK_STATE_PTR(state_of_mind);

    state_of_mind->io          = io;
    state_of_mind->own_io      = own_io;
    state_of_mind->state       = NULL;
    state_of_mind->plugin_info = plugin_info;
    state_of_mind->plugin      = NULL;

    *state = state_of_mind;

    SAIL_TRY(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin));

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    if (read_options == NULL) {
        struct sail_read_options *read_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(state_of_mind->plugin_info->read_features, &read_options_local),
                            /* cleanup */ sail_destroy_read_options(read_options_local));
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_read_options(read_options_local));
        sail_destroy_read_options(read_options_local);
    } else {
        SAIL_TRY(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options, &state_of_mind->state));
    }

    return 0;
}

static sail_error_t sail_start_writing_io_with_options_impl(struct sail_io *io, bool own_io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_CHECK_STATE_PTR(state_of_mind);

    state_of_mind->io          = io;
    state_of_mind->own_io      = own_io;
    state_of_mind->state       = NULL;
    state_of_mind->plugin_info = plugin_info;
    state_of_mind->plugin      = NULL;

    *state = state_of_mind;

    SAIL_TRY(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin));

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    if (write_options == NULL) {
        struct sail_write_options *write_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_write_options_from_features(state_of_mind->plugin_info->write_features, &write_options_local),
                            /* cleanup */ sail_destroy_write_options(write_options_local));
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->write_init_v2(state_of_mind->io, write_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_write_options(write_options_local));
        sail_destroy_write_options(write_options_local);
    } else {
        SAIL_TRY(state_of_mind->plugin->v2->write_init_v2(state_of_mind->io, write_options, &state_of_mind->state));
    }

    return 0;
}

/*
 * Public functions.
 */

sail_error_t sail_start_reading_io(struct sail_io *io, struct sail_context *context,
                                   const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_reading_io_with_options(struct sail_io *io, struct sail_context *context,
                                                const struct sail_plugin_info *plugin_info,
                                                const struct sail_read_options *read_options, void **state) {

    SAIL_TRY(sail_start_reading_io_with_options_impl(io, false, context, plugin_info, read_options, state));

    return 0;
}

sail_error_t sail_start_reading_file_with_options(const char *path, struct sail_context *context,
                                                  const struct sail_plugin_info *plugin_info,
                                                  const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_file(path, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options_impl(io, true, context, plugin_info_local, read_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_reading_mem_with_options(const void *buffer, unsigned long buffer_length, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info,
                                                 const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options_impl(io, true, context, plugin_info, read_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_writing_io(struct sail_io *io, struct sail_context *context,
                                   const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_writing_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_writing_io_with_options(struct sail_io *io, struct sail_context *context,
                                                const struct sail_plugin_info *plugin_info,
                                                const struct sail_write_options *write_options, void **state) {

    SAIL_TRY(sail_start_writing_io_with_options_impl(io, false, context, plugin_info, write_options, state));

    return 0;
}

sail_error_t sail_start_writing_file_with_options(const char *path, struct sail_context *context,
                                                  const struct sail_plugin_info *plugin_info,
                                                  const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_write_file(path, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_writing_io_with_options_impl(io, true, context, plugin_info_local, write_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_writing_mem_with_options(void *buffer, unsigned long buffer_length, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info,
                                                 const struct sail_write_options *write_options, void **state) {
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_write_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(sail_start_writing_io_with_options_impl(io, true, context, plugin_info, write_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_stop_writing_with_written(void *state, unsigned long *written) {

    *written = 0;

    /* Not an error. */
    if (state == NULL) {
        return 0;
    }

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    if (state_of_mind->io != NULL) {
        state_of_mind->io->tell(state_of_mind->io->stream, written);
    }

    SAIL_TRY(sail_stop_writing(state));

    return 0;
}
