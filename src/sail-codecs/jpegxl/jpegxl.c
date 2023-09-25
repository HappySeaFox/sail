/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#include <jxl/decode.h>

#include "sail-common.h"

#include "helpers.h"
#include "memory.h"

/*
 * Codec-specific state.
 */
struct jpegxl_state {
    struct sail_io *io;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;

    JxlMemoryManager *memory_manager;
    JxlDecoder *decoder;
};

static sail_status_t alloc_jpegxl_state(struct jpegxl_state **jpegxl_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpegxl_state), &ptr));
    *jpegxl_state = ptr;

    (*jpegxl_state)->io           = NULL;
    (*jpegxl_state)->load_options = NULL;
    (*jpegxl_state)->save_options = NULL;

    (*jpegxl_state)->frame_loaded = false;

    (*jpegxl_state)->memory_manager = NULL;
    (*jpegxl_state)->decoder        = NULL;

    return SAIL_OK;
}

static void destroy_jpegxl_state(struct jpegxl_state *jpegxl_state) {

    if (jpegxl_state == NULL) {
        return;
    }

    sail_destroy_load_options(jpegxl_state->load_options);
    sail_destroy_save_options(jpegxl_state->save_options);

    sail_free(jpegxl_state->memory_manager);
    JxlDecoderDestroy(jpegxl_state->decoder);

    sail_free(jpegxl_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_jpegxl(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct jpegxl_state *jpegxl_state;
    SAIL_TRY(alloc_jpegxl_state(&jpegxl_state));
    *state = jpegxl_state;

    /* Save I/O for further operations. */
    jpegxl_state->io = io;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &jpegxl_state->load_options));

    /* Init decoder. */
    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(JxlMemoryManager), &ptr));
    jpegxl_state->memory_manager = ptr;
    jpegxl_state->memory_manager->opaque = NULL,
    jpegxl_state->memory_manager->alloc  = jpegxl_private_alloc_func,
    jpegxl_state->memory_manager->free   = jpegxl_private_free_func,

    jpegxl_state->decoder = JxlDecoderCreate(jpegxl_state->memory_manager);

    if (JxlDecoderSubscribeEvents(jpegxl_state->decoder, JXL_DEC_BASIC_INFO
                                                            | JXL_DEC_COLOR_ENCODING
                                                            | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to subscribe to decoder events");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpegxl(void *state, struct sail_image **image) {

    struct jpegxl_state *jpegxl_state = state;

    if (jpegxl_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jpegxl_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

#if 0
    image_local->source_image->pixel_format =
        jpegxl_private_sail_pixel_format(jpegxl_image->yuvFormat, jpegxl_image->depth, jpegxl_image->alphaPlane != NULL);
    image_local->source_image->chroma_subsampling = jpegxl_private_sail_chroma_subsampling(jpegxl_image->yuvFormat);
    image_local->source_image->compression = SAIL_COMPRESSION_NONE;

    image_local->width          = jpegxl_state->width;
    image_local->height         = jpegxl_state->height;
    image_local->pixel_format   = jpegxl_private_rgb_sail_pixel_format(jpegxl_state->rgb_image.format, jpegxl_state->rgb_image.depth);
    image_local->delay          = (int)(jpegxl_state->jpegxl_decoder->imageTiming.duration * 1000);
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Fetch ICC profile. */
    if (sail_iccp_codec_option(jpegxl_state->load_options->codec_options)) {
        SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_iccp(&jpegxl_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }
#endif

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_jpegxl(void *state, struct sail_image *image) {

    const struct jpegxl_state *jpegxl_state = state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_jpegxl(void **state) {

    struct jpegxl_state *jpegxl_state = *state;

    *state = NULL;

    destroy_jpegxl_state(jpegxl_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_jpegxl(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_jpegxl(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_jpegxl(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_jpegxl(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
