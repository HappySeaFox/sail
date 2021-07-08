/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avif/avif.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct avif_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    struct avifIO *avif_io;
    struct avifDecoder *avif_decoder;
};

static sail_status_t alloc_avif_state(struct avif_state **avif_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct avif_state), &ptr));
    *avif_state = ptr;

    (*avif_state)->read_options  = NULL;
    (*avif_state)->write_options = NULL;
    (*avif_state)->avif_io       = NULL;
    (*avif_state)->avif_decoder  = NULL;

    SAIL_TRY(sail_malloc(sizeof(struct avifIO), &ptr));
    (*avif_state)->avif_io = ptr;

    (*avif_state)->avif_io->destroy    = NULL;
    (*avif_state)->avif_io->read       = avif_private_read_proc;
    (*avif_state)->avif_io->write      = NULL;
    (*avif_state)->avif_io->sizeHint   = 0;
    (*avif_state)->avif_io->persistent = AVIF_TRUE;
    (*avif_state)->avif_io->data       = NULL;

    (*avif_state)->avif_decoder = avifDecoderCreate();
    avifDecoderSetIO((*avif_state)->avif_decoder, (*avif_state)->avif_io);

    return SAIL_OK;
}

static void destroy_avif_state(struct avif_state *avif_state) {

    if (avif_state == NULL) {
        return;
    }

    sail_destroy_read_options(avif_state->read_options);
    sail_destroy_write_options(avif_state->write_options);

    avifDecoderDestroy(avif_state->avif_decoder);

    sail_free(avif_state->avif_io);

    sail_free(avif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v5_avif(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    /* Allocate a new state. */
    struct avif_state *avif_state;
    SAIL_TRY(alloc_avif_state(&avif_state));
    *state = avif_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &avif_state->read_options));

    avif_state->avif_decoder->ignoreExif = avif_state->avif_decoder->ignoreXMP = (avif_state->read_options->io_options & SAIL_IO_OPTION_EXIF) == 0;

    /* Initialize AVIF. */
    avif_state->avif_io->data = io;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v5_avif(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_IMAGE_PTR(image);

    struct avif_state *avif_state = (struct avif_state *)state;

    avifResult avif_result = avifDecoderParse(avif_state->avif_decoder);

    if (avif_result != AVIF_RESULT_OK) {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    avif_result = avifDecoderNextImage(avif_state->avif_decoder);

    if (avif_result != AVIF_RESULT_OK) {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    SAIL_LOG_DEBUG("AVIF: Image %ux%u@%u", avif_state->avif_decoder->image->width, avif_state->avif_decoder->image->height, avif_state->avif_decoder->image->depth);

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    image_local->width = avif_state->avif_decoder->image->width;
    image_local->height = avif_state->avif_decoder->image->height;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v5_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v5_avif(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct avif_state *avif_state = (struct avif_state *)state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v5_avif(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct avif_state *avif_state = (struct avif_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_avif_state(avif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v5_avif(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v5_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v5_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v5_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v5_avif(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
