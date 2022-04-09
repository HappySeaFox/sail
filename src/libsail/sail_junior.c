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

static sail_status_t probe_file_with_io(const char *path, struct sail_image **image, const struct sail_codec_info **codec_info) {

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_file(path, &io));

    SAIL_TRY_OR_CLEANUP(sail_probe_io(io, image, codec_info),
                        /* cleanup */ sail_destroy_io(io));

    sail_destroy_io(io);

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_probe_file(const char *path, struct sail_image **image, const struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(path);

    const struct sail_codec_info *codec_info_noop;
    const struct sail_codec_info **codec_info_local = codec_info == NULL ? &codec_info_noop : codec_info;

    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_path(path, codec_info_local),
                        /* cleanup */ SAIL_TRY(probe_file_with_io(path, image, codec_info)));

    const struct sail_codec *codec;
    SAIL_TRY(load_codec_by_codec_info(*codec_info_local, &codec));

    struct sail_load_options *load_options_local;
    SAIL_TRY(sail_alloc_load_options_from_features((*codec_info_local)->load_features, &load_options_local));

    struct sail_io *io;
    SAIL_TRY_OR_CLEANUP(sail_alloc_io_read_file(path, &io),
                        /* cleanup */ sail_destroy_load_options(load_options_local));

    void *state = NULL;
    SAIL_TRY_OR_CLEANUP(codec->v7->load_init(io, load_options_local, &state),
                        /* cleanup */ codec->v7->load_finish(&state, io),
                                      sail_destroy_io(io),
                                      sail_destroy_load_options(load_options_local));

    sail_destroy_load_options(load_options_local);

    struct sail_image *image_local;

    SAIL_TRY_OR_CLEANUP(codec->v7->load_seek_next_frame(state, io, &image_local),
                        /* cleanup */ codec->v7->load_finish(&state, io),
                                      sail_destroy_io(io));

    SAIL_TRY_OR_CLEANUP(codec->v7->load_finish(&state, io),
                        /* cleanup */ sail_destroy_image(image_local),
                                      sail_destroy_io(io));

    sail_destroy_io(io);

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_load_image_from_file(const char *path, struct sail_image **image) {

    SAIL_CHECK_PTR(path);
    SAIL_CHECK_PTR(image);

    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_loading_file(path, NULL /* codec info */, &state),
                        /* cleanup */ sail_stop_loading(state));

    struct sail_image *image_local;

    SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image_local),
                        /* cleanup */ sail_stop_loading(state));

    SAIL_TRY_OR_CLEANUP(sail_stop_loading(state),
                        /* cleanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_load_image_from_memory(const void *buffer, size_t buffer_length, struct sail_image **image) {

    SAIL_CHECK_PTR(buffer);
    SAIL_CHECK_PTR(image);

    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_loading_memory(buffer, buffer_length, NULL /* codec info */, &state),
                        /* cleanup */ sail_stop_loading(state));

    SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, image),
                        /* cleanup */ sail_stop_loading(state));

    SAIL_TRY_OR_CLEANUP(sail_stop_loading(state),
                        /* cleanup */ sail_destroy_image(*image));

    return SAIL_OK;
}

sail_status_t sail_save_image_into_file(const char *path, const struct sail_image *image) {

    SAIL_CHECK_PTR(path);
    SAIL_TRY(sail_check_image_valid(image));

    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_saving_file(path, NULL /* codec info */, &state),
                        sail_stop_saving(state));

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image),
                        sail_stop_saving(state));

    SAIL_TRY(sail_stop_saving(state));

    return SAIL_OK;
}

sail_status_t sail_save_image_into_memory(void *buffer, size_t buffer_length, const struct sail_image *image, size_t *written) {

    SAIL_CHECK_PTR(buffer);
    SAIL_TRY(sail_check_image_valid(image));

    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_saving_memory(buffer, buffer_length, NULL /* codec info */, &state),
                        sail_stop_saving(state));

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image),
                        sail_stop_saving(state));

    if (written == NULL) {
        SAIL_TRY(sail_stop_saving(state));
    } else {
        SAIL_TRY(sail_stop_saving_with_written(state, written));
    }

    return SAIL_OK;
}
