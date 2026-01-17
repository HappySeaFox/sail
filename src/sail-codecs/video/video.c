/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io_src.h"

/*
 * Codec-specific state.
 */
struct video_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct sail_io* io;
    AVFormatContext* format_ctx;
    AVIOContext* avio_ctx;
    uint8_t* avio_buffer;
    int video_stream_index;
    AVCodecContext* codec_ctx;
    AVFrame* frame;
    AVPacket* packet;
    bool frame_decoded;
    int current_frame_index;
    struct SwsContext* sws_ctx;
    enum AVPixelFormat target_av_pix_fmt;
    AVFrame* converted_frame;
};

static sail_status_t alloc_video_state(const struct sail_load_options* load_options,
                                       const struct sail_save_options* save_options,
                                       struct video_state** video_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct video_state), &ptr));
    *video_state = ptr;

    **video_state = (struct video_state){
        .load_options        = load_options,
        .save_options        = save_options,
        .io                  = NULL,
        .format_ctx          = NULL,
        .avio_ctx            = NULL,
        .avio_buffer         = NULL,
        .video_stream_index  = -1,
        .codec_ctx           = NULL,
        .frame               = NULL,
        .packet              = NULL,
        .frame_decoded       = false,
        .current_frame_index = 0,
        .sws_ctx             = NULL,
        .target_av_pix_fmt   = AV_PIX_FMT_NONE,
        .converted_frame     = NULL,
    };

    return SAIL_OK;
}

static void destroy_video_state(struct video_state* video_state)
{
    if (video_state == NULL)
    {
        return;
    }

    if (video_state->sws_ctx != NULL)
    {
        sws_freeContext(video_state->sws_ctx);
        video_state->sws_ctx = NULL;
    }

    if (video_state->converted_frame != NULL)
    {
        if (video_state->converted_frame->data[0] != NULL)
        {
            av_freep(&video_state->converted_frame->data[0]);
        }
        av_frame_free(&video_state->converted_frame);
    }

    if (video_state->frame != NULL)
    {
        av_frame_free(&video_state->frame);
    }

    if (video_state->packet != NULL)
    {
        av_packet_free(&video_state->packet);
    }

    if (video_state->codec_ctx != NULL)
    {
        avcodec_free_context(&video_state->codec_ctx);
    }

    if (video_state->avio_ctx != NULL)
    {
        if (video_state->avio_ctx->buffer != NULL)
        {
            av_freep(&video_state->avio_ctx->buffer);
        }
        avio_context_free(&video_state->avio_ctx);
    }

    if (video_state->format_ctx != NULL)
    {
        avformat_close_input(&video_state->format_ctx);
    }

    sail_free(video_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_video(struct sail_io* io,
                                                        const struct sail_load_options* load_options,
                                                        void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct video_state* video_state;
    SAIL_TRY(alloc_video_state(load_options, NULL, &video_state));
    *state          = video_state;
    video_state->io = io;

    /* Allocate format context. */
    video_state->format_ctx = avformat_alloc_context();
    if (video_state->format_ctx == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Allocate AVIO buffer. */
    const size_t avio_buffer_size = 4096;
    video_state->avio_buffer      = av_malloc(avio_buffer_size);
    if (video_state->avio_buffer == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Create AVIO context. */
    video_state->avio_ctx = avio_alloc_context(video_state->avio_buffer, (int)avio_buffer_size, 0, io,
                                               video_private_avio_read_packet, NULL, video_private_avio_seek);
    if (video_state->avio_ctx == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    video_state->format_ctx->pb     = video_state->avio_ctx;
    video_state->format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

    /* Open input. */
    int ret = avformat_open_input(&video_state->format_ctx, NULL, NULL, NULL);
    if (ret < 0)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to open input: %s", av_err2str(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Find stream info. */
    ret = avformat_find_stream_info(video_state->format_ctx, NULL);
    if (ret < 0)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to find stream info: %s", av_err2str(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Find video stream. */
    video_state->video_stream_index = -1;
    for (unsigned i = 0; i < video_state->format_ctx->nb_streams; i++)
    {
        if (video_state->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_state->video_stream_index = (int)i;
            break;
        }
    }

    if (video_state->video_stream_index < 0)
    {
        SAIL_LOG_ERROR("VIDEO: No video stream found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    AVStream* video_stream      = video_state->format_ctx->streams[video_state->video_stream_index];
    AVCodecParameters* codecpar = video_stream->codecpar;

    /* Find decoder. */
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (codec == NULL)
    {
        SAIL_LOG_ERROR("VIDEO: Codec not found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Allocate codec context. */
    video_state->codec_ctx = avcodec_alloc_context3(codec);
    if (video_state->codec_ctx == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Copy codec parameters to context. */
    ret = avcodec_parameters_to_context(video_state->codec_ctx, codecpar);
    if (ret < 0)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to copy codec parameters: %s", av_err2str(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Handle tuning options. */
    if (video_state->load_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(video_state->load_options->tuning,
                                              video_private_load_tuning_key_value_callback,
                                              video_state->codec_ctx);
    }

    /* Open codec. */
    ret = avcodec_open2(video_state->codec_ctx, codec, NULL);
    if (ret < 0)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to open codec: %s", av_err2str(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Allocate frame. */
    video_state->frame = av_frame_alloc();
    if (video_state->frame == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Allocate packet. */
    video_state->packet = av_packet_alloc();
    if (video_state->packet == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_video(void* state, struct sail_image** image)
{
    struct video_state* video_state = state;

    /* By default, read first frame only. */
    if (video_state->current_frame_index > 0)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Allocate image. */
    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    /* Allocate source_image. */
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    AVStream* video_stream      = video_state->format_ctx->streams[video_state->video_stream_index];
    AVCodecParameters* codecpar = video_stream->codecpar;

    image_local->width  = codecpar->width;
    image_local->height = codecpar->height;

    /* Get pixel format from codec. */
    enum AVPixelFormat source_av_pix_fmt = (enum AVPixelFormat)codecpar->format;
    enum SailPixelFormat source_sail_fmt = video_private_av_pixel_format_to_sail(source_av_pix_fmt);

    video_state->target_av_pix_fmt = video_private_find_best_sail_pixel_format(source_av_pix_fmt);
    image_local->pixel_format      = video_private_av_pixel_format_to_sail(video_state->target_av_pix_fmt);

    if (image_local->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to find compatible pixel format for: %d", source_av_pix_fmt);
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* If format conversion is needed, swscale will handle it in load_frame. */
    if (source_av_pix_fmt != video_state->target_av_pix_fmt || source_sail_fmt != image_local->pixel_format)
    {
        SAIL_LOG_DEBUG("VIDEO: Converting pixel format from %d (%d) to %d (%d)", source_av_pix_fmt,
                       (int)source_sail_fmt, video_state->target_av_pix_fmt, (int)image_local->pixel_format);
    }

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Set source_image properties. */
    image_local->source_image->pixel_format = source_av_pix_fmt != AV_PIX_FMT_NONE
                                                  ? video_private_av_pixel_format_to_sail(source_av_pix_fmt)
                                                  : SAIL_PIXEL_FORMAT_UNKNOWN;
    image_local->source_image->compression  = SAIL_COMPRESSION_UNKNOWN;

    /* Fetch specialized properties. */
    if (video_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        SAIL_TRY(sail_alloc_hash_map(&image_local->source_image->special_properties));
        SAIL_TRY(video_private_fetch_special_properties(video_state->format_ctx, video_stream,
                                                        image_local->source_image->special_properties));
    }

    /* Fetch resolution. */
    AVRational sar = video_stream->sample_aspect_ratio;
    if (sar.num > 0 && sar.den > 0)
    {
        SAIL_TRY(sail_alloc_resolution(&image_local->resolution));
        image_local->resolution->unit = SAIL_RESOLUTION_UNIT_INCH;
        image_local->resolution->x    = (double)codecpar->width * sar.num / sar.den;
        image_local->resolution->y    = (double)codecpar->height;
    }

    /* Fetch metadata. */
    if (video_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        struct sail_meta_data_node** last_meta_data_node = &image_local->meta_data_node;
        SAIL_TRY(video_private_fetch_meta_data(video_state->format_ctx, video_stream, &last_meta_data_node));
    }

    video_state->current_frame_index++;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_video(void* state, struct sail_image* image)
{
    struct video_state* video_state = state;

    /* Decode first frame. */
    if (!video_state->frame_decoded)
    {
        int ret;

        /* Read frames until we get a video frame. */
        while ((ret = av_read_frame(video_state->format_ctx, video_state->packet)) >= 0)
        {
            if (video_state->packet->stream_index == video_state->video_stream_index)
            {
                /* Send packet to decoder. */
                ret = avcodec_send_packet(video_state->codec_ctx, video_state->packet);
                if (ret < 0)
                {
                    av_packet_unref(video_state->packet);
                    SAIL_LOG_ERROR("VIDEO: Error sending packet: %s", av_err2str(ret));
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                /* Receive frames from decoder (may need multiple calls). */
                while (true)
                {
                    ret = avcodec_receive_frame(video_state->codec_ctx, video_state->frame);
                    if (ret == 0)
                    {
                        video_state->frame_decoded = true;
                        av_packet_unref(video_state->packet);
                        break;
                    }
                    else if (ret == AVERROR(EAGAIN))
                    {
                        break;
                    }
                    else if (ret == AVERROR_EOF)
                    {
                        SAIL_LOG_ERROR("VIDEO: Got EOF");
                        break;
                    }
                    else
                    {
                        av_packet_unref(video_state->packet);
                        SAIL_LOG_ERROR("VIDEO: Error receiving frame: %s", av_err2str(ret));
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                    }
                }

                if (video_state->frame_decoded)
                {
                    break;
                }
            }

            av_packet_unref(video_state->packet);
        }

        /* If we haven't decoded a frame yet, try flushing the decoder. */
        if (!video_state->frame_decoded)
        {
            ret = avcodec_send_packet(video_state->codec_ctx, NULL);
            if (ret < 0 && ret != AVERROR_EOF)
            {
                SAIL_LOG_ERROR("VIDEO: Error flushing decoder: %s", av_err2str(ret));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            while (true)
            {
                ret = avcodec_receive_frame(video_state->codec_ctx, video_state->frame);
                if (ret == 0)
                {
                    video_state->frame_decoded = true;
                    break;
                }
                else if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                {
                    break;
                }
                else
                {
                    SAIL_LOG_ERROR("VIDEO: Error receiving frame during flush: %s", av_err2str(ret));
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }
            }

            if (!video_state->frame_decoded)
            {
                SAIL_LOG_ERROR("VIDEO: Failed to decode frame");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
        }
    }

    /* Get source pixel format from decoded frame. */
    enum AVPixelFormat source_av_pix_fmt = video_state->codec_ctx->pix_fmt;

    /* Determine target format from image pixel format. */
    enum AVPixelFormat target_av_pix_fmt = video_private_sail_pixel_format_to_av(image->pixel_format);
    if (target_av_pix_fmt == AV_PIX_FMT_NONE)
    {
        SAIL_LOG_ERROR("VIDEO: Invalid pixel format in image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    int ret;

    /* If pixel format conversion is needed, use swscale. */
    if (source_av_pix_fmt != target_av_pix_fmt)
    {
        /* Allocate converted frame if not already allocated. */
        if (video_state->converted_frame == NULL)
        {
            video_state->converted_frame = av_frame_alloc();
            if (video_state->converted_frame == NULL)
            {
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
            }
        }

        /* Initialize or recreate swscale context if needed. */
        if (video_state->sws_ctx == NULL || video_state->frame->width != video_state->converted_frame->width
            || video_state->frame->height != video_state->converted_frame->height)
        {
            if (video_state->sws_ctx != NULL)
            {
                sws_freeContext(video_state->sws_ctx);
            }

            video_state->sws_ctx =
                sws_getContext(video_state->frame->width, video_state->frame->height, source_av_pix_fmt, image->width,
                               image->height, target_av_pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);

            if (video_state->sws_ctx == NULL)
            {
                SAIL_LOG_ERROR("VIDEO: Failed to create swscale context");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            /* Allocate buffer for converted frame. */
            ret = av_image_alloc(video_state->converted_frame->data, video_state->converted_frame->linesize,
                                 image->width, image->height, target_av_pix_fmt, 32);

            if (ret < 0)
            {
                SAIL_LOG_ERROR("VIDEO: Failed to allocate converted frame buffer: %s", av_err2str(ret));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
            }

            video_state->converted_frame->width  = image->width;
            video_state->converted_frame->height = image->height;
            video_state->converted_frame->format = target_av_pix_fmt;
        }

        /* Perform pixel format conversion. */
        ret = sws_scale(video_state->sws_ctx, (const uint8_t* const*)video_state->frame->data,
                        video_state->frame->linesize, 0, video_state->frame->height, video_state->converted_frame->data,
                        video_state->converted_frame->linesize);

        if (ret < 0)
        {
            SAIL_LOG_ERROR("VIDEO: Failed to convert pixel format: %s", av_err2str(ret));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Copy converted frame data to image buffer. */
        ret = av_image_copy_to_buffer((uint8_t*)image->pixels, (int)(image->bytes_per_line * image->height),
                                      (const uint8_t* const*)video_state->converted_frame->data,
                                      video_state->converted_frame->linesize, target_av_pix_fmt, image->width,
                                      image->height, 1);
    }
    else
    {
        /* No conversion needed, copy directly. */
        ret = av_image_copy_to_buffer((uint8_t*)image->pixels, (int)(image->bytes_per_line * image->height),
                                      (const uint8_t* const*)video_state->frame->data, video_state->frame->linesize,
                                      source_av_pix_fmt, image->width, image->height, 1);
    }

    if (ret < 0)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to copy frame data: %s", av_err2str(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_video(void** state)
{
    struct video_state* video_state = *state;

    *state = NULL;

    destroy_video_state(video_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_video(struct sail_io* io,
                                                        const struct sail_save_options* save_options,
                                                        void** state)
{
    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_video(void* state, const struct sail_image* image)
{
    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_video(void* state, const struct sail_image* image)
{
    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_video(void** state)
{
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
