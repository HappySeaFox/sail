/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)
 *
 *  Copyright (c) 2026 Dmitry Baryshev
 *
 *  The MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <stdint.h>

#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

#include <sail-common/export.h>
#include <sail-common/sail-common.h>

#include "scale_swscale.h"

#ifdef SAIL_MANIP_SWSCALE_ENABLED

/* Convert SailScaling enum to swscale flags. */
int sail_scaling_to_swscale_flags(enum SailScaling algorithm)
{
    switch (algorithm)
    {
    case SAIL_SCALING_NEAREST_NEIGHBOR:
        return SWS_POINT;
    case SAIL_SCALING_BILINEAR:
        return SWS_BILINEAR;
    case SAIL_SCALING_BICUBIC:
        return SWS_BICUBIC;
    case SAIL_SCALING_LANCZOS:
        return SWS_LANCZOS;
    default:
        return SWS_BILINEAR;
    }
}

/* Convert SailPixelFormat to AVPixelFormat. */
enum AVPixelFormat sail_to_av_pixel_format(enum SailPixelFormat sail_pix_fmt)
{
    switch (sail_pix_fmt)
    {
    case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
        return AV_PIX_FMT_GRAY8;
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:
        return AV_PIX_FMT_GRAY16LE;
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA:
        return AV_PIX_FMT_YA8;
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA:
        return AV_PIX_FMT_YA16LE;

    case SAIL_PIXEL_FORMAT_BPP24_RGB:
        return AV_PIX_FMT_RGB24;
    case SAIL_PIXEL_FORMAT_BPP24_BGR:
        return AV_PIX_FMT_BGR24;
    case SAIL_PIXEL_FORMAT_BPP48_RGB:
        return AV_PIX_FMT_RGB48LE;
    case SAIL_PIXEL_FORMAT_BPP48_BGR:
        return AV_PIX_FMT_BGR48LE;

    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        return AV_PIX_FMT_RGBA;
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        return AV_PIX_FMT_BGRA;
    case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        return AV_PIX_FMT_ARGB;
    case SAIL_PIXEL_FORMAT_BPP32_ABGR:
        return AV_PIX_FMT_ABGR;
    case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        return AV_PIX_FMT_RGBA64LE;
    case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        return AV_PIX_FMT_BGRA64LE;

    case SAIL_PIXEL_FORMAT_BPP32_RGBX:
        return AV_PIX_FMT_RGB0;
    case SAIL_PIXEL_FORMAT_BPP32_BGRX:
        return AV_PIX_FMT_BGR0;
    case SAIL_PIXEL_FORMAT_BPP32_XRGB:
        return AV_PIX_FMT_0RGB;
    case SAIL_PIXEL_FORMAT_BPP32_XBGR:
        return AV_PIX_FMT_0BGR;

    case SAIL_PIXEL_FORMAT_BPP24_YUV:
        return AV_PIX_FMT_YUV420P;
    case SAIL_PIXEL_FORMAT_BPP30_YUV:
        return AV_PIX_FMT_YUV420P10LE;
    case SAIL_PIXEL_FORMAT_BPP36_YUV:
        return AV_PIX_FMT_YUV420P12LE;
    case SAIL_PIXEL_FORMAT_BPP48_YUV:
        return AV_PIX_FMT_YUV420P16LE;

    case SAIL_PIXEL_FORMAT_BPP32_YUVA:
        return AV_PIX_FMT_YUVA420P;
    case SAIL_PIXEL_FORMAT_BPP40_YUVA:
        return AV_PIX_FMT_YUVA420P10LE;
    case SAIL_PIXEL_FORMAT_BPP48_YUVA:
        return AV_PIX_FMT_YUVA422P12LE;
    case SAIL_PIXEL_FORMAT_BPP64_YUVA:
        return AV_PIX_FMT_YUVA420P16LE;

    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
        return AV_PIX_FMT_PAL8;

    default:
        return AV_PIX_FMT_NONE;
    }
}

/* Scale using libswscale. */
sail_status_t scale_with_swscale(const struct sail_image* src_image,
                                 struct sail_image* dst_image,
                                 enum SailScaling algorithm)
{
    enum AVPixelFormat src_av = sail_to_av_pixel_format(src_image->pixel_format);
    enum AVPixelFormat dst_av = sail_to_av_pixel_format(dst_image->pixel_format);

    if (src_av == AV_PIX_FMT_NONE || dst_av == AV_PIX_FMT_NONE)
    {
        SAIL_LOG_ERROR("SWSCALE: Unsupported pixel format for scaling");
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    /* Check if formats are supported. */
    if (!sws_isSupportedInput(src_av) || !sws_isSupportedOutput(dst_av))
    {
        SAIL_LOG_ERROR("SWSCALE: Format not supported by swscale");
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    /* Skip indexed formats - swscale doesn't handle palette directly. */
    if (src_av == AV_PIX_FMT_PAL8 || src_image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED
        || src_image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED
        || src_image->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED)
    {
        SAIL_LOG_ERROR("SWSCALE: Indexed formats not supported for scaling");
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    int flags = sail_scaling_to_swscale_flags(algorithm);

    /* Create swscale context. */
    struct SwsContext* sws_ctx = sws_getContext(src_image->width, src_image->height, src_av, dst_image->width,
                                                dst_image->height, dst_av, flags, NULL, NULL, NULL);

    if (sws_ctx == NULL)
    {
        SAIL_LOG_ERROR("SWSCALE: Failed to create context for scaling");
        return SAIL_ERROR_MEMORY_ALLOCATION;
    }

    /* Prepare source data pointers. */
    const uint8_t* src_data[4] = {(const uint8_t*)src_image->pixels, NULL, NULL, NULL};
    int src_linesize[4]        = {(int)src_image->bytes_per_line, 0, 0, 0};

    /* Prepare destination data pointers. */
    uint8_t* dst_data[4] = {(uint8_t*)dst_image->pixels, NULL, NULL, NULL};
    int dst_linesize[4]  = {(int)dst_image->bytes_per_line, 0, 0, 0};

    int result = sws_scale(sws_ctx, src_data, src_linesize, 0, src_image->height, dst_data, dst_linesize);

    sws_freeContext(sws_ctx);

    if (result < 0 || result != (int)dst_image->height)
    {
        SAIL_LOG_ERROR("SWSCALE: Scaling failed or incomplete (result: %d, expected: %u)", result, dst_image->height);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    SAIL_LOG_DEBUG("SWSCALE: Successfully scaled image from %ux%u to %ux%u using algorithm %d", src_image->width,
                   src_image->height, dst_image->width, dst_image->height, algorithm);

    return SAIL_OK;
}

#endif /* SAIL_MANIP_SWSCALE_ENABLED */
