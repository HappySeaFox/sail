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
#include <jxl/resizable_parallel_runner.h>

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

    void *image_data;

    bool libjxl_success;
    JxlBasicInfo *basic_info;
    JxlMemoryManager *memory_manager;
    void *runner;
    JxlDecoder *decoder;
};

static sail_status_t alloc_jpegxl_state(struct jpegxl_state **jpegxl_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpegxl_state), &ptr));
    *jpegxl_state = ptr;

    (*jpegxl_state)->io           = NULL;
    (*jpegxl_state)->load_options = NULL;
    (*jpegxl_state)->save_options = NULL;

    (*jpegxl_state)->image_data     = NULL;

    (*jpegxl_state)->libjxl_success = false;
    (*jpegxl_state)->basic_info     = NULL;
    (*jpegxl_state)->memory_manager = NULL;
    (*jpegxl_state)->runner         = NULL;
    (*jpegxl_state)->decoder        = NULL;

    return SAIL_OK;
}

static void destroy_jpegxl_state(struct jpegxl_state *jpegxl_state) {

    if (jpegxl_state == NULL) {
        return;
    }

    sail_destroy_load_options(jpegxl_state->load_options);
    sail_destroy_save_options(jpegxl_state->save_options);

    sail_free(jpegxl_state->image_data);
    sail_free(jpegxl_state->basic_info);
    sail_free(jpegxl_state->memory_manager);

    JxlResizableParallelRunnerDestroy(jpegxl_state->runner);
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

    jpegxl_state->runner  = JxlResizableParallelRunnerCreate(jpegxl_state->memory_manager);
    jpegxl_state->decoder = JxlDecoderCreate(jpegxl_state->memory_manager);

    if (JxlDecoderSetCoalescing(jpegxl_state->decoder, JXL_TRUE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set coalescing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlDecoderSubscribeEvents(jpegxl_state->decoder, JXL_DEC_BASIC_INFO
                                                            | JXL_DEC_COLOR_ENCODING
                                                            | JXL_DEC_FRAME
                                                            | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to subscribe to decoder events");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlDecoderSetParallelRunner(jpegxl_state->decoder,
                                    JxlResizableParallelRunner,
                                    jpegxl_state->runner) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set parallel runner");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    // TODO Progressive
    /* Read the entire image to use the libjxl memory API. */
    size_t image_size;
    SAIL_TRY(sail_alloc_data_from_io_contents(io, &jpegxl_state->image_data, &image_size));

    JxlDecoderSetInput(jpegxl_state->decoder, jpegxl_state->image_data, image_size);
    JxlDecoderCloseInput(jpegxl_state->decoder);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpegxl(void *state, struct sail_image **image) {

    struct jpegxl_state *jpegxl_state = state;

    if (jpegxl_state->libjxl_success) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    struct sail_image *image_local;

    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    bool done = false;

    while (!done) {
        JxlDecoderStatus status = JxlDecoderProcessInput(jpegxl_state->decoder);

        switch (status) {
            case JXL_DEC_NEED_MORE_INPUT: {
                sail_destroy_image(image_local);
                SAIL_LOG_ERROR("JPEGXL: For unknown reason decoder still needs more input");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
            case JXL_DEC_BASIC_INFO: {
                void *ptr;
                SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(JxlBasicInfo), &ptr),
                                        /* cleanup */ sail_destroy_image(image_local));
                jpegxl_state->basic_info = ptr;

                if (JxlDecoderGetBasicInfo(jpegxl_state->decoder, jpegxl_state->basic_info) != JXL_DEC_SUCCESS) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_ERROR("JPEGXL: Failed to get image info");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                SAIL_LOG_TRACE("JPEGXL: Animation: %s", jpegxl_state->basic_info->have_animation ? "yes" : "no");

                if (jpegxl_state->basic_info->have_animation) {
                    SAIL_LOG_TRACE("JPEGXL: num: %u, denom: %u, loops: %u",
                        jpegxl_state->basic_info->animation.tps_numerator, jpegxl_state->basic_info->animation.tps_denominator,
                        jpegxl_state->basic_info->animation.num_loops);
                }

                JxlResizableParallelRunnerSetThreads(
                        jpegxl_state->runner,
                        JxlResizableParallelRunnerSuggestThreads(jpegxl_state->basic_info->xsize, jpegxl_state->basic_info->ysize));
                break;
            }
            case JXL_DEC_FRAME: {
                JxlFrameHeader frame_header;

                if (JxlDecoderGetFrameHeader(jpegxl_state->decoder, &frame_header) != JXL_DEC_SUCCESS) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_ERROR("JPEGXL: Failed to get frame header");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                image_local->source_image->pixel_format =
                    jpegxl_private_sail_pixel_format(jpegxl_state->basic_info->bits_per_sample,
                                                        jpegxl_state->basic_info->num_color_channels,
                                                        jpegxl_state->basic_info->alpha_bits);
                image_local->source_image->compression = SAIL_COMPRESSION_UNKNOWN;

                image_local->width          = jpegxl_state->basic_info->xsize;
                image_local->height         = jpegxl_state->basic_info->ysize;
                image_local->pixel_format   = image_local->source_image->pixel_format;
                image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

                if (jpegxl_state->basic_info->have_animation) {
                    float ms = frame_header.duration
                                * 1000.f
                                * jpegxl_state->basic_info->animation.tps_denominator
                                / jpegxl_state->basic_info->animation.tps_numerator;
                    image_local->delay = (int)ms;
                    SAIL_LOG_TRACE("JPEGXL: Frame delay: %d ms.", image_local->delay);
                }
                break;
            }
            case JXL_DEC_COLOR_ENCODING: {
                size_t icc_size;
                if (JxlDecoderGetICCProfileSize(jpegxl_state->decoder,
                                                /* unused */ NULL,
                                                JXL_COLOR_PROFILE_TARGET_DATA,
                                                &icc_size) != JXL_DEC_SUCCESS) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_ERROR("JPEGXL: Failed to get ICC size");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_for_data((unsigned)icc_size, &image_local->iccp),
                                    /* cleanup */ sail_destroy_image(image_local));

                if (JxlDecoderGetColorAsICCProfile(jpegxl_state->decoder,
                                                    /* unused */ NULL,
                                                    JXL_COLOR_PROFILE_TARGET_DATA,
                                                    image_local->iccp->data,
                                                    image_local->iccp->data_length) != JXL_DEC_SUCCESS) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_ERROR("JPEGXL: Failed to get ICC profile");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }
                break;
            }
            case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
                done = true;
                break;
            }
            case JXL_DEC_SUCCESS: {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
            }
            default: {
                sail_destroy_image(image_local);
                SAIL_LOG_ERROR("JPEGXL: Unexpected decoder status %u", status);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
        }
    }

#if 0
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

    struct jpegxl_state *jpegxl_state = state;

    JxlPixelFormat format = {
        .num_channels = jpegxl_private_sail_pixel_format_to_num_channels(image->pixel_format),
        .data_type    = jpegxl_private_sail_pixel_format_to_jxl_data_type(image->pixel_format),
        .endianness   = JXL_NATIVE_ENDIAN,
        .align        = 0
    };

    JxlDecoderStatus status = JxlDecoderSetImageOutBuffer(
            jpegxl_state->decoder,
            &format,
            image->pixels,
            image->bytes_per_line * image->height);
    if (status != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set output buffer. Error: %u", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    bool done = false;

    while (!done) {
        status = JxlDecoderProcessInput(jpegxl_state->decoder);

        switch (status) {
            case JXL_DEC_FULL_IMAGE: {
                done = true;
                break;
            }
            case JXL_DEC_SUCCESS: {
                done = true;
                jpegxl_state->libjxl_success = true;
                break;
            }
            default: {
                SAIL_LOG_ERROR("JPEGXL: Unexpected decoder status %u", status);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
        }
    }

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
