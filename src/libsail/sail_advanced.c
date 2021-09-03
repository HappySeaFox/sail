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

sail_status_t sail_probe_io(struct sail_io *io, struct sail_image **image, const struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(io);

    const struct sail_codec_info *codec_info_noop;
    const struct sail_codec_info **codec_info_local = codec_info == NULL ? &codec_info_noop : codec_info;

    SAIL_TRY(sail_codec_info_by_magic_number_from_io(io, codec_info_local));

    const struct sail_codec *codec;
    SAIL_TRY(load_codec_by_codec_info(*codec_info_local, &codec));

    struct sail_read_options *read_options_local;
    SAIL_TRY(sail_alloc_read_options_from_features((*codec_info_local)->read_features, &read_options_local));

    void *state = NULL;
    SAIL_TRY_OR_CLEANUP(codec->v5->read_init(io, read_options_local, &state),
                        /* cleanup */ codec->v5->read_finish(&state, io),
                                      sail_destroy_read_options(read_options_local));

    sail_destroy_read_options(read_options_local);

    struct sail_image *image_local;

    SAIL_TRY_OR_CLEANUP(codec->v5->read_seek_next_frame(state, io, &image_local),
                        /* cleanup */ codec->v5->read_finish(&state, io));
    SAIL_TRY_OR_CLEANUP(codec->v5->read_finish(&state, io),
                        /* ceanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_probe_mem(const void *buffer, size_t buffer_length, struct sail_image **image, const struct sail_codec_info **codec_info) {

    SAIL_CHECK_PTR(buffer);

    struct sail_io *io;
    SAIL_TRY(alloc_io_read_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(sail_probe_io(io, image, codec_info),
                        /* cleanup */ sail_destroy_io(io));

    sail_destroy_io(io);

    return SAIL_OK;
}

sail_status_t sail_start_reading_file(const char *path, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_reading_file_with_options(path, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_reading_mem(const void *buffer, size_t buffer_length, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_reading_mem_with_options(buffer, buffer_length, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_read_next_frame(void *state, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_TRY(sail_check_io_valid(state_of_mind->io));
    SAIL_CHECK_PTR(state_of_mind->state);
    SAIL_CHECK_PTR(state_of_mind->codec);

    struct sail_image *image_local;
    SAIL_TRY(state_of_mind->codec->v5->read_seek_next_frame(state_of_mind->state, state_of_mind->io, &image_local));

    if (image_local->pixels != NULL) {
        SAIL_LOG_ERROR("Internal error in %s codec: codecs must not allocate pixels", state_of_mind->codec_info->name);
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CONFLICTING_OPERATION);
    }

    /* Detect the number of passes needed to write an interlaced image. */
    int interlaced_passes;
    if (image_local->source_image->properties & SAIL_IMAGE_PROPERTY_INTERLACED) {
        interlaced_passes = image_local->interlaced_passes;

        if (interlaced_passes < 1) {
            sail_destroy_image(image_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INTERLACING_UNSUPPORTED);
        }
    } else {
        interlaced_passes = 1;
    }

    /* Allocate pixels. */
    const unsigned pixels_size = image_local->height * image_local->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &image_local->pixels),
                        /* cleanup */ sail_destroy_image(image_local));

    for (int pass = 0; pass < interlaced_passes; pass++) {
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v5->read_seek_next_pass(state_of_mind->state, state_of_mind->io, image_local),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v5->read_frame(state_of_mind->state, state_of_mind->io, image_local),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

sail_status_t sail_stop_reading(void *state) {

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

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v5->read_finish(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    destroy_hidden_state(state_of_mind);

    return SAIL_OK;
}

sail_status_t sail_start_writing_file(const char *path, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_writing_file_with_options(path, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_writing_mem(void *buffer, size_t buffer_length, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_writing_mem_with_options(buffer, buffer_length, codec_info, NULL, state));

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

    /* Check if we actually able to write the requested pixel format. */
    SAIL_TRY(allowed_write_output_pixel_format(state_of_mind->codec_info->write_features,
                                                image->pixel_format));

    /* Detect the number of passes needed to write an interlaced image. */
    int interlaced_passes;
    if (state_of_mind->write_options->io_options & SAIL_IO_OPTION_INTERLACED) {
        interlaced_passes = state_of_mind->codec_info->write_features->interlaced_passes;

        if (interlaced_passes < 1) {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INTERLACING_UNSUPPORTED);
        }
    } else {
        interlaced_passes = 1;
    }

    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

    SAIL_TRY(state_of_mind->codec->v5->write_seek_next_frame(state_of_mind->state, state_of_mind->io, image));

    for (int pass = 0; pass < interlaced_passes; pass++) {
        SAIL_TRY(state_of_mind->codec->v5->write_seek_next_pass(state_of_mind->state, state_of_mind->io, image));

        SAIL_TRY(state_of_mind->codec->v5->write_frame(state_of_mind->state,
                                                        state_of_mind->io,
                                                        image));
    }

    return SAIL_OK;
}

sail_status_t sail_stop_writing(void *state) {

    SAIL_TRY(stop_writing(state, NULL));

    return SAIL_OK;
}
