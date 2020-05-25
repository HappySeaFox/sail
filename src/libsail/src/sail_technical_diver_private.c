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

static sail_error_t check_io_arguments(struct sail_io *io, struct sail_context *context,
                                        const struct sail_plugin_info *plugin_info) {

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    return 0;
}

static sail_error_t check_state_ptr(void *state) {

    SAIL_CHECK_STATE_PTR(state);

    return 0;
}

/*
 * Public functions.
 */

sail_error_t start_reading_io_with_options(struct sail_io *io, bool own_io, struct sail_context *context,
                                           const struct sail_plugin_info *plugin_info,
                                           const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, context, plugin_info),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_TRY_OR_CLEANUP(check_state_ptr(state_of_mind),
                        /*cleanup */ if (own_io) sail_destroy_io(io));

    state_of_mind->io            = io;
    state_of_mind->own_io        = own_io;
    state_of_mind->write_options = NULL;
    state_of_mind->state         = NULL;
    state_of_mind->plugin_info   = plugin_info;
    state_of_mind->plugin        = NULL;

    SAIL_TRY_OR_CLEANUP(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (read_options == NULL) {
        struct sail_read_options *read_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(state_of_mind->plugin_info->read_features, &read_options_local),
                            /* cleanup */ sail_destroy_read_options(read_options_local),
                                          destroy_hidden_state(state_of_mind));
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_read_options(read_options_local),
                                          state_of_mind->plugin->v2->read_finish_v2(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
        sail_destroy_read_options(read_options_local);
    } else {
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_init_v2(state_of_mind->io, read_options, &state_of_mind->state),
                            /* cleanup */ state_of_mind->plugin->v2->read_finish_v2(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
    }

    *state = state_of_mind;

    return 0;
}

sail_error_t start_writing_io_with_options(struct sail_io *io, bool own_io, struct sail_context *context,
                                           const struct sail_plugin_info *plugin_info,
                                           const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, context, plugin_info),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    struct hidden_state *state_of_mind = (struct hidden_state *)malloc(sizeof(struct hidden_state));
    SAIL_TRY_OR_CLEANUP(check_state_ptr(state_of_mind),
                        /*cleanup */ if (own_io) sail_destroy_io(io));

    state_of_mind->io            = io;
    state_of_mind->own_io        = own_io;
    state_of_mind->write_options = NULL;
    state_of_mind->state         = NULL;
    state_of_mind->plugin_info   = plugin_info;
    state_of_mind->plugin        = NULL;

    SAIL_TRY_OR_CLEANUP(load_plugin_by_plugin_info(context, state_of_mind->plugin_info, &state_of_mind->plugin),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (write_options == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_write_options_from_features(state_of_mind->plugin_info->write_features, &state_of_mind->write_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_copy_write_options(write_options, &state_of_mind->write_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->write_init_v2(state_of_mind->io, state_of_mind->write_options, &state_of_mind->state),
                        /* cleanup */ state_of_mind->plugin->v2->write_finish_v2(&state_of_mind->state, state_of_mind->io),
                                      destroy_hidden_state(state_of_mind));
    *state = state_of_mind;

    return 0;
}
