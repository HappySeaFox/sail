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

#include <webp/decode.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct webp_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    WebPDecoderConfig webp_config;
    WebPIDecoder *webp_decoder;
};

static sail_status_t alloc_webp_state(struct webp_state **webp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct webp_state), &ptr));
    *webp_state = ptr;

    (*webp_state)->read_options  = NULL;
    (*webp_state)->write_options = NULL;

    (*webp_state)->webp_decoder = NULL;

    return SAIL_OK;
}

static void destroy_webp_state(struct webp_state *webp_state) {

    if (webp_state == NULL) {
        return;
    }

    WebPIDelete(webp_state->webp_decoder);
    WebPFreeDecBuffer(&webp_state->webp_config.output);

    sail_destroy_read_options(webp_state->read_options);
    sail_destroy_write_options(webp_state->write_options);

    sail_free(webp_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v5_webp(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    /* Allocate a new state. */
    struct webp_state *webp_state;
    SAIL_TRY(alloc_webp_state(&webp_state));
    *state = webp_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &webp_state->read_options));

    if (WebPInitDecoderConfig(&webp_state->webp_config) == 0) {
        SAIL_LOG_ERROR("WEBP: Failed to initialize WebP configuration");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    webp_state->webp_config.options.use_threads = 1;
    webp_state->webp_config.output.colorspace   = MODE_RGBA;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v5_webp(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_IMAGE_PTR(image);

    struct webp_state *webp_state = (struct webp_state *)state;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

#if 0
    webp_state->webp_config.output.u.RGBA.rgba        = memory_buffer;
    webp_state->webp_config.output.u.RGBA.stride      = scanline_stride;
    webp_state->webp_config.output.u.RGBA.size        = total_size_of_the_memory_buffer;
    webp_state->webp_config.output.is_external_memory = 1;
    webp_state->webp_decoder = WebPINewDecoder(&webp_state->webp_config.output);

    if (webp_state->webp_decoder == NULL) {
        SAIL_LOG_ERROR("WEBP: Failed to initialize WebP decoder");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    while (additional_data_is_available) {
        // ... (get additional data in some new_data[] buffer)
        VP8StatusCode status = WebPIAppend(idec, new_data, new_data_size);

        if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
            break;
        }

        WEBP_EXTERN uint8_t* WebPIDecGetRGB(
            const WebPIDecoder* idec, int* last_y,
            int* width, int* height, int* stride);
    }
#endif

#if 0
    image_local->source_image->pixel_format =
        webp_private_sail_pixel_format(webp_image->yuvFormat, webp_image->depth, webp_image->alphaPlane != NULL);
    image_local->source_image->chroma_subsampling = webp_private_sail_chroma_subsampling(webp_image->yuvFormat);

    image_local->width = webp_image->width;
    image_local->height = webp_image->height;
    image_local->pixel_format = webp_private_rgb_sail_pixel_format(webp_state->rgb_image.format, webp_state->rgb_image.depth);
    image_local->delay = (int)(webp_state->webp_decoder->imageTiming.duration * 1000);

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Fetch ICC profile. */
    if (webp_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(webp_private_fetch_iccp(&webp_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }
#endif

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v5_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v5_webp(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct webp_state *webp_state = (struct webp_state *)state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v5_webp(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct webp_state *webp_state = (struct webp_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_webp_state(webp_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v5_webp(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v5_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v5_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v5_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v5_webp(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
