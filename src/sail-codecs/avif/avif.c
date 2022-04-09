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

#include <avif/avif.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct avif_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    struct avifIO *avif_io;
    struct avifDecoder *avif_decoder;
    struct avifRGBImage rgb_image;
    struct sail_avif_context avif_context;
};

static sail_status_t alloc_avif_state(struct avif_state **avif_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct avif_state), &ptr));
    *avif_state = ptr;

    (*avif_state)->load_options = NULL;
    (*avif_state)->save_options = NULL;
    (*avif_state)->avif_io      = NULL;
    (*avif_state)->avif_decoder = NULL;
    (*avif_state)->avif_io      = NULL;

    SAIL_TRY(sail_malloc(sizeof(struct avifIO), &ptr));
    (*avif_state)->avif_io = ptr;

    (*avif_state)->avif_io->destroy    = NULL;
    (*avif_state)->avif_io->read       = avif_private_read_proc;
    (*avif_state)->avif_io->write      = NULL;
    (*avif_state)->avif_io->sizeHint   = 0;
    (*avif_state)->avif_io->persistent = AVIF_FALSE;
    (*avif_state)->avif_io->data       = NULL;

    (*avif_state)->avif_context.io          = NULL;
    (*avif_state)->avif_context.buffer      = NULL;
    (*avif_state)->avif_context.buffer_size = 0;

    const size_t initial_buffer_size = 10*1024;
    SAIL_TRY(sail_malloc(initial_buffer_size, &ptr));
    (*avif_state)->avif_context.buffer      = ptr;
    (*avif_state)->avif_context.buffer_size = initial_buffer_size;

    (*avif_state)->avif_decoder = avifDecoderCreate();
#if AVIF_VERSION_MAJOR > 0 || AVIF_VERSION_MINOR >= 9
    (*avif_state)->avif_decoder->strictFlags = AVIF_STRICT_DISABLED;
#endif
    avifDecoderSetIO((*avif_state)->avif_decoder, (*avif_state)->avif_io);

    return SAIL_OK;
}

static void destroy_avif_state(struct avif_state *avif_state) {

    if (avif_state == NULL) {
        return;
    }

    avifDecoderDestroy(avif_state->avif_decoder);

    sail_free(avif_state->avif_context.buffer);

    sail_free(avif_state->avif_io);

    sail_destroy_load_options(avif_state->load_options);
    sail_destroy_save_options(avif_state->save_options);

    sail_free(avif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_avif(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct avif_state *avif_state;
    SAIL_TRY(alloc_avif_state(&avif_state));
    *state = avif_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &avif_state->load_options));

    avif_state->avif_decoder->ignoreExif = avif_state->avif_decoder->ignoreXMP = (avif_state->load_options->options & SAIL_OPTION_META_DATA) == 0;

    /* Initialize AVIF. */
    avif_state->avif_context.io = io;
    avif_state->avif_io->data = &avif_state->avif_context;

    avifResult avif_result = avifDecoderParse(avif_state->avif_decoder);

    if (avif_result != AVIF_RESULT_OK) {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_avif(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct avif_state *avif_state = (struct avif_state *)state;

    avifResult avif_result = avifDecoderNextImage(avif_state->avif_decoder);

    if (avif_result == AVIF_RESULT_NO_IMAGES_REMAINING) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    if (avif_result != AVIF_RESULT_OK) {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    const struct avifImage *avif_image = avif_state->avif_decoder->image;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    avifRGBImageSetDefaults(&avif_state->rgb_image, avif_image);
    avif_state->rgb_image.depth = avif_private_round_depth(avif_state->rgb_image.depth);

    image_local->source_image->pixel_format =
        avif_private_sail_pixel_format(avif_image->yuvFormat, avif_image->depth, avif_image->alphaPlane != NULL);
    image_local->source_image->chroma_subsampling = avif_private_sail_chroma_subsampling(avif_image->yuvFormat);
    image_local->source_image->compression = SAIL_COMPRESSION_AV1;

    image_local->width = avif_image->width;
    image_local->height = avif_image->height;
    image_local->pixel_format = avif_private_rgb_sail_pixel_format(avif_state->rgb_image.format, avif_state->rgb_image.depth);
    image_local->delay = (int)(avif_state->avif_decoder->imageTiming.duration * 1000);

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Fetch ICC profile. */
    if (avif_state->load_options->options & SAIL_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(avif_private_fetch_iccp(&avif_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_avif(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct avif_state *avif_state = (struct avif_state *)state;
    const struct avifImage *avif_image = avif_state->avif_decoder->image;

    avif_state->rgb_image.pixels = image->pixels;
    avif_state->rgb_image.rowBytes = image->bytes_per_line;

    avifResult avif_result = avifImageYUVToRGB(avif_image, &avif_state->rgb_image);

    if (avif_result != AVIF_RESULT_OK) {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_avif(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct avif_state *avif_state = (struct avif_state *)(*state);

    *state = NULL;

    destroy_avif_state(avif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_avif(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_avif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_avif(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
