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
struct jxl_state {
    struct sail_io *io;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;

    JxlMemoryManager *memory_manager;
    JxlDecoder *decoder;
};

static sail_status_t alloc_jxl_state(struct jxl_state **jxl_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jxl_state), &ptr));
    *jxl_state = ptr;

    (*jxl_state)->io           = NULL;
    (*jxl_state)->load_options = NULL;
    (*jxl_state)->save_options = NULL;

    (*jxl_state)->frame_loaded = false;

    (*jxl_state)->memory_manager = NULL;
    (*jxl_state)->decoder        = NULL;

    return SAIL_OK;
}

static void destroy_jxl_state(struct jxl_state *jxl_state) {

    if (jxl_state == NULL) {
        return;
    }

    sail_destroy_load_options(jxl_state->load_options);
    sail_destroy_save_options(jxl_state->save_options);

    sail_free(jxl_state->memory_manager);
    JxlDecoderDestroy(jxl_state->decoder);

    sail_free(jxl_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_jxl(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct jxl_state *jxl_state;
    SAIL_TRY(alloc_jxl_state(&jxl_state));
    *state = jxl_state;

    /* Save I/O for further operations. */
    jxl_state->io = io;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &jxl_state->load_options));

    /* Init decoder. */
    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(JxlMemoryManager), &ptr));
    jxl_state->memory_manager = ptr;
    jxl_state->memory_manager->opaque = NULL,
    jxl_state->memory_manager->alloc  = jxl_private_alloc_func,
    jxl_state->memory_manager->free   = jxl_private_free_func,

    jxl_state->decoder = JxlDecoderCreate(jxl_state->memory_manager);

    if (JxlDecoderSubscribeEvents(jxl_state->decoder, JXL_DEC_BASIC_INFO
                                                      | JXL_DEC_COLOR_ENCODING
                                                      | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JXL: Failed to subscribe to decoder events");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jxl(void *state, struct sail_image **image) {

    struct jxl_state *jxl_state = state;

    if (jxl_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jxl_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

#if 0
    image_local->source_image->pixel_format =
        jxl_private_sail_pixel_format(jxl_image->yuvFormat, jxl_image->depth, jxl_image->alphaPlane != NULL);
    image_local->source_image->chroma_subsampling = jxl_private_sail_chroma_subsampling(jxl_image->yuvFormat);
    image_local->source_image->compression = SAIL_COMPRESSION_NONE;

    image_local->width          = jxl_state->width;
    image_local->height         = jxl_state->height;
    image_local->pixel_format   = jxl_private_rgb_sail_pixel_format(jxl_state->rgb_image.format, jxl_state->rgb_image.depth);
    image_local->delay          = (int)(jxl_state->jxl_decoder->imageTiming.duration * 1000);
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Fetch ICC profile. */
    if (sail_iccp_codec_option(jxl_state->load_options->codec_options)) {
        SAIL_TRY_OR_CLEANUP(jxl_private_fetch_iccp(&jxl_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }
#endif

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_jxl(void *state, struct sail_image *image) {

    const struct jxl_state *jxl_state = state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_jxl(void **state) {

    struct jxl_state *jxl_state = *state;

    *state = NULL;

    destroy_jxl_state(jxl_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_jxl(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_jxl(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_jxl(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_jxl(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
