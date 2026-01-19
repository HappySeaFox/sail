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

#include <stdbool.h>
#include <stdint.h>

#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

#include <sail-common/sail-common.h>

#include "swscale_conversions.h"

#ifdef SAIL_MANIP_SWSCALE_ENABLED

/*
 * Private functions.
 */

/* Convert SailPixelFormat to AVPixelFormat. */
static enum AVPixelFormat sail_to_av_pixel_format(enum SailPixelFormat sail_pix_fmt)
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

/* Check if swscale supports this conversion. */
static bool swscale_supports_conversion(enum AVPixelFormat src_fmt, enum AVPixelFormat dst_fmt)
{
    if (src_fmt == AV_PIX_FMT_NONE || dst_fmt == AV_PIX_FMT_NONE)
    {
        return false;
    }

    /* Check if formats are supported. */
    if (!sws_isSupportedInput(src_fmt) || !sws_isSupportedOutput(dst_fmt))
    {
        return false;
    }

    /* Try to create a context to verify conversion is possible. */
    struct SwsContext* test_ctx = sws_getContext(1, 1, src_fmt, 1, 1, dst_fmt, SWS_BILINEAR, NULL, NULL, NULL);
    if (test_ctx == NULL)
    {
        return false;
    }
    sws_freeContext(test_ctx);

    return true;
}

/*
 * Public functions.
 */

bool sail_try_swscale_conversion(const struct sail_image* image_input,
                                 struct sail_image* image_output,
                                 enum SailPixelFormat output_pixel_format)
{
    /* Ensure dimensions match. */
    if (image_input->width != image_output->width || image_input->height != image_output->height)
    {
        return false;
    }

    /* Convert formats to AVPixelFormat. */
    enum AVPixelFormat src_av = sail_to_av_pixel_format(image_input->pixel_format);
    enum AVPixelFormat dst_av = sail_to_av_pixel_format(output_pixel_format);

    /* Check if swscale supports this conversion. */
    if (!swscale_supports_conversion(src_av, dst_av))
    {
        return false;
    }

    /* Skip indexed formats - swscale doesn't handle palette directly. */
    if (src_av == AV_PIX_FMT_PAL8 || image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED
        || image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED
        || image_input->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED)
    {
        return false;
    }

    /* Create swscale context. */
    struct SwsContext* sws_ctx =
        sws_getContext(image_input->width, image_input->height, src_av, image_output->width, image_output->height,
                       dst_av, SWS_BILINEAR | SWS_ACCURATE_RND, NULL, NULL, NULL);

    if (sws_ctx == NULL)
    {
        SAIL_LOG_ERROR("SWSCALE: Failed to create context for conversion");
        return false;
    }

    /* Prepare source data pointers. */
    const uint8_t* src_data[4] = {(const uint8_t*)image_input->pixels, NULL, NULL, NULL};
    int src_linesize[4]        = {(int)image_input->bytes_per_line, 0, 0, 0};

    /* Prepare destination data pointers. */
    uint8_t* dst_data[4] = {(uint8_t*)image_output->pixels, NULL, NULL, NULL};
    int dst_linesize[4]  = {(int)image_output->bytes_per_line, 0, 0, 0};

    /* Perform conversion. */
    int result = sws_scale(sws_ctx, src_data, src_linesize, 0, image_input->height, dst_data, dst_linesize);

    /* Cleanup. */
    sws_freeContext(sws_ctx);

    if (result < 0 || result != (int)image_output->height)
    {
        SAIL_LOG_ERROR("SWSCALE: Conversion failed or incomplete (result: %d, expected: %u)", result,
                       image_output->height);
        return false;
    }

    SAIL_LOG_DEBUG("SWSCALE: Successfully converted from %s to %s",
                   sail_pixel_format_to_string(image_input->pixel_format),
                   sail_pixel_format_to_string(output_pixel_format));

    return true;
}

#endif /* SAIL_MANIP_SWSCALE_ENABLED */
