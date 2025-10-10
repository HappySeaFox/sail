/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct avif_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct avifIO* avif_io;
    struct avifDecoder* avif_decoder;
    struct avifEncoder* avif_encoder;
    struct avifRGBImage rgb_image;
    struct sail_avif_context avif_context;
    struct avifImage* avif_image;
    unsigned frames_saved;
};

static sail_status_t alloc_avif_state(struct sail_io* io,
                                      const struct sail_load_options* load_options,
                                      const struct sail_save_options* save_options,
                                      struct avif_state** avif_state)
{

    void* ptr;

    /* avifIO */
    SAIL_TRY(sail_malloc(sizeof(struct avifIO), &ptr));
    struct avifIO* avif_io = ptr;

    *avif_io = (struct avifIO){
        .destroy    = NULL,
        .read       = avif_private_read_proc,
        .write      = NULL,
        .sizeHint   = 0,
        .persistent = AVIF_FALSE,
        .data       = NULL,
    };

    /* buffer */
    const size_t buffer_size = 8 * 1024;
    void* buffer;
    SAIL_TRY_OR_CLEANUP(sail_malloc(buffer_size, &buffer),
                        /* on error */ sail_free(avif_io));

    /* avif_state */
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct avif_state), &ptr),
                        /* on error */ sail_free(buffer), sail_free(avif_io));
    *avif_state = ptr;

    **avif_state = (struct avif_state){.load_options = load_options,
                                       .save_options = save_options,
                                       .avif_io      = avif_io,
                                       .avif_decoder = avifDecoderCreate(),
                                       .avif_encoder = NULL,
                                       .avif_image   = NULL,
                                       .frames_saved = 0,
                                       .avif_context = (struct sail_avif_context){
                                           .io          = io,
                                           .buffer      = buffer,
                                           .buffer_size = buffer_size,
                                       }};

#if AVIF_VERSION_MAJOR > 0 || AVIF_VERSION_MINOR >= 9
    (*avif_state)->avif_decoder->strictFlags = AVIF_STRICT_DISABLED;
#endif

    avifDecoderSetIO((*avif_state)->avif_decoder, (*avif_state)->avif_io);

    (*avif_state)->avif_io->data = &(*avif_state)->avif_context;

    return SAIL_OK;
}

static void destroy_avif_state(struct avif_state* avif_state)
{

    if (avif_state == NULL)
    {
        return;
    }

    if (avif_state->avif_decoder != NULL)
    {
        avifDecoderDestroy(avif_state->avif_decoder);
    }

    if (avif_state->avif_encoder != NULL)
    {
        avifEncoderDestroy(avif_state->avif_encoder);
    }

    if (avif_state->avif_image != NULL)
    {
        avifImageDestroy(avif_state->avif_image);
    }

    sail_free(avif_state->avif_context.buffer);

    sail_free(avif_state->avif_io);

    sail_free(avif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_avif(struct sail_io* io,
                                                       const struct sail_load_options* load_options,
                                                       void** state)
{

    *state = NULL;

    /* Allocate a new state. */
    struct avif_state* avif_state;
    SAIL_TRY(alloc_avif_state(io, load_options, NULL, &avif_state));
    *state = avif_state;

    avif_state->avif_decoder->ignoreExif = avif_state->avif_decoder->ignoreXMP =
        (avif_state->load_options->options & SAIL_OPTION_META_DATA) == 0;

    /* Handle tuning options. */
    if (load_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(load_options->tuning, avif_private_load_tuning_key_value_callback,
                                              avif_state->avif_decoder);
    }

    /* Initialize AVIF. */
    avifResult avif_result = avifDecoderParse(avif_state->avif_decoder);

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_avif(void* state, struct sail_image** image)
{

    struct avif_state* avif_state = state;

    avifResult avif_result = avifDecoderNextImage(avif_state->avif_decoder);
    if (avif_result == AVIF_RESULT_NO_IMAGES_REMAINING)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    const struct avifImage* avif_image = avif_state->avif_decoder->image;

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    avifRGBImageSetDefaults(&avif_state->rgb_image, avif_image);
    avif_state->rgb_image.depth = avif_private_round_depth(avif_state->rgb_image.depth);

    if (avif_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format =
            avif_private_sail_pixel_format(avif_image->yuvFormat, avif_image->depth, avif_image->alphaPlane != NULL);
        image_local->source_image->chroma_subsampling = avif_private_sail_chroma_subsampling(avif_image->yuvFormat);
        image_local->source_image->compression        = SAIL_COMPRESSION_AV1;
    }

    image_local->width  = avif_image->width;
    image_local->height = avif_image->height;
    image_local->pixel_format =
        avif_private_rgb_sail_pixel_format(avif_state->rgb_image.format, avif_state->rgb_image.depth);
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);
    image_local->delay          = (int)(avif_state->avif_decoder->imageTiming.duration * 1000);

    /* Fetch ICC profile. */
    if (avif_state->load_options->options & SAIL_OPTION_ICCP)
    {
        SAIL_TRY_OR_CLEANUP(avif_private_fetch_iccp(&avif_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    struct sail_meta_data_node** last_meta_data_node = &image_local->meta_data_node;

    if (avif_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        /* Fetch EXIF. */
        SAIL_TRY_OR_CLEANUP(avif_private_fetch_meta_data(SAIL_META_DATA_EXIF, &avif_image->exif, last_meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
        if (*last_meta_data_node != NULL)
        {
            last_meta_data_node = &(*last_meta_data_node)->next;
        }

        /* Fetch XMP. */
        SAIL_TRY_OR_CLEANUP(avif_private_fetch_meta_data(SAIL_META_DATA_XMP, &avif_image->xmp, last_meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
        if (*last_meta_data_node != NULL)
        {
            last_meta_data_node = &(*last_meta_data_node)->next;
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_avif(void* state, struct sail_image* image)
{

    struct avif_state* avif_state      = state;
    const struct avifImage* avif_image = avif_state->avif_decoder->image;

    avif_state->rgb_image.pixels   = image->pixels;
    avif_state->rgb_image.rowBytes = image->bytes_per_line;

    avifResult avif_result = avifImageYUVToRGB(avif_image, &avif_state->rgb_image);

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_avif(void** state)
{

    struct avif_state* avif_state = *state;

    *state = NULL;

    destroy_avif_state(avif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_avif(struct sail_io* io,
                                                       const struct sail_save_options* save_options,
                                                       void** state)
{

    *state = NULL;

    /* Allocate a new state. */
    struct avif_state* avif_state;
    SAIL_TRY(alloc_avif_state(io, NULL, save_options, &avif_state));
    *state = avif_state;

    /* Setup write callback. */
    avif_state->avif_io->write = avif_private_write_proc;

    /* Create encoder. */
    avif_state->avif_encoder = avifEncoderCreate();
    if (avif_state->avif_encoder == NULL)
    {
        SAIL_LOG_ERROR("AVIF: Failed to create encoder");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Set encoder parameters. */
    avif_state->avif_encoder->maxThreads = 1;
    avif_state->avif_encoder->speed      = AVIF_SPEED_DEFAULT;
    avif_state->avif_encoder->timescale  = 1000; /* 1 unit = 1 millisecond. */

    /* Quality settings. */
    if (save_options->compression_level >= 0 && save_options->compression_level <= 100)
    {
        int quality                            = 100 - (int)save_options->compression_level;
        avif_state->avif_encoder->quality      = quality;
        avif_state->avif_encoder->qualityAlpha = quality;
    }
    else
    {
        avif_state->avif_encoder->quality      = AVIF_QUALITY_DEFAULT;
        avif_state->avif_encoder->qualityAlpha = AVIF_QUALITY_DEFAULT;
    }

    /* Compression type check. */
    if (save_options->compression != SAIL_COMPRESSION_UNKNOWN && save_options->compression != SAIL_COMPRESSION_AV1)
    {
        SAIL_LOG_ERROR("AVIF: Only AV1 compression is supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Handle tuning options. */
    if (save_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(save_options->tuning, avif_private_tuning_key_value_callback,
                                              avif_state->avif_encoder);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_avif(void* state, const struct sail_image* image)
{

    struct avif_state* avif_state = state;

    /* Determine pixel format and depth. */
    enum avifRGBFormat rgb_format;
    uint32_t depth;

    if (!avif_private_sail_pixel_format_to_avif_rgb_format(image->pixel_format, &rgb_format, &depth))
    {
        SAIL_LOG_ERROR("AVIF: %s pixel format is not supported for saving",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Create AVIF image for this frame. */
    avif_state->avif_image = avifImageCreate(image->width, image->height, depth, AVIF_PIXEL_FORMAT_YUV444);
    if (avif_state->avif_image == NULL)
    {
        SAIL_LOG_ERROR("AVIF: Failed to create AVIF image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Setup RGB image for conversion. */
    avifRGBImageSetDefaults(&avif_state->rgb_image, avif_state->avif_image);
    avif_state->rgb_image.format = rgb_format;
    avif_state->rgb_image.depth  = depth;

    /* Write ICC profile and meta data only for first frame. */
    if (avif_state->frames_saved == 0)
    {
        if (avif_state->save_options->options & SAIL_OPTION_ICCP)
        {
            SAIL_TRY(avif_private_write_iccp(avif_state->avif_image, image->iccp));
        }

        if (avif_state->save_options->options & SAIL_OPTION_META_DATA)
        {
            SAIL_TRY(
                avif_private_write_meta_data(avif_state->avif_encoder, avif_state->avif_image, image->meta_data_node));
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_avif(void* state, const struct sail_image* image)
{

    struct avif_state* avif_state = state;

    /* Setup pixel data for conversion. */
    avif_state->rgb_image.pixels   = image->pixels;
    avif_state->rgb_image.rowBytes = image->bytes_per_line;

    /* Convert RGB to YUV. */
    avifResult avif_result = avifImageRGBToYUV(avif_state->avif_image, &avif_state->rgb_image);

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: Failed to convert RGB to YUV: %s", avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    uint64_t duration_in_timescales = (image->delay > 0) ? (uint64_t)image->delay : 100;

    /* Add frame to encoder. */
    avifAddImageFlags flags = (avif_state->frames_saved == 0) ? AVIF_ADD_IMAGE_FLAG_NONE : AVIF_ADD_IMAGE_FLAG_NONE;
    avif_result = avifEncoderAddImage(avif_state->avif_encoder, avif_state->avif_image, duration_in_timescales, flags);

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: Failed to add frame #%u to encoder: %s", avif_state->frames_saved,
                       avifResultToString(avif_result));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    avif_state->frames_saved++;

    /* Destroy the image after adding to encoder. */
    avifImageDestroy(avif_state->avif_image);
    avif_state->avif_image = NULL;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_avif(void** state)
{

    struct avif_state* avif_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (avif_state->frames_saved == 0)
    {
        SAIL_LOG_ERROR("AVIF: No frames were added");
        destroy_avif_state(avif_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Finalize encoding. */
    avifRWData output      = AVIF_DATA_EMPTY;
    avifResult avif_result = avifEncoderFinish(avif_state->avif_encoder, &output);

    if (avif_result != AVIF_RESULT_OK)
    {
        SAIL_LOG_ERROR("AVIF: Failed to finish encoding: %s", avifResultToString(avif_result));
        avifRWDataFree(&output);
        destroy_avif_state(avif_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Write encoded data to stream. */
    sail_status_t status =
        avif_state->avif_context.io->strict_write(avif_state->avif_context.io->stream, output.data, output.size);

    avifRWDataFree(&output);

    if (status != SAIL_OK)
    {
        destroy_avif_state(avif_state);
        SAIL_LOG_AND_RETURN(status);
    }

    SAIL_LOG_TRACE("AVIF: Saved %u frame(s)", avif_state->frames_saved);

    destroy_avif_state(avif_state);

    return SAIL_OK;
}
