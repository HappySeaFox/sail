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

static sail_status_t check_io_arguments(struct sail_io *io,
                                        const struct sail_codec_info *codec_info,
                                        void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(codec_info);
    SAIL_CHECK_PTR(state);

    return SAIL_OK;
}

static sail_status_t allowed_write_compression(const struct sail_save_features *save_features,
                                               enum SailCompression compression) {

    SAIL_CHECK_PTR(save_features);

    for (unsigned i = 0; i < save_features->compressions_length; i++) {
        if (save_features->compressions[i] == compression) {
            return SAIL_OK;
        }
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
}

/*
 * Public functions.
 */

sail_status_t start_loading_io_with_options(struct sail_io *io, bool own_io,
                                            const struct sail_codec_info *codec_info,
                                            const struct sail_load_options *load_options, void **state) {

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, codec_info, state),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    *state = NULL;

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct hidden_state), &ptr),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

    state_of_mind->io           = io;
    state_of_mind->own_io       = own_io;
    state_of_mind->save_options = NULL;
    state_of_mind->state        = NULL;
    state_of_mind->codec_info   = codec_info;
    state_of_mind->codec        = NULL;

    SAIL_TRY_OR_CLEANUP(load_codec_by_codec_info(state_of_mind->codec_info, &state_of_mind->codec),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (load_options == NULL) {
        struct sail_load_options *load_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_load_options_from_features(state_of_mind->codec_info->load_features, &load_options_local),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->load_init(state_of_mind->io, load_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_load_options(load_options_local),
                                          state_of_mind->codec->v7->load_finish(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
        sail_destroy_load_options(load_options_local);
    } else {
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->load_init(state_of_mind->io, load_options, &state_of_mind->state),
                            /* cleanup */ state_of_mind->codec->v7->load_finish(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
    }

    *state = state_of_mind;

    return SAIL_OK;
}

sail_status_t start_saving_io_with_options(struct sail_io *io, bool own_io,
                                           const struct sail_codec_info *codec_info,
                                           const struct sail_save_options *save_options, void **state) {

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, codec_info, state),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    *state = NULL;

    /*
     * When save options is not NULL, we need to check if we can actually output the requested compression.
     * When save options is NULL, we use the default compression which is always acceptable.
     */
    if (save_options != NULL) {
        SAIL_TRY_OR_CLEANUP(allowed_write_compression(codec_info->save_features, save_options->compression),
                            /* cleanup */ if (own_io) sail_destroy_io(io));
    }

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct hidden_state), &ptr),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

    state_of_mind->io           = io;
    state_of_mind->own_io       = own_io;
    state_of_mind->save_options = NULL;
    state_of_mind->state        = NULL;
    state_of_mind->codec_info   = codec_info;
    state_of_mind->codec        = NULL;

    SAIL_TRY_OR_CLEANUP(load_codec_by_codec_info(state_of_mind->codec_info, &state_of_mind->codec),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (save_options == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_save_options_from_features(state_of_mind->codec_info->save_features, &state_of_mind->save_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_copy_save_options(save_options, &state_of_mind->save_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->save_init(state_of_mind->io, state_of_mind->save_options, &state_of_mind->state),
                        /* cleanup */ state_of_mind->codec->v7->save_finish(&state_of_mind->state, state_of_mind->io),
                                      destroy_hidden_state(state_of_mind));

    *state = state_of_mind;

    return SAIL_OK;
}
