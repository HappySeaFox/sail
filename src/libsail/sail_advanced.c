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

#include <stddef.h>
#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

sail_status_t sail_probe_io(struct sail_io *io, struct sail_image **image, const struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(io);

    const struct sail_codec_info *codec_info_noop;
    const struct sail_codec_info **codec_info_local = codec_info == NULL ? &codec_info_noop : codec_info;

    SAIL_TRY(sail_codec_info_by_magic_number_from_io(io, codec_info_local));

    const struct sail_codec *codec;
    SAIL_TRY(load_codec_by_codec_info(*codec_info_local, &codec));

    struct sail_load_options *load_options_local;
    SAIL_TRY(sail_alloc_load_options_from_features((*codec_info_local)->load_features, &load_options_local));

    void *state = NULL;
    SAIL_TRY_OR_CLEANUP(codec->v7->load_init(io, load_options_local, &state),
                        /* cleanup */ codec->v7->load_finish(&state, io),
                                      sail_destroy_load_options(load_options_local));

    sail_destroy_load_options(load_options_local);

    struct sail_image *image_local;

    SAIL_TRY_OR_CLEANUP(codec->v7->load_seek_next_frame(state, io, &image_local),
                        /* cleanup */ codec->v7->load_finish(&state, io));
    SAIL_TRY_OR_CLEANUP(codec->v7->load_finish(&state, io),
                        /* ceanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_probe_memory(const void *buffer, size_t buffer_length, struct sail_image **image, const struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(buffer);

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_memory(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(sail_probe_io(io, image, codec_info),
                        /* cleanup */ sail_destroy_io(io));

    sail_destroy_io(io);

    return SAIL_OK;
}

sail_status_t sail_start_loading_file(const char *path, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_loading_file_with_options(path, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_loading_memory(const void *buffer, size_t buffer_length, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_loading_memory_with_options(buffer, buffer_length, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_load_next_frame(void *state, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_TRY(sail_check_io_valid(state_of_mind->io));
    SAIL_CHECK_PTR(state_of_mind->state);
    SAIL_CHECK_PTR(state_of_mind->codec);

    struct sail_image *image_local;
    SAIL_TRY(state_of_mind->codec->v7->load_seek_next_frame(state_of_mind->state, state_of_mind->io, &image_local));

    if (image_local->pixels != NULL) {
        SAIL_LOG_ERROR("Internal error in %s codec: codecs must not allocate pixels", state_of_mind->codec_info->name);
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CONFLICTING_OPERATION);
    }

    /* Allocate pixels. */
    const size_t pixels_size = (size_t)image_local->height * image_local->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->load_frame(state_of_mind->state, state_of_mind->io, image_local),
                        /* cleanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_stop_loading(void *state) {

    /* Not an error. */
    if (state == NULL) {
        return SAIL_OK;
    }

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    /* Not an error. */
    if (state_of_mind->codec == NULL) {
        destroy_hidden_state(state_of_mind);
        return SAIL_OK;
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->load_finish(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    destroy_hidden_state(state_of_mind);

    return SAIL_OK;
}

sail_status_t sail_start_saving_file(const char *path, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_saving_file_with_options(path, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_saving_memory(void *buffer, size_t buffer_length, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_saving_memory_with_options(buffer, buffer_length, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_write_next_frame(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_TRY(sail_check_io_valid(state_of_mind->io));
    SAIL_CHECK_PTR(state_of_mind->state);
    SAIL_CHECK_PTR(state_of_mind->codec_info);
    SAIL_CHECK_PTR(state_of_mind->codec);

    /* Check if we actually able to save the requested pixel format. */
    SAIL_TRY(allowed_write_output_pixel_format(state_of_mind->codec_info->save_features,
                                                image->pixel_format));

    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

    SAIL_TRY(state_of_mind->codec->v7->save_seek_next_frame(state_of_mind->state, state_of_mind->io, image));
    SAIL_TRY(state_of_mind->codec->v7->save_frame(state_of_mind->state, state_of_mind->io, image));

    return SAIL_OK;
}

sail_status_t sail_stop_saving(void *state) {

    SAIL_TRY(stop_saving(state, NULL));

    return SAIL_OK;
}
