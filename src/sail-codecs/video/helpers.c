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

#include <libavcodec/avcodec.h>
#include <libavcodec/defs.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

enum SailPixelFormat video_private_av_pixel_format_to_sail(enum AVPixelFormat av_pix_fmt)
{
    switch (av_pix_fmt)
    {
    /* Grayscale formats */
    case AV_PIX_FMT_GRAY8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
    case AV_PIX_FMT_YA8: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
    case AV_PIX_FMT_YA16BE:
    case AV_PIX_FMT_YA16LE: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;

    /* RGB formats */
    case AV_PIX_FMT_RGB24: return SAIL_PIXEL_FORMAT_BPP24_RGB;
    case AV_PIX_FMT_BGR24: return SAIL_PIXEL_FORMAT_BPP24_BGR;
    case AV_PIX_FMT_RGB48BE:
    case AV_PIX_FMT_RGB48LE: return SAIL_PIXEL_FORMAT_BPP48_RGB;
    case AV_PIX_FMT_BGR48BE:
    case AV_PIX_FMT_BGR48LE: return SAIL_PIXEL_FORMAT_BPP48_BGR;

    /* RGBA formats */
    case AV_PIX_FMT_RGBA: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
    case AV_PIX_FMT_BGRA: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
    case AV_PIX_FMT_ARGB: return SAIL_PIXEL_FORMAT_BPP32_ARGB;
    case AV_PIX_FMT_ABGR: return SAIL_PIXEL_FORMAT_BPP32_ABGR;
    case AV_PIX_FMT_RGBA64BE:
    case AV_PIX_FMT_RGBA64LE: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
    case AV_PIX_FMT_BGRA64BE:
    case AV_PIX_FMT_BGRA64LE: return SAIL_PIXEL_FORMAT_BPP64_BGRA;

    /* RGBX formats */
    case AV_PIX_FMT_RGB0: return SAIL_PIXEL_FORMAT_BPP32_RGBX;
    case AV_PIX_FMT_BGR0: return SAIL_PIXEL_FORMAT_BPP32_BGRX;
    case AV_PIX_FMT_0RGB: return SAIL_PIXEL_FORMAT_BPP32_XRGB;
    case AV_PIX_FMT_0BGR: return SAIL_PIXEL_FORMAT_BPP32_XBGR;

    /* YUV formats - 8-bit */
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_YUV410P:
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
    case AV_PIX_FMT_UYVY422:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_YVYU422: return SAIL_PIXEL_FORMAT_BPP24_YUV;

    /* YUV formats 10-bit */
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV440P10BE:
    case AV_PIX_FMT_YUV440P10LE: return SAIL_PIXEL_FORMAT_BPP30_YUV;

    /* YUV formats 12-bit */
    case AV_PIX_FMT_YUV420P12BE:
    case AV_PIX_FMT_YUV420P12LE:
    case AV_PIX_FMT_YUV422P12BE:
    case AV_PIX_FMT_YUV422P12LE:
    case AV_PIX_FMT_YUV444P12BE:
    case AV_PIX_FMT_YUV444P12LE:
    case AV_PIX_FMT_YUV440P12BE:
    case AV_PIX_FMT_YUV440P12LE: return SAIL_PIXEL_FORMAT_BPP36_YUV;

    /* YUV formats 16-bit */
    case AV_PIX_FMT_YUV420P16BE:
    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV422P16BE:
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV444P16BE:
    case AV_PIX_FMT_YUV444P16LE: return SAIL_PIXEL_FORMAT_BPP48_YUV;

    /* YUVA formats 8-bit */
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P: return SAIL_PIXEL_FORMAT_BPP32_YUVA;

    /* YUVA formats 10-bit */
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P10LE:
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P10LE:
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P10LE: return SAIL_PIXEL_FORMAT_BPP40_YUVA;

    /* YUVA formats 12-bit */
    case AV_PIX_FMT_YUVA422P12BE:
    case AV_PIX_FMT_YUVA422P12LE:
    case AV_PIX_FMT_YUVA444P12BE:
    case AV_PIX_FMT_YUVA444P12LE: return SAIL_PIXEL_FORMAT_BPP48_YUVA;

    /* YUVA formats 16-bit */
    case AV_PIX_FMT_YUVA420P16BE:
    case AV_PIX_FMT_YUVA420P16LE:
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUVA422P16LE:
    case AV_PIX_FMT_YUVA444P16BE:
    case AV_PIX_FMT_YUVA444P16LE: return SAIL_PIXEL_FORMAT_BPP64_YUVA;

    /* Indexed formats */
    case AV_PIX_FMT_PAL8: return SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

enum AVPixelFormat video_private_sail_pixel_format_to_av(enum SailPixelFormat sail_pix_fmt)
{
    switch (sail_pix_fmt)
    {
    case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: return AV_PIX_FMT_GRAY8;
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: return AV_PIX_FMT_GRAY16LE;
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: return AV_PIX_FMT_YA8;
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: return AV_PIX_FMT_YA16LE;

    case SAIL_PIXEL_FORMAT_BPP24_RGB: return AV_PIX_FMT_RGB24;
    case SAIL_PIXEL_FORMAT_BPP24_BGR: return AV_PIX_FMT_BGR24;
    case SAIL_PIXEL_FORMAT_BPP48_RGB: return AV_PIX_FMT_RGB48LE;
    case SAIL_PIXEL_FORMAT_BPP48_BGR: return AV_PIX_FMT_BGR48LE;

    case SAIL_PIXEL_FORMAT_BPP32_RGBA: return AV_PIX_FMT_RGBA;
    case SAIL_PIXEL_FORMAT_BPP32_BGRA: return AV_PIX_FMT_BGRA;
    case SAIL_PIXEL_FORMAT_BPP32_ARGB: return AV_PIX_FMT_ARGB;
    case SAIL_PIXEL_FORMAT_BPP32_ABGR: return AV_PIX_FMT_ABGR;
    case SAIL_PIXEL_FORMAT_BPP64_RGBA: return AV_PIX_FMT_RGBA64LE;
    case SAIL_PIXEL_FORMAT_BPP64_BGRA: return AV_PIX_FMT_BGRA64LE;

    case SAIL_PIXEL_FORMAT_BPP32_RGBX: return AV_PIX_FMT_RGB0;
    case SAIL_PIXEL_FORMAT_BPP32_BGRX: return AV_PIX_FMT_BGR0;
    case SAIL_PIXEL_FORMAT_BPP32_XRGB: return AV_PIX_FMT_0RGB;
    case SAIL_PIXEL_FORMAT_BPP32_XBGR: return AV_PIX_FMT_0BGR;

    case SAIL_PIXEL_FORMAT_BPP24_YUV: return AV_PIX_FMT_YUV420P;
    case SAIL_PIXEL_FORMAT_BPP30_YUV: return AV_PIX_FMT_YUV420P10LE;
    case SAIL_PIXEL_FORMAT_BPP36_YUV: return AV_PIX_FMT_YUV420P12LE;
    case SAIL_PIXEL_FORMAT_BPP48_YUV: return AV_PIX_FMT_YUV420P16LE;

    case SAIL_PIXEL_FORMAT_BPP32_YUVA: return AV_PIX_FMT_YUVA420P;
    case SAIL_PIXEL_FORMAT_BPP40_YUVA: return AV_PIX_FMT_YUVA420P10LE;
    case SAIL_PIXEL_FORMAT_BPP48_YUVA: return AV_PIX_FMT_YUVA422P12LE;
    case SAIL_PIXEL_FORMAT_BPP64_YUVA: return AV_PIX_FMT_YUVA420P16LE;

    case SAIL_PIXEL_FORMAT_BPP8_INDEXED: return AV_PIX_FMT_PAL8;

    default: return AV_PIX_FMT_NONE;
    }
}

enum AVPixelFormat video_private_find_best_sail_pixel_format(enum AVPixelFormat source_pix_fmt)
{
    enum AVPixelFormat sail_formats[] = {
        /* RGB formats. */
        AV_PIX_FMT_RGB24,
        AV_PIX_FMT_BGR24,
        AV_PIX_FMT_RGB48LE,
        AV_PIX_FMT_BGR48LE,

        /* RGBA formats. */
        AV_PIX_FMT_RGBA,
        AV_PIX_FMT_BGRA,
        AV_PIX_FMT_ARGB,
        AV_PIX_FMT_ABGR,
        AV_PIX_FMT_RGBA64LE,
        AV_PIX_FMT_BGRA64LE,

        /* RGBX formats. */
        AV_PIX_FMT_RGB0,
        AV_PIX_FMT_BGR0,
        AV_PIX_FMT_0RGB,
        AV_PIX_FMT_0BGR,

        /* Grayscale formats. */
        AV_PIX_FMT_GRAY8,
        AV_PIX_FMT_GRAY16LE,
        AV_PIX_FMT_YA8,
        AV_PIX_FMT_YA16LE,

        /* YUV formats. */
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_YUV420P10LE,
        AV_PIX_FMT_YUV420P12LE,
        AV_PIX_FMT_YUV420P16LE,
        AV_PIX_FMT_YUVA420P,
        AV_PIX_FMT_YUVA420P10LE,
        AV_PIX_FMT_YUVA422P12LE,
        AV_PIX_FMT_YUVA420P16LE,

        /* Indexed formats. */
        AV_PIX_FMT_PAL8,

        /* Terminator for avcodec_find_best_pix_fmt_of_list(). */
        AV_PIX_FMT_NONE,
    };

    /* If the source format is already supported and supported, return it. */
    enum SailPixelFormat direct_format = video_private_av_pixel_format_to_sail(source_pix_fmt);
    if (direct_format != SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        switch (direct_format)
        {
        case SAIL_PIXEL_FORMAT_BPP24_YUV:
        case SAIL_PIXEL_FORMAT_BPP30_YUV:
        case SAIL_PIXEL_FORMAT_BPP36_YUV:
        case SAIL_PIXEL_FORMAT_BPP48_YUV:
        case SAIL_PIXEL_FORMAT_BPP32_YUVA:
        case SAIL_PIXEL_FORMAT_BPP40_YUVA:
        case SAIL_PIXEL_FORMAT_BPP48_YUVA:
        case SAIL_PIXEL_FORMAT_BPP64_YUVA: break;
        default: return source_pix_fmt;
        }
    }

    /* Find the best matching format from the list. */
    enum AVPixelFormat best_format = avcodec_find_best_pix_fmt_of_list(sail_formats, source_pix_fmt, 0, NULL);
    if (best_format == AV_PIX_FMT_NONE)
    {
        best_format = AV_PIX_FMT_RGB24;
    }

    enum SailPixelFormat best_sail_format = video_private_av_pixel_format_to_sail(best_format);
    switch (best_sail_format)
    {
    case SAIL_PIXEL_FORMAT_BPP24_YUV: best_format = AV_PIX_FMT_RGB24; break;
    case SAIL_PIXEL_FORMAT_BPP30_YUV: best_format = AV_PIX_FMT_RGB48LE; break;
    case SAIL_PIXEL_FORMAT_BPP36_YUV: best_format = AV_PIX_FMT_RGB48LE; break;
    case SAIL_PIXEL_FORMAT_BPP48_YUV: best_format = AV_PIX_FMT_RGB48LE; break;
    case SAIL_PIXEL_FORMAT_BPP32_YUVA: best_format = AV_PIX_FMT_RGBA; break;
    case SAIL_PIXEL_FORMAT_BPP40_YUVA: best_format = AV_PIX_FMT_RGBA64LE; break;
    case SAIL_PIXEL_FORMAT_BPP48_YUVA: best_format = AV_PIX_FMT_RGBA64LE; break;
    case SAIL_PIXEL_FORMAT_BPP64_YUVA: best_format = AV_PIX_FMT_RGBA64LE; break;
    default: break;
    }

    return best_format;
}

static sail_status_t fetch_single_meta_data_from_dict(AVDictionary* dict,
                                                      const char* ffmpeg_key,
                                                      enum SailMetaData sail_key,
                                                      struct sail_meta_data_node*** last_meta_data_node)
{
    AVDictionaryEntry* entry = av_dict_get(dict, ffmpeg_key, NULL, 0);
    if (entry == NULL || entry->value == NULL)
    {
        return SAIL_OK;
    }

    struct sail_meta_data_node* meta_data_node;
    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(sail_key, &meta_data_node->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, entry->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    **last_meta_data_node = meta_data_node;
    *last_meta_data_node  = &meta_data_node->next;

    return SAIL_OK;
}

sail_status_t video_private_fetch_meta_data(AVFormatContext* format_ctx,
                                            AVStream* video_stream,
                                            struct sail_meta_data_node*** last_meta_data_node)
{
    SAIL_CHECK_PTR(last_meta_data_node);

    /* Extract metadata from format context (file-level). */
    if (format_ctx != NULL && format_ctx->metadata != NULL)
    {
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "title", SAIL_META_DATA_TITLE,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "author", SAIL_META_DATA_AUTHOR,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "artist", SAIL_META_DATA_ARTIST,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "copyright", SAIL_META_DATA_COPYRIGHT,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "comment", SAIL_META_DATA_COMMENT,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "description", SAIL_META_DATA_DESCRIPTION,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "encoder", SAIL_META_DATA_SOFTWARE,
                                                  last_meta_data_node));
        SAIL_TRY(fetch_single_meta_data_from_dict(format_ctx->metadata, "date", SAIL_META_DATA_CREATION_TIME,
                                                  last_meta_data_node));
    }

    /* Extract metadata from video stream (stream-level). */
    if (video_stream != NULL && video_stream->metadata != NULL)
    {
        AVDictionaryEntry* entry;

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "title", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "title", SAIL_META_DATA_TITLE,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "author", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "author", SAIL_META_DATA_AUTHOR,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "artist", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "artist", SAIL_META_DATA_ARTIST,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "copyright", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "copyright", SAIL_META_DATA_COPYRIGHT,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "comment", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "comment", SAIL_META_DATA_COMMENT,
                                                      last_meta_data_node));
        }

        entry =
            av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "description", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "description", SAIL_META_DATA_DESCRIPTION,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "encoder", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "encoder", SAIL_META_DATA_SOFTWARE,
                                                      last_meta_data_node));
        }

        entry = av_dict_get(format_ctx != NULL ? format_ctx->metadata : NULL, "date", NULL, AV_DICT_IGNORE_SUFFIX);
        if (entry == NULL)
        {
            SAIL_TRY(fetch_single_meta_data_from_dict(video_stream->metadata, "date", SAIL_META_DATA_CREATION_TIME,
                                                      last_meta_data_node));
        }
    }

    return SAIL_OK;
}

static double av_rational_to_double(AVRational r)
{
    if (r.den == 0)
    {
        return 0.0;
    }
    return (double)r.num / (double)r.den;
}

static const char* av_level_to_string(int level)
{
    static char level_str[16];

    /*
     * FFmpeg levels are typically encoded as XX for X.X, XXX for XX.X, etc.
     * For example: 10 -> "1.0", 11 -> "1.1", 20 -> "2.0", 51 -> "5.1"
     */
    if (level < 10)
    {
        snprintf(level_str, sizeof(level_str), "%d", level);
    }
    else if (level < 100)
    {
        snprintf(level_str, sizeof(level_str), "%d.%d", level / 10, level % 10);
    }
    else if (level < 1000)
    {
        snprintf(level_str, sizeof(level_str), "%d.%d", level / 10, level % 10);
    }
    else if (level < 10000)
    {
        snprintf(level_str, sizeof(level_str), "%d.%d", level / 10, level % 10);
    }
    else
    {
        snprintf(level_str, sizeof(level_str), "%d", level);
    }

    return level_str;
}


sail_status_t video_private_fetch_special_properties(struct AVFormatContext* format_ctx,
                                                     struct AVStream* video_stream,
                                                     struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(video_stream);
    SAIL_CHECK_PTR(special_properties);

    AVCodecParameters* codecpar = video_stream->codecpar;

    /* Codec ID and name. */
    if (codecpar->codec_id != AV_CODEC_ID_NONE)
    {
        const char* codec_name = avcodec_get_name(codecpar->codec_id);
        if (codec_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-codec", codec_name));
        }
    }

    /* Bitrate. */
    if (codecpar->bit_rate > 0)
    {
        SAIL_TRY(sail_put_hash_map_unsigned_long_long(special_properties, "video-bitrate", (unsigned long long)codecpar->bit_rate));
    }

    /* Profile (check for unknown values). */
    if (codecpar->profile != AV_PROFILE_UNKNOWN && codecpar->profile >= 0)
    {
        const char* profile_name = avcodec_profile_name(codecpar->codec_id, codecpar->profile);
        if (profile_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-profile", profile_name));
        }
    }

    /* Level (check for unknown values). */
    if (codecpar->level != AV_LEVEL_UNKNOWN && codecpar->level >= 0)
    {
        const char* level_str = av_level_to_string(codecpar->level);
        SAIL_TRY(sail_put_hash_map_string(special_properties, "video-level", level_str));
    }

    /* Framerate from codecpar (constant framerate). */
    if (codecpar->framerate.num > 0 && codecpar->framerate.den > 0)
    {
        double framerate = av_rational_to_double(codecpar->framerate);
        SAIL_TRY(sail_put_hash_map_double(special_properties, "video-framerate", framerate));
    }

    /* Estimated framerate from stream. */
    if (video_stream->r_frame_rate.num > 0 && video_stream->r_frame_rate.den > 0)
    {
        double r_framerate = av_rational_to_double(video_stream->r_frame_rate);
        SAIL_TRY(sail_put_hash_map_double(special_properties, "video-estimated-framerate", r_framerate));
    }

    /* Time base. */
    if (video_stream->time_base.num > 0 && video_stream->time_base.den > 0)
    {
        double time_base = av_rational_to_double(video_stream->time_base);
        SAIL_TRY(sail_put_hash_map_double(special_properties, "video-time-base", time_base));
    }

    /* Color space. */
    if (codecpar->color_space != AVCOL_SPC_UNSPECIFIED)
    {
        const char* color_space_name = av_color_space_name(codecpar->color_space);
        if (color_space_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-color-space", color_space_name));
        }
    }

    /* Color range. */
    if (codecpar->color_range != AVCOL_RANGE_UNSPECIFIED)
    {
        const char* color_range_name = av_color_range_name(codecpar->color_range);
        if (color_range_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-color-range", color_range_name));
        }
    }

    /* Color primaries. */
    if (codecpar->color_primaries != AVCOL_PRI_UNSPECIFIED)
    {
        const char* color_primaries_name = av_color_primaries_name(codecpar->color_primaries);
        if (color_primaries_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-color-primaries", color_primaries_name));
        }
    }

    /* Color transfer characteristics. */
    if (codecpar->color_trc != AVCOL_TRC_UNSPECIFIED)
    {
        const char* color_transfer_name = av_color_transfer_name(codecpar->color_trc);
        if (color_transfer_name != NULL)
        {
            SAIL_TRY(sail_put_hash_map_string(special_properties, "video-color-transfer", color_transfer_name));
        }
    }

    /* Duration. */
    if (format_ctx != NULL && video_stream->duration != AV_NOPTS_VALUE && video_stream->time_base.den > 0)
    {
        int64_t duration_ms = av_rescale_q(video_stream->duration, video_stream->time_base, (AVRational){1, 1000});
        if (duration_ms > 0)
        {
            SAIL_TRY(sail_put_hash_map_unsigned_long_long(special_properties, "video-duration", (unsigned long long)duration_ms));
        }
    }

    /* Number of frames. */
    if (video_stream->nb_frames > 0)
    {
        SAIL_TRY(sail_put_hash_map_unsigned_long_long(special_properties, "video-nb-frames",
                                                       (unsigned long long)video_stream->nb_frames));
    }

    return SAIL_OK;
}


static int video_private_parse_skip_frame(const char* str_value)
{
    if (strcmp(str_value, "none") == 0)
    {
        return AVDISCARD_NONE;
    }
    else if (strcmp(str_value, "non-ref") == 0)
    {
        return AVDISCARD_NONREF;
    }
    else if (strcmp(str_value, "bidir") == 0)
    {
        return AVDISCARD_BIDIR;
    }
    else if (strcmp(str_value, "non-key") == 0)
    {
        return AVDISCARD_NONKEY;
    }
    else if (strcmp(str_value, "all") == 0)
    {
        return AVDISCARD_ALL;
    }
    else
    {
        return -1;
    }
}

static int video_private_parse_lowres(const char* str_value)
{
    if (str_value == NULL)
    {
        return -1;
    }
    else if (strcmp(str_value, "full") == 0)
    {
        return 0;
    }
    else if (strcmp(str_value, "half") == 0)
    {
        return 1;
    }
    else if (strcmp(str_value, "quarter") == 0)
    {
        return 2;
    }
    else if (strcmp(str_value, "eighth") == 0)
    {
        return 3;
    }
    else
    {
        return -1;
    }
}

static int video_private_parse_error_concealment(const char* str_value)
{
    struct sail_string_node* string_node_flags;
    if (sail_split_into_string_node_chain(str_value, &string_node_flags) != SAIL_OK)
    {
        return -1;
    }

    int flags = 0;

    for (const struct sail_string_node* node = string_node_flags; node != NULL; node = node->next)
    {
        if (strcmp(node->string, "flags") == 0)
        {
            flags |= FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
        }
        else if (strcmp(node->string, "mv") == 0)
        {
            flags |= FF_EC_GUESS_MVS;
        }
        else if (strcmp(node->string, "dc") == 0)
    {
            flags |= FF_EC_DEBLOCK;
        }
    }

    sail_destroy_string_node_chain(string_node_flags);

    return flags;
}

bool video_private_load_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    AVCodecContext* codec_ctx = user_data;

    if (strcmp(key, "video-threads") == 0)
    {
        unsigned threads = sail_variant_to_unsigned_int(value);
        if (threads > 0 && threads <= 64)
        {
            av_opt_set_int(codec_ctx, "threads", (int64_t)threads, 0);
            SAIL_LOG_TRACE("VIDEO: Set decoder threads to %u", threads);
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-threads' must be in range [1, 64], got %u", threads);
        }
    }
    else if (strcmp(key, "video-low-resolution") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int lowres = video_private_parse_lowres(str_value);
            if (lowres >= 0)
            {
                av_opt_set_int(codec_ctx, "lowres", (int64_t)lowres, 0);
                SAIL_LOG_TRACE("VIDEO: Set lowres to %s", str_value);
            }
            else
            {
                SAIL_LOG_ERROR("VIDEO: 'video-low-resolution' must be one of: full, half, quarter, eighth (or 0, 1, 2, 3)");
            }
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-low-resolution' must be a string");
        }
    }
    else if (strcmp(key, "video-skip-frame") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int skip_frame = video_private_parse_skip_frame(str_value);
            if (skip_frame >= 0)
            {
                codec_ctx->skip_frame = skip_frame;
                SAIL_LOG_TRACE("VIDEO: Set skip_frame to %s", str_value);
            }
            else
            {
                SAIL_LOG_ERROR("VIDEO: 'video-skip-frame' must be one of: none, non-ref, bidir, non-key, all");
            }
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-skip-frame' must be a string");
        }
    }
    else if (strcmp(key, "video-skip-idct") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int skip_idct = video_private_parse_skip_frame(str_value);
            if (skip_idct >= 0)
            {
                codec_ctx->skip_idct = skip_idct;
                SAIL_LOG_TRACE("VIDEO: Set skip_idct to %s", str_value);
            }
            else
            {
                SAIL_LOG_ERROR("VIDEO: 'video-skip-idct' must be one of: none, non-ref, bidir, non-key, all");
            }
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-skip-idct' must be a string");
        }
    }
    else if (strcmp(key, "video-skip-loop-filter") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int skip_loop_filter = video_private_parse_skip_frame(str_value);
            if (skip_loop_filter >= 0)
            {
                codec_ctx->skip_loop_filter = skip_loop_filter;
                SAIL_LOG_TRACE("VIDEO: Set skip_loop_filter to %s", str_value);
            }
            else
            {
                SAIL_LOG_ERROR("VIDEO: 'video-skip-loop-filter' must be one of: none, non-ref, bidir, non-key, all");
            }
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-skip-loop-filter' must be a string");
        }
    }
    else if (strcmp(key, "video-error-concealment") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int error_concealment = video_private_parse_error_concealment(str_value);
            if (error_concealment >= 0)
            {
                codec_ctx->error_concealment = error_concealment;
                SAIL_LOG_TRACE("VIDEO: Set error_concealment to %s", str_value);
            }
            else
            {
                SAIL_LOG_ERROR("VIDEO: 'video-error-concealment' must be a ';'-separated list of: flags, mv, dc");
            }
        }
        else
        {
            SAIL_LOG_ERROR("VIDEO: 'video-error-concealment' must be a string");
        }
    }

    return true;
}
