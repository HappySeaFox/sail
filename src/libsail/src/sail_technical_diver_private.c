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

    /*
     * When read options is not NULL, we need to check if we can actually output the requested pixel format.
     * When read options is NULL, we use the preferred output pixel format which is always acceptable.
     */
    if (read_options != NULL) {
        SAIL_TRY_OR_CLEANUP(allowed_read_output_pixel_format(plugin_info->read_features, read_options->output_pixel_format),
                            /* cleanup */ if (own_io) sail_destroy_io(io));
    }

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(&ptr, sizeof(struct hidden_state)),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

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
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->read_init(state_of_mind->io, read_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_read_options(read_options_local),
                                          state_of_mind->plugin->v3->read_finish(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
        sail_destroy_read_options(read_options_local);
    } else {
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->read_init(state_of_mind->io, read_options, &state_of_mind->state),
                            /* cleanup */ state_of_mind->plugin->v3->read_finish(&state_of_mind->state, state_of_mind->io),
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

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(&ptr, sizeof(struct hidden_state)),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

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

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->write_init(state_of_mind->io, state_of_mind->write_options, &state_of_mind->state),
                        /* cleanup */ state_of_mind->plugin->v3->write_finish(&state_of_mind->state, state_of_mind->io),
                                      destroy_hidden_state(state_of_mind));

    *state = state_of_mind;

    return 0;
}
