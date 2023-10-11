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
#include <stddef.h> /* size_t */
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

    struct sail_source_image *source_image;

    bool libjxl_success;
    JxlBasicInfo *basic_info;
    JxlMemoryManager *memory_manager;
    void *runner;
    JxlDecoder *decoder;
    /* For progressive reading. */
    unsigned char *buffer;
    size_t buffer_size;
};

static sail_status_t alloc_jpegxl_state(struct jpegxl_state **jpegxl_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpegxl_state), &ptr));
    *jpegxl_state = ptr;

    (*jpegxl_state)->io           = NULL;
    (*jpegxl_state)->load_options = NULL;
    (*jpegxl_state)->save_options = NULL;

    (*jpegxl_state)->source_image = NULL;

    (*jpegxl_state)->libjxl_success = false;
    (*jpegxl_state)->basic_info     = NULL;
    (*jpegxl_state)->memory_manager = NULL;
    (*jpegxl_state)->runner         = NULL;
    (*jpegxl_state)->decoder        = NULL;
    (*jpegxl_state)->buffer         = NULL;
    (*jpegxl_state)->buffer_size    = 10*1024;

    return SAIL_OK;
}

static void destroy_jpegxl_state(struct jpegxl_state *jpegxl_state) {

    if (jpegxl_state == NULL) {
        return;
    }

    sail_destroy_load_options(jpegxl_state->load_options);
    sail_destroy_save_options(jpegxl_state->save_options);

    sail_destroy_source_image(jpegxl_state->source_image);

    sail_free(jpegxl_state->basic_info);
    sail_free(jpegxl_state->memory_manager);

    JxlResizableParallelRunnerDestroy(jpegxl_state->runner);
    JxlDecoderCloseInput(jpegxl_state->decoder);
    JxlDecoderDestroy(jpegxl_state->decoder);
    sail_free(jpegxl_state->buffer);

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

    SAIL_TRY(sail_malloc(jpegxl_state->buffer_size, &ptr));
    jpegxl_state->buffer = ptr;

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

    size_t bytes_read;
    SAIL_TRY(jpegxl_state->io->tolerant_read(jpegxl_state->io->stream, jpegxl_state->buffer, jpegxl_state->buffer_size, &bytes_read));

    if (JxlDecoderSetInput(jpegxl_state->decoder, jpegxl_state->buffer, bytes_read) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set input buffer");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpegxl(void *state, struct sail_image **image) {

    struct jpegxl_state *jpegxl_state = state;

    if (jpegxl_state->libjxl_success) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    struct sail_image *image_local;

    SAIL_TRY(sail_alloc_image(&image_local));

    struct sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

    for(bool done = false; !done; ) {
        JxlDecoderStatus status = JxlDecoderProcessInput(jpegxl_state->decoder);

        switch (status) {
            case JXL_DEC_ERROR: {
                sail_destroy_image(image_local);
                SAIL_LOG_ERROR("JPEGXL: Decoder error");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
            case JXL_DEC_NEED_MORE_INPUT: {
                SAIL_TRY_OR_CLEANUP(jpegxl_private_read_more_data(jpegxl_state->io,
                                                                     jpegxl_state->decoder,
                                                                     jpegxl_state->buffer,
                                                                     jpegxl_state->buffer_size),
                                    /* cleanup */ sail_destroy_image(image_local));
                break;
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

                SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&jpegxl_state->source_image),
                                    /* cleanup */ sail_destroy_image(image_local));

                /* Special properties. */
                SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&jpegxl_state->source_image->special_properties),
                                    /* cleanup */ sail_destroy_image(image_local));
                SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_special_properties(
                                        jpegxl_state->basic_info,
                                        jpegxl_state->source_image->special_properties),
                                    /* cleanup*/ sail_destroy_image(image_local));

                SAIL_LOG_TRACE("JPEGXL: Animation(%s)", jpegxl_state->basic_info->have_animation ? "yes" : "no");

                if (jpegxl_state->basic_info->have_animation) {
                    SAIL_LOG_TRACE("JPEGXL: Animation parameters: num(%u), denom(%u), loops(%u)",
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

                SAIL_TRY_OR_CLEANUP(sail_copy_source_image(jpegxl_state->source_image, &image_local->source_image),
                                    /* cleanup */ sail_destroy_image(image_local));

                if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA) {
                    if (frame_header.name_length > 0) {
                        SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_name(jpegxl_state->decoder,
                                                                        frame_header.name_length,
                                                                        last_meta_data_node),
                                            /* cleanup*/ sail_destroy_image(image_local));
                        last_meta_data_node = &(*last_meta_data_node)->next;
                    }
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
                    SAIL_LOG_TRACE("JPEGXL: Frame delay(%d) ms.", image_local->delay);
                }
                break;
            }
            case JXL_DEC_COLOR_ENCODING: {
                SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_iccp(jpegxl_state->decoder, &image_local->iccp),
                                    /* cleanup */ sail_destroy_image(image_local));
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

    for(bool done = false; !done; ) {
        status = JxlDecoderProcessInput(jpegxl_state->decoder);

        switch (status) {
            case JXL_DEC_ERROR: {
                SAIL_LOG_ERROR("JPEGXL: Decoder error");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
            case JXL_DEC_NEED_MORE_INPUT: {
                SAIL_TRY(jpegxl_private_read_more_data(jpegxl_state->io,
                                                        jpegxl_state->decoder,
                                                        jpegxl_state->buffer,
                                                        jpegxl_state->buffer_size));
                break;
            }
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
