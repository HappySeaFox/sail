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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <webp/decode.h>
#include <webp/demux.h>

#include "sail-common.h"

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct webp_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    struct sail_image *canvas_image;
    WebPDemuxer *webp_demux;
    WebPIterator *webp_iterator;
    unsigned frame_number;
    uint32_t background_color;
    uint32_t frame_count;
    unsigned bytes_per_pixel;
    unsigned frame_x;
    unsigned frame_y;
    unsigned frame_width;
    unsigned frame_height;
    WebPMuxAnimDispose frame_dispose_method;
    WebPMuxAnimBlend frame_blend_method;

    void *image_data;
    size_t image_data_size;
};

static sail_status_t alloc_webp_state(struct webp_state **webp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct webp_state), &ptr));
    *webp_state = ptr;

    (*webp_state)->load_options = NULL;
    (*webp_state)->save_options = NULL;
    (*webp_state)->canvas_image = NULL;

    (*webp_state)->webp_demux            = NULL;
    (*webp_state)->webp_iterator         = NULL;
    (*webp_state)->frame_number          = 0;
    (*webp_state)->background_color      = 0;
    (*webp_state)->frame_count           = 0;
    (*webp_state)->bytes_per_pixel       = 0;
    (*webp_state)->frame_x               = 0;
    (*webp_state)->frame_y               = 0;
    (*webp_state)->frame_width           = 0;
    (*webp_state)->frame_height          = 0;
    (*webp_state)->frame_dispose_method  = WEBP_MUX_DISPOSE_NONE;
    (*webp_state)->frame_blend_method    = WEBP_MUX_NO_BLEND;

    (*webp_state)->image_data      = NULL;
    (*webp_state)->image_data_size = 0;

    return SAIL_OK;
}

static void destroy_webp_state(struct webp_state *webp_state) {

    if (webp_state == NULL) {
        return;
    }

    if (webp_state->webp_iterator != NULL) {
        WebPDemuxReleaseIterator(webp_state->webp_iterator);
        sail_free(webp_state->webp_iterator);
    }

    sail_free(webp_state->image_data);

    WebPDemuxDelete(webp_state->webp_demux);

    sail_destroy_load_options(webp_state->load_options);
    sail_destroy_save_options(webp_state->save_options);
    sail_destroy_image(webp_state->canvas_image);

    sail_free(webp_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_webp(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct webp_state *webp_state;
    SAIL_TRY(alloc_webp_state(&webp_state));
    *state = webp_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &webp_state->load_options));

    /* Read the entire image. */
    SAIL_ALIGNAS(uint32_t) char signature_and_size[8];
    SAIL_TRY(io->strict_read(io->stream, signature_and_size, sizeof(signature_and_size)));
    webp_state->image_data_size = *(uint32_t *)(signature_and_size + 4) + sizeof(signature_and_size);

    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    void *ptr;
    SAIL_TRY(sail_malloc(webp_state->image_data_size, &ptr));
    webp_state->image_data = ptr;

    SAIL_TRY(io->strict_read(io->stream, webp_state->image_data, webp_state->image_data_size));

    /* Construct a WebP demuxer. */
    const WebPData data = { webp_state->image_data, webp_state->image_data_size };

    webp_state->webp_demux = WebPDemux(&data);

    SAIL_TRY(sail_malloc(sizeof(WebPIterator), &ptr));
    webp_state->webp_iterator = ptr;

    /* Frame count and other image info. */
    webp_state->background_color = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_BACKGROUND_COLOR);
    webp_state->frame_count      = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_FRAME_COUNT);

    /* Construct a canvas image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->chroma_subsampling = SAIL_CHROMA_SUBSAMPLING_420;
    image_local->source_image->compression = SAIL_COMPRESSION_WEBP;

    image_local->width = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_CANVAS_WIDTH);
    image_local->height = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_CANVAS_HEIGHT);
    image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));
    webp_state->bytes_per_pixel = image_local->bytes_per_line / image_local->width;

    /* Fetch ICCP. */
    if (webp_state->load_options->options & SAIL_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(webp_private_fetch_iccp(webp_state->webp_demux, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch meta data. */
    if (webp_state->load_options->options & SAIL_OPTION_META_DATA) {
        SAIL_TRY_OR_CLEANUP(webp_private_fetch_meta_data(webp_state->webp_demux, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    webp_state->canvas_image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_webp(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct webp_state *webp_state = (struct webp_state *)state;

    /* Start demuxing. */
    if (webp_state->frame_number == 0) {
        if (WebPDemuxGetFrame(webp_state->webp_demux, 1, webp_state->webp_iterator) == 0) {
            SAIL_LOG_ERROR("WEBP: Failed to get the first frame");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Allocate a canvas frame to apply disposal later. */
        size_t image_size = (size_t)webp_state->canvas_image->bytes_per_line * webp_state->canvas_image->height;

        void *ptr;
        SAIL_TRY(sail_malloc(image_size, &ptr));
        webp_state->canvas_image->pixels = ptr;

        /* Fill background. */
        webp_private_fill_color(webp_state->canvas_image->pixels, webp_state->canvas_image->bytes_per_line, webp_state->bytes_per_pixel,
                                webp_state->background_color, 0, 0, webp_state->canvas_image->width, webp_state->canvas_image->height);
    } else {
        switch (webp_state->frame_dispose_method) {
            case WEBP_MUX_DISPOSE_BACKGROUND: {
                webp_private_fill_color(webp_state->canvas_image->pixels, webp_state->canvas_image->bytes_per_line, webp_state->bytes_per_pixel,
                                        webp_state->background_color, webp_state->frame_x, webp_state->frame_y,
                                        webp_state->frame_width, webp_state->frame_height);
                break;
            }
            case WEBP_MUX_DISPOSE_NONE: {
                break;
            }
            default: {
                SAIL_LOG_ERROR("WEBP: Unknown disposal method");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
        }

        if (WebPDemuxNextFrame(webp_state->webp_iterator) == 0) {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
        }
    }

    webp_state->frame_number++;
    webp_state->frame_x              = webp_state->webp_iterator->x_offset;
    webp_state->frame_y              = webp_state->webp_iterator->y_offset;
    webp_state->frame_width          = webp_state->webp_iterator->width;
    webp_state->frame_height         = webp_state->webp_iterator->height;
    webp_state->frame_dispose_method = webp_state->webp_iterator->dispose_method;
    webp_state->frame_blend_method   = webp_state->webp_iterator->blend_method;

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image_skeleton(webp_state->canvas_image, &image_local));

    image_local->source_image->pixel_format = webp_state->webp_iterator->has_alpha ? SAIL_PIXEL_FORMAT_BPP32_YUVA : SAIL_PIXEL_FORMAT_BPP24_YUV;

    if (webp_state->frame_count > 1) {
        /* Fall back to 100 ms. when the duration is <= 0. */
        image_local->delay = webp_state->webp_iterator->duration <= 0 ? 100 : webp_state->webp_iterator->duration;
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_webp(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct webp_state *webp_state = (struct webp_state *)state;

    switch (webp_state->frame_blend_method) {
        case WEBP_MUX_NO_BLEND: {
            if (WebPDecodeRGBAInto(webp_state->webp_iterator->fragment.bytes,
                                    webp_state->webp_iterator->fragment.size,
                                    (uint8_t *)webp_state->canvas_image->pixels + webp_state->canvas_image->bytes_per_line * webp_state->frame_y +
                                        webp_state->frame_x * webp_state->bytes_per_pixel,
                                    (size_t)webp_state->canvas_image->bytes_per_line * webp_state->canvas_image->height,
                                    webp_state->canvas_image->bytes_per_line) == NULL) {
                SAIL_LOG_ERROR("WEBP: Failed to decode image");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }
            break;
        }
        case WEBP_MUX_BLEND: {
            if (WebPDecodeRGBAInto(webp_state->webp_iterator->fragment.bytes,
                                    webp_state->webp_iterator->fragment.size,
                                    image->pixels,
                                    (size_t)image->bytes_per_line * image->height,
                                    webp_state->frame_width * webp_state->bytes_per_pixel) == NULL) {
                SAIL_LOG_ERROR("WEBP: Failed to decode image");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            uint8_t *dst_scanline = (uint8_t *)webp_state->canvas_image->pixels + webp_state->frame_y * image->bytes_per_line + webp_state->frame_x * webp_state->bytes_per_pixel;
            uint8_t *src_scanline = image->pixels;

            for (unsigned row = 0; row < webp_state->frame_height; row++, dst_scanline += webp_state->canvas_image->bytes_per_line,
                                                                          src_scanline += webp_state->frame_width * webp_state->bytes_per_pixel) {
                SAIL_TRY(webp_private_blend_over(dst_scanline, 0, src_scanline, webp_state->frame_width, webp_state->bytes_per_pixel));
            }
            break;
        }
        default: {
            SAIL_LOG_ERROR("WEBP: Unknown blending method");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    memcpy(image->pixels, webp_state->canvas_image->pixels, (size_t)image->bytes_per_line * image->height);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_webp(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct webp_state *webp_state = (struct webp_state *)(*state);

    *state = NULL;

    destroy_webp_state(webp_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_webp(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_webp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_webp(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
