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

#include <string.h>

#include "sail-common.h"

#include "helpers.h"

void webp_private_fill_color(uint8_t *pixels, unsigned bytes_per_line, unsigned bytes_per_pixel,
                                uint32_t color, unsigned x, unsigned y, unsigned width, unsigned height) {

    uint8_t *scanline = pixels + y * bytes_per_line + x * bytes_per_pixel;

    for (unsigned row = 0; row < height; row++, scanline += bytes_per_line) {
        for (unsigned column = 0; column < width * bytes_per_pixel; column += bytes_per_pixel) {
            memcpy(scanline + column, &color, sizeof(color));
        }
    }
}

sail_status_t webp_private_blend_over(void *dst_raw, unsigned dst_offset, const void *src_raw, unsigned width, unsigned bytes_per_pixel) {

    SAIL_CHECK_PTR(src_raw);
    SAIL_CHECK_PTR(dst_raw);

    if (bytes_per_pixel == 4) {
        const uint8_t *src = src_raw;
        uint8_t *dst = (uint8_t *)dst_raw + dst_offset * bytes_per_pixel;

        while (width--) {
            const double src_a = *(src+3) / 255.0;
            const double dst_a = *(dst+3) / 255.0;

            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)(src_a * (*src) + (1-src_a) * dst_a * (*dst)); src++; dst++;
            *dst = (uint8_t)((src_a + (1-src_a) * dst_a) * 255);           src++; dst++;
        }
    } else {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    return SAIL_OK;
}

sail_status_t webp_private_fetch_iccp(WebPDemuxer *webp_demux, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(webp_demux);
    SAIL_CHECK_PTR(iccp);

    const uint32_t webp_flags = WebPDemuxGetI(webp_demux, WEBP_FF_FORMAT_FLAGS);

    if (webp_flags & ICCP_FLAG) {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "ICCP", 1, &chunk_iterator)) {
            SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_from_data(chunk_iterator.chunk.bytes, (unsigned)chunk_iterator.chunk.size, iccp),
                        /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            WebPDemuxReleaseChunkIterator(&chunk_iterator);
        }
    }

    return SAIL_OK;
}

sail_status_t webp_private_fetch_meta_data(WebPDemuxer *webp_demux, struct sail_meta_data_node **last_meta_data_node) {

    SAIL_CHECK_PTR(webp_demux);
    SAIL_CHECK_PTR(last_meta_data_node);

    const uint32_t webp_flags = WebPDemuxGetI(webp_demux, WEBP_FF_FORMAT_FLAGS);

    if (webp_flags & XMP_FLAG) {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "XMP ", 1, &chunk_iterator)) {
            struct sail_meta_data_node *meta_data_node;

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node),
                                /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_XMP, &meta_data_node->meta_data),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_substring(meta_data_node->meta_data->value,
                                                            (const char *)chunk_iterator.chunk.bytes,
                                                            chunk_iterator.chunk.size),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));

            WebPDemuxReleaseChunkIterator(&chunk_iterator);

            *last_meta_data_node = meta_data_node;
            last_meta_data_node = &meta_data_node->next;
        }
    }

    if (webp_flags & EXIF_FLAG) {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "EXIF", 1, &chunk_iterator)) {
            struct sail_meta_data_node *meta_data_node;

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node),
                                /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_EXIF, &meta_data_node->meta_data),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value,
                                                        chunk_iterator.chunk.bytes,
                                                        chunk_iterator.chunk.size),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                              WebPDemuxReleaseChunkIterator(&chunk_iterator));

            WebPDemuxReleaseChunkIterator(&chunk_iterator);

            *last_meta_data_node = meta_data_node;
            last_meta_data_node = &meta_data_node->next;
        }
    }

    return SAIL_OK;
}
