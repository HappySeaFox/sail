/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-common.h"

#include "helpers.h"

enum SailPixelFormat avif_private_sail_pixel_format(enum avifPixelFormat avif_pixel_format, uint32_t depth, bool has_alpha) {

    switch (avif_pixel_format) {
        case AVIF_PIXEL_FORMAT_NONE: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }

        default: {
            switch (depth) {
                case 8: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP32_YUVA : SAIL_PIXEL_FORMAT_BPP24_YUV;
                }
                case 10: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP40_YUVA : SAIL_PIXEL_FORMAT_BPP30_YUV;
                }
                case 12: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP48_YUVA : SAIL_PIXEL_FORMAT_BPP36_YUV;
                }
                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
    }
}

enum SailChromaSubsampling avif_private_sail_chroma_subsampling(enum avifPixelFormat avif_pixel_format) {

    switch (avif_pixel_format) {
        case AVIF_PIXEL_FORMAT_YUV444: return SAIL_CHROMA_SUBSAMPLING_444;
        case AVIF_PIXEL_FORMAT_YUV422: return SAIL_CHROMA_SUBSAMPLING_422;
        case AVIF_PIXEL_FORMAT_YUV420: return SAIL_CHROMA_SUBSAMPLING_420;
        case AVIF_PIXEL_FORMAT_YUV400: return SAIL_CHROMA_SUBSAMPLING_400;

        default: {
            return SAIL_CHROMA_SUBSAMPLING_UNKNOWN;
        }
    }
}

enum SailPixelFormat avif_private_rgb_sail_pixel_format(enum avifRGBFormat rgb_pixel_format, uint32_t depth) {

    switch (depth) {
        case 8: {
            switch (rgb_pixel_format) {
                case AVIF_RGB_FORMAT_RGB:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case AVIF_RGB_FORMAT_RGBA: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                case AVIF_RGB_FORMAT_ARGB: return SAIL_PIXEL_FORMAT_BPP32_ARGB;
                case AVIF_RGB_FORMAT_BGR:  return SAIL_PIXEL_FORMAT_BPP24_BGR;
                case AVIF_RGB_FORMAT_BGRA: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
                case AVIF_RGB_FORMAT_ABGR: return SAIL_PIXEL_FORMAT_BPP32_ABGR;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case 16: {
            switch (rgb_pixel_format) {
                case AVIF_RGB_FORMAT_RGB:  return SAIL_PIXEL_FORMAT_BPP48_RGB;
                case AVIF_RGB_FORMAT_RGBA: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
                case AVIF_RGB_FORMAT_ARGB: return SAIL_PIXEL_FORMAT_BPP64_ARGB;
                case AVIF_RGB_FORMAT_BGR:  return SAIL_PIXEL_FORMAT_BPP48_BGR;
                case AVIF_RGB_FORMAT_BGRA: return SAIL_PIXEL_FORMAT_BPP64_BGRA;
                case AVIF_RGB_FORMAT_ABGR: return SAIL_PIXEL_FORMAT_BPP64_ABGR;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

uint32_t avif_private_round_depth(uint32_t depth) {

    if (depth > 8) {
        return 16;
    } else {
        return 8;
    }
}

sail_status_t avif_private_fetch_iccp(const struct avifRWData *avif_iccp, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(avif_iccp);
    SAIL_CHECK_PTR(iccp);

    if (avif_iccp->data != NULL) {
        SAIL_TRY(sail_alloc_iccp_from_data(avif_iccp->data, (unsigned)avif_iccp->size, iccp));
        SAIL_LOG_DEBUG("AVIF: Found ICC profile %u bytes long", (unsigned)avif_iccp->size);
    } else {
        SAIL_LOG_DEBUG("AVIF: ICC profile is not found");
    }

    return SAIL_OK;
}
