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

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "memory.h"

/*
 * Codec-specific state.
 */
struct jpegxl_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    struct sail_source_image *source_image;

    bool libjxl_success;
    bool frame_header_seen;
    JxlBasicInfo *basic_info;
    JxlMemoryManager *memory_manager;
    void *runner;
    JxlDecoder *decoder;
    /* For progressive reading. */
    unsigned char *buffer;
    size_t buffer_size;
};

static sail_status_t alloc_jpegxl_state(struct sail_io *io,
                                        const struct sail_load_options *load_options,
                                        const struct sail_save_options *save_options,
                                        struct jpegxl_state **jpegxl_state) {

    void *ptr;

    /* JxlMemoryManager */
    SAIL_TRY(sail_malloc(sizeof(JxlMemoryManager), &ptr));
    JxlMemoryManager *memory_manager = ptr;

    *memory_manager = (JxlMemoryManager) {
        .opaque = NULL,
        .alloc  = jpegxl_private_alloc_func,
        .free   = jpegxl_private_free_func,
    };

    /* buffer */
    const size_t buffer_size = 8192;
    void *buffer;
    SAIL_TRY_OR_CLEANUP(sail_malloc(buffer_size, &buffer),
                        /* on error */ sail_free(memory_manager));

    /* jpegxl_state */
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct jpegxl_state), &ptr),
                        /* on error */ sail_free(buffer), sail_free(memory_manager));
    *jpegxl_state = ptr;

    **jpegxl_state = (struct jpegxl_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .source_image      = NULL,

        .libjxl_success    = false,
        .frame_header_seen = false,
        .basic_info        = NULL,
        .memory_manager    = memory_manager,
        .runner            = NULL,
        .decoder           = NULL,
        .buffer            = buffer,
        .buffer_size       = buffer_size,
    };

    return SAIL_OK;
}

static void destroy_jpegxl_state(struct jpegxl_state *jpegxl_state) {

    if (jpegxl_state == NULL) {
        return;
    }

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
    SAIL_TRY(alloc_jpegxl_state(io, load_options, NULL, &jpegxl_state));
    *state = jpegxl_state;

    /* Init decoder. */
    jpegxl_state->runner  = JxlResizableParallelRunnerCreate(jpegxl_state->memory_manager);
    jpegxl_state->decoder = JxlDecoderCreate(jpegxl_state->memory_manager);

    if (JxlDecoderSetCoalescing(jpegxl_state->decoder, JXL_TRUE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set coalescing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlDecoderSubscribeEvents(jpegxl_state->decoder, JXL_DEC_BASIC_INFO
                                                            | JXL_DEC_BOX
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

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpegxl(void *state, struct sail_image **image) {

    struct jpegxl_state *jpegxl_state = state;

    if (jpegxl_state->libjxl_success) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    struct sail_image *image_local = NULL;
    SAIL_TRY(sail_alloc_image(&image_local));

    struct sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

    for (JxlDecoderStatus status = jpegxl_state->frame_header_seen
                                    ? JXL_DEC_FRAME
                                    : JxlDecoderProcessInput(jpegxl_state->decoder);
            status != JXL_DEC_NEED_IMAGE_OUT_BUFFER;
            status = JxlDecoderProcessInput(jpegxl_state->decoder)) {
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

                /* Source image. */
                SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&jpegxl_state->source_image),
                                    /* cleanup */ sail_destroy_image(image_local));

                if (jpegxl_private_is_cmyk(jpegxl_state->decoder, jpegxl_state->basic_info->num_extra_channels)) {
                    jpegxl_state->source_image->pixel_format =
                        jpegxl_private_source_pixel_format_cmyk(
                            jpegxl_state->basic_info->bits_per_sample, jpegxl_state->basic_info->alpha_bits);
                } else {
                    jpegxl_state->source_image->pixel_format =
                        jpegxl_private_source_pixel_format(jpegxl_state->basic_info->bits_per_sample,
                                                            jpegxl_state->basic_info->num_color_channels,
                                                            jpegxl_state->basic_info->alpha_bits);
                }

                jpegxl_state->source_image->compression = SAIL_COMPRESSION_UNKNOWN;

                /* Special properties. */
                if (jpegxl_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
                    if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA) {
                        SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&jpegxl_state->source_image->special_properties),
                                            /* cleanup */ sail_destroy_image(image_local));
                        SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_special_properties(
                                                jpegxl_state->basic_info,
                                                jpegxl_state->source_image->special_properties),
                                            /* cleanup*/ sail_destroy_image(image_local));
                    }
                }

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

                if (jpegxl_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
                    SAIL_TRY_OR_CLEANUP(sail_copy_source_image(jpegxl_state->source_image, &image_local->source_image),
                                        /* cleanup */ sail_destroy_image(image_local));
                }

                if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA) {
                    if (frame_header.name_length > 0) {
                        SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_name(jpegxl_state->decoder,
                                                                        frame_header.name_length,
                                                                        last_meta_data_node),
                                            /* cleanup*/ sail_destroy_image(image_local));
                        last_meta_data_node = &(*last_meta_data_node)->next;
                    }
                }

                image_local->width          = jpegxl_state->basic_info->xsize;
                image_local->height         = jpegxl_state->basic_info->ysize;
                image_local->pixel_format   = jpegxl_private_source_pixel_format_to_output(jpegxl_state->source_image->pixel_format);
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
            case JXL_DEC_BOX: {
                if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA) {
                    SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_metadata(jpegxl_state->decoder,
                                                                        last_meta_data_node),
                                        /* cleanup*/ sail_destroy_image(image_local));

                    if (*last_meta_data_node != NULL) {
                        last_meta_data_node = &(*last_meta_data_node)->next;
                    }
                }
                break;
            }
            case JXL_DEC_COLOR_ENCODING: {
                SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_iccp(jpegxl_state->decoder, &image_local->iccp),
                                    /* cleanup */ sail_destroy_image(image_local));
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
        .num_channels = jpegxl_private_pixel_format_to_num_channels(image->pixel_format),
        .data_type    = jpegxl_private_pixel_format_to_jxl_data_type(image->pixel_format),
        .endianness   = JXL_NATIVE_ENDIAN,
        .align        = 0
    };

    JxlDecoderStatus status = JxlDecoderSetImageOutBuffer(
            jpegxl_state->decoder,
            &format,
            image->pixels,
            (size_t)image->bytes_per_line * image->height);

    if (status != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set output buffer. Error: %u", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    jpegxl_state->frame_header_seen = false;

    struct sail_meta_data_node **last_meta_data_node = &image->meta_data_node;

    for (status = JxlDecoderProcessInput(jpegxl_state->decoder);
            !jpegxl_state->frame_header_seen && !jpegxl_state->libjxl_success;
            status = JxlDecoderProcessInput(jpegxl_state->decoder)) {
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
                break;
            }
            case JXL_DEC_FRAME: {
                jpegxl_state->frame_header_seen = true;
                break;
            }
            case JXL_DEC_BOX: {
                if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA) {
                    SAIL_TRY(jpegxl_private_fetch_metadata(jpegxl_state->decoder, last_meta_data_node));

                    if (*last_meta_data_node != NULL) {
                        last_meta_data_node = &(*last_meta_data_node)->next;
                    }
                }
                break;
            }
            case JXL_DEC_SUCCESS: {
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
