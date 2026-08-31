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

#include <sail-common/sail-common.h>

#include "scale_swscale.h"
#include "swscale_conversions.h"

#ifdef SAIL_MANIP_SWSCALE_ENABLED

#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

/*
 * Private functions.
 */

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
