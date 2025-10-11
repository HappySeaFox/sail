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
#include <jxl/encode.h>
#include <jxl/resizable_parallel_runner.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "memory.h"

/*
 * Codec-specific data types.
 */

static const double COMPRESSION_MIN     = 0;
static const double COMPRESSION_MAX     = 100;
static const double COMPRESSION_DEFAULT = 75;

/*
 * Codec-specific state.
 */
struct jpegxl_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct sail_source_image* source_image;

    bool libjxl_success;
    bool frame_header_seen;
    JxlBasicInfo* basic_info;
    JxlMemoryManager* memory_manager;
    void* runner;
    JxlDecoder* decoder;
    JxlEncoder* encoder;
    JxlEncoderFrameSettings* frame_settings;
    /* For progressive reading. */
    unsigned char* buffer;
    size_t buffer_size;
    bool frame_saved;
    unsigned current_frame;
    bool is_animation;
};

static sail_status_t alloc_jpegxl_state(struct sail_io* io,
                                        const struct sail_load_options* load_options,
                                        const struct sail_save_options* save_options,
                                        struct jpegxl_state** jpegxl_state)
{
    void* ptr;

    /* JxlMemoryManager */
    SAIL_TRY(sail_malloc(sizeof(JxlMemoryManager), &ptr));
    JxlMemoryManager* memory_manager = ptr;

    *memory_manager = (JxlMemoryManager){
        .opaque = NULL,
        .alloc  = jpegxl_private_alloc_func,
        .free   = jpegxl_private_free_func,
    };

    /* buffer */
    const size_t buffer_size = 8192;
    void* buffer;
    SAIL_TRY_OR_CLEANUP(sail_malloc(buffer_size, &buffer),
                        /* on error */ sail_free(memory_manager));

    /* jpegxl_state */
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct jpegxl_state), &ptr),
                        /* on error */ sail_free(buffer), sail_free(memory_manager));
    *jpegxl_state = ptr;

    **jpegxl_state = (struct jpegxl_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .source_image = NULL,

        .libjxl_success    = false,
        .frame_header_seen = false,
        .basic_info        = NULL,
        .memory_manager    = memory_manager,
        .runner            = NULL,
        .decoder           = NULL,
        .encoder           = NULL,
        .frame_settings    = NULL,
        .buffer            = buffer,
        .buffer_size       = buffer_size,
        .frame_saved       = false,
        .current_frame     = 0,
        .is_animation      = false,
    };

    return SAIL_OK;
}

static void destroy_jpegxl_state(struct jpegxl_state* jpegxl_state)
{
    if (jpegxl_state == NULL)
    {
        return;
    }

    sail_destroy_source_image(jpegxl_state->source_image);

    sail_free(jpegxl_state->basic_info);
    sail_free(jpegxl_state->memory_manager);

    JxlResizableParallelRunnerDestroy(jpegxl_state->runner);

    if (jpegxl_state->decoder != NULL)
    {
        JxlDecoderCloseInput(jpegxl_state->decoder);
        JxlDecoderDestroy(jpegxl_state->decoder);
    }

    if (jpegxl_state->encoder != NULL)
    {
        JxlEncoderDestroy(jpegxl_state->encoder);
    }

    sail_free(jpegxl_state->buffer);

    sail_free(jpegxl_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_jpegxl(struct sail_io* io,
                                                         const struct sail_load_options* load_options,
                                                         void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct jpegxl_state* jpegxl_state;
    SAIL_TRY(alloc_jpegxl_state(io, load_options, NULL, &jpegxl_state));
    *state = jpegxl_state;

    /* Init decoder. */
    jpegxl_state->runner  = JxlResizableParallelRunnerCreate(jpegxl_state->memory_manager);
    jpegxl_state->decoder = JxlDecoderCreate(jpegxl_state->memory_manager);

    if (JxlDecoderSetCoalescing(jpegxl_state->decoder, JXL_TRUE) != JXL_DEC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to set coalescing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlDecoderSubscribeEvents(jpegxl_state->decoder, JXL_DEC_BASIC_INFO | JXL_DEC_BOX | JXL_DEC_COLOR_ENCODING
                                                             | JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE)
        != JXL_DEC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to subscribe to decoder events");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlDecoderSetParallelRunner(jpegxl_state->decoder, JxlResizableParallelRunner, jpegxl_state->runner)
        != JXL_DEC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to set parallel runner");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Handle decoder tuning. */
    if (jpegxl_state->load_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(jpegxl_state->load_options->tuning,
                                              jpegxl_private_decoder_tuning_key_value_callback, jpegxl_state->decoder);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpegxl(void* state, struct sail_image** image)
{
    struct jpegxl_state* jpegxl_state = state;

    if (jpegxl_state->libjxl_success)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    struct sail_image* image_local = NULL;
    SAIL_TRY(sail_alloc_image(&image_local));

    struct sail_meta_data_node** last_meta_data_node = &image_local->meta_data_node;

    for (JxlDecoderStatus status                         = jpegxl_state->frame_header_seen ? JXL_DEC_FRAME
                                                                                           : JxlDecoderProcessInput(jpegxl_state->decoder);
         status != JXL_DEC_NEED_IMAGE_OUT_BUFFER; status = JxlDecoderProcessInput(jpegxl_state->decoder))
    {
        switch (status)
        {
        case JXL_DEC_ERROR:
        {
            sail_destroy_image(image_local);
            SAIL_LOG_ERROR("JPEGXL: Decoder error");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        case JXL_DEC_NEED_MORE_INPUT:
        {
            SAIL_TRY_OR_CLEANUP(jpegxl_private_read_more_data(jpegxl_state->io, jpegxl_state->decoder,
                                                              jpegxl_state->buffer, jpegxl_state->buffer_size),
                                /* cleanup */ sail_destroy_image(image_local));
            break;
        }
        case JXL_DEC_BASIC_INFO:
        {
            void* ptr;
            SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(JxlBasicInfo), &ptr),
                                /* cleanup */ sail_destroy_image(image_local));
            jpegxl_state->basic_info = ptr;

            if (JxlDecoderGetBasicInfo(jpegxl_state->decoder, jpegxl_state->basic_info) != JXL_DEC_SUCCESS)
            {
                sail_destroy_image(image_local);
                SAIL_LOG_ERROR("JPEGXL: Failed to get image info");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            /* Source image. */
            SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&jpegxl_state->source_image),
                                /* cleanup */ sail_destroy_image(image_local));

            if (jpegxl_private_is_cmyk(jpegxl_state->decoder, jpegxl_state->basic_info->num_extra_channels))
            {
                jpegxl_state->source_image->pixel_format = jpegxl_private_source_pixel_format_cmyk(
                    jpegxl_state->basic_info->bits_per_sample, jpegxl_state->basic_info->alpha_bits);
            }
            else
            {
                jpegxl_state->source_image->pixel_format = jpegxl_private_source_pixel_format(
                    jpegxl_state->basic_info->bits_per_sample, jpegxl_state->basic_info->num_color_channels,
                    jpegxl_state->basic_info->alpha_bits);
            }

            jpegxl_state->source_image->compression = SAIL_COMPRESSION_UNKNOWN;

            /* Special properties. */
            if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA)
            {
                SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                                    /* cleanup */ sail_destroy_image(image_local));
                SAIL_TRY_OR_CLEANUP(
                    jpegxl_private_fetch_special_properties(jpegxl_state->basic_info, image_local->special_properties),
                    /* cleanup*/ sail_destroy_image(image_local));
            }

            SAIL_LOG_TRACE("JPEGXL: Animation(%s)", jpegxl_state->basic_info->have_animation ? "yes" : "no");

            if (jpegxl_state->basic_info->have_animation)
            {
                SAIL_LOG_TRACE("JPEGXL: Animation parameters: num(%u), denom(%u), loops(%u)",
                               jpegxl_state->basic_info->animation.tps_numerator,
                               jpegxl_state->basic_info->animation.tps_denominator,
                               jpegxl_state->basic_info->animation.num_loops);
            }

            JxlResizableParallelRunnerSetThreads(
                jpegxl_state->runner, JxlResizableParallelRunnerSuggestThreads(jpegxl_state->basic_info->xsize,
                                                                               jpegxl_state->basic_info->ysize));
            break;
        }
        case JXL_DEC_FRAME:
        {
            JxlFrameHeader frame_header;

            if (JxlDecoderGetFrameHeader(jpegxl_state->decoder, &frame_header) != JXL_DEC_SUCCESS)
            {
                sail_destroy_image(image_local);
                SAIL_LOG_ERROR("JPEGXL: Failed to get frame header");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            if (jpegxl_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
            {
                SAIL_TRY_OR_CLEANUP(sail_copy_source_image(jpegxl_state->source_image, &image_local->source_image),
                                    /* cleanup */ sail_destroy_image(image_local));
            }

            if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA)
            {
                if (frame_header.name_length > 0)
                {
                    SAIL_TRY_OR_CLEANUP(
                        jpegxl_private_fetch_name(jpegxl_state->decoder, frame_header.name_length, last_meta_data_node),
                        /* cleanup*/ sail_destroy_image(image_local));
                    last_meta_data_node = &(*last_meta_data_node)->next;
                }
            }

            image_local->width  = jpegxl_state->basic_info->xsize;
            image_local->height = jpegxl_state->basic_info->ysize;
            image_local->pixel_format =
                jpegxl_private_source_pixel_format_to_output(jpegxl_state->source_image->pixel_format);
            image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

            if (jpegxl_state->basic_info->have_animation)
            {
                float ms = frame_header.duration * 1000.f * jpegxl_state->basic_info->animation.tps_denominator
                           / jpegxl_state->basic_info->animation.tps_numerator;
                image_local->delay = (int)ms;
                SAIL_LOG_TRACE("JPEGXL: Frame delay(%d) ms.", image_local->delay);
            }
            break;
        }
        case JXL_DEC_BOX:
        {
            if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA)
            {
                SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_metadata(jpegxl_state->decoder, last_meta_data_node),
                                    /* cleanup*/ sail_destroy_image(image_local));

                if (*last_meta_data_node != NULL)
                {
                    last_meta_data_node = &(*last_meta_data_node)->next;
                }
            }
            break;
        }
        case JXL_DEC_COLOR_ENCODING:
        {
            SAIL_TRY_OR_CLEANUP(jpegxl_private_fetch_iccp(jpegxl_state->decoder, &image_local->iccp),
                                /* cleanup */ sail_destroy_image(image_local));
            break;
        }
        case JXL_DEC_SUCCESS:
        {
            sail_destroy_image(image_local);
            return SAIL_ERROR_NO_MORE_FRAMES;
        }
        default:
        {
            sail_destroy_image(image_local);
            SAIL_LOG_ERROR("JPEGXL: Unexpected decoder status %u", status);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_jpegxl(void* state, struct sail_image* image)
{
    struct jpegxl_state* jpegxl_state = state;

    JxlPixelFormat format = {.num_channels = jpegxl_private_pixel_format_to_num_channels(image->pixel_format),
                             .data_type    = jpegxl_private_pixel_format_to_jxl_data_type(image->pixel_format),
                             .endianness   = JXL_NATIVE_ENDIAN,
                             .align        = 0};

    JxlDecoderStatus status = JxlDecoderSetImageOutBuffer(jpegxl_state->decoder, &format, image->pixels,
                                                          (size_t)image->bytes_per_line * image->height);

    if (status != JXL_DEC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to set output buffer. Error: %u", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    jpegxl_state->frame_header_seen = false;

    struct sail_meta_data_node** last_meta_data_node = &image->meta_data_node;

    for (status = JxlDecoderProcessInput(jpegxl_state->decoder);
         !jpegxl_state->frame_header_seen && !jpegxl_state->libjxl_success;
         status = JxlDecoderProcessInput(jpegxl_state->decoder))
    {
        switch (status)
        {
        case JXL_DEC_ERROR:
        {
            SAIL_LOG_ERROR("JPEGXL: Decoder error");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        case JXL_DEC_NEED_MORE_INPUT:
        {
            SAIL_TRY(jpegxl_private_read_more_data(jpegxl_state->io, jpegxl_state->decoder, jpegxl_state->buffer,
                                                   jpegxl_state->buffer_size));
            break;
        }
        case JXL_DEC_FULL_IMAGE:
        {
            break;
        }
        case JXL_DEC_FRAME:
        {
            jpegxl_state->frame_header_seen = true;
            break;
        }
        case JXL_DEC_BOX:
        {
            if (jpegxl_state->load_options->options & SAIL_OPTION_META_DATA)
            {
                SAIL_TRY(jpegxl_private_fetch_metadata(jpegxl_state->decoder, last_meta_data_node));

                if (*last_meta_data_node != NULL)
                {
                    last_meta_data_node = &(*last_meta_data_node)->next;
                }
            }
            break;
        }
        case JXL_DEC_SUCCESS:
        {
            jpegxl_state->libjxl_success = true;
            break;
        }
        default:
        {
            SAIL_LOG_ERROR("JPEGXL: Unexpected decoder status %u", status);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_jpegxl(void** state)
{
    struct jpegxl_state* jpegxl_state = *state;

    *state = NULL;

    destroy_jpegxl_state(jpegxl_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_jpegxl(struct sail_io* io,
                                                         const struct sail_save_options* save_options,
                                                         void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct jpegxl_state* jpegxl_state;
    SAIL_TRY(alloc_jpegxl_state(io, NULL, save_options, &jpegxl_state));
    *state = jpegxl_state;

    /* Sanity check. */
    if (jpegxl_state->save_options->compression != SAIL_COMPRESSION_JPEG_XL)
    {
        SAIL_LOG_ERROR("JPEGXL: Only JPEG-XL compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Init encoder. */
    jpegxl_state->runner  = JxlResizableParallelRunnerCreate(jpegxl_state->memory_manager);
    jpegxl_state->encoder = JxlEncoderCreate(jpegxl_state->memory_manager);

    if (jpegxl_state->encoder == NULL)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to create encoder");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Use container format. */
    if (JxlEncoderUseContainer(jpegxl_state->encoder, JXL_TRUE) != JXL_ENC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to set use container");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (JxlEncoderSetParallelRunner(jpegxl_state->encoder, JxlResizableParallelRunner, jpegxl_state->runner)
        != JXL_ENC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to set parallel runner");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_jpegxl(void* state, const struct sail_image* image)
{
    struct jpegxl_state* jpegxl_state = state;

    /* Set basic info and color encoding on first frame. */
    if (jpegxl_state->current_frame == 0)
    {
        /* Validate pixel format and convert to JXL format. */
        JxlBasicInfo basic_info;
        JxlPixelFormat pixel_format;

        SAIL_TRY(jpegxl_private_pixel_format_to_jxl_basic_info(image->pixel_format, &basic_info, &pixel_format));

        /* Set image dimensions. */
        basic_info.xsize = image->width;
        basic_info.ysize = image->height;

        /* Check if this is animation. */
        if (image->delay > 0)
        {
            jpegxl_state->is_animation           = true;
            basic_info.have_animation            = JXL_TRUE;
            basic_info.animation.tps_numerator   = 1000;
            basic_info.animation.tps_denominator = 1;
            basic_info.animation.num_loops       = 0;
        }

        if (JxlEncoderSetBasicInfo(jpegxl_state->encoder, &basic_info) != JXL_ENC_SUCCESS)
        {
            SAIL_LOG_ERROR("JPEGXL: Failed to set basic info");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Set ICC profile if provided, otherwise set color encoding to sRGB. */
        if (jpegxl_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL)
        {
            if (JxlEncoderSetICCProfile(jpegxl_state->encoder, image->iccp->data, image->iccp->size) != JXL_ENC_SUCCESS)
            {
                SAIL_LOG_WARNING("JPEGXL: Failed to set ICC profile");
            }
        }
        else
        {
            JxlColorEncoding color_encoding;
            JxlColorEncodingSetToSRGB(&color_encoding, pixel_format.num_channels < 3);

            if (JxlEncoderSetColorEncoding(jpegxl_state->encoder, &color_encoding) != JXL_ENC_SUCCESS)
            {
                SAIL_LOG_ERROR("JPEGXL: Failed to set color encoding");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
        }

        /* Create frame settings. */
        jpegxl_state->frame_settings = JxlEncoderFrameSettingsCreate(jpegxl_state->encoder, NULL);

        /* Set compression quality. */
        const double compression = (jpegxl_state->save_options->compression_level < COMPRESSION_MIN
                                    || jpegxl_state->save_options->compression_level > COMPRESSION_MAX)
                                       ? COMPRESSION_DEFAULT
                                       : jpegxl_state->save_options->compression_level;

        /* Convert compression level (0-100) to distance (0-15, lower is better quality). */
        const float distance = (float)((COMPRESSION_MAX - compression) / COMPRESSION_MAX * 15.0);

        if (JxlEncoderSetFrameDistance(jpegxl_state->frame_settings, distance) != JXL_ENC_SUCCESS)
        {
            SAIL_LOG_ERROR("JPEGXL: Failed to set frame distance");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Handle encoder tuning. */
        if (jpegxl_state->save_options->tuning != NULL)
        {
            sail_traverse_hash_map_with_user_data(jpegxl_state->save_options->tuning,
                                                  jpegxl_private_encoder_tuning_key_value_callback,
                                                  jpegxl_state->frame_settings);
        }
    }
    else
    {
        /* Reuse frame settings for subsequent frames. */
        if (jpegxl_state->frame_settings == NULL)
        {
            jpegxl_state->frame_settings = JxlEncoderFrameSettingsCreate(jpegxl_state->encoder, NULL);
        }
    }

    /* Set frame header for animation. */
    if (jpegxl_state->is_animation && image->delay > 0)
    {
        JxlFrameHeader frame_header;
        JxlEncoderInitFrameHeader(&frame_header);
        frame_header.duration = (uint32_t)image->delay;

        if (JxlEncoderSetFrameHeader(jpegxl_state->frame_settings, &frame_header) != JXL_ENC_SUCCESS)
        {
            SAIL_LOG_ERROR("JPEGXL: Failed to set frame header");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    jpegxl_state->current_frame++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_jpegxl(void* state, const struct sail_image* image)
{
    struct jpegxl_state* jpegxl_state = state;

    /* Get pixel format. */
    JxlBasicInfo basic_info;
    JxlPixelFormat pixel_format;

    SAIL_TRY(jpegxl_private_pixel_format_to_jxl_basic_info(image->pixel_format, &basic_info, &pixel_format));

    /* Add image frame. */
    const size_t buffer_size = (size_t)image->bytes_per_line * image->height;

    if (JxlEncoderAddImageFrame(jpegxl_state->frame_settings, &pixel_format, image->pixels, buffer_size)
        != JXL_ENC_SUCCESS)
    {
        SAIL_LOG_ERROR("JPEGXL: Failed to add image frame");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    jpegxl_state->frame_saved = true;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_jpegxl(void** state)
{
    struct jpegxl_state* jpegxl_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    sail_status_t status = SAIL_OK;

    if (jpegxl_state->frame_saved && jpegxl_state->encoder != NULL)
    {
        /* Close input. */
        JxlEncoderCloseInput(jpegxl_state->encoder);

        /* Write final output. */
        status = jpegxl_private_write_output(jpegxl_state->encoder, jpegxl_state->io, jpegxl_state->buffer,
                                             jpegxl_state->buffer_size);
    }

    destroy_jpegxl_state(jpegxl_state);

    return status;
}
