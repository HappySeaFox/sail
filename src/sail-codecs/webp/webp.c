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
#include <string.h>

#include <webp/decode.h>
#include <webp/demux.h>
#include <webp/encode.h>
#include <webp/mux.h>
#include <webp/mux_types.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct webp_state {
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    /* Loading-specific fields. */
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

    /* Saving-specific fields. */
    struct sail_io *io;
    struct WebPAnimEncoder *anim_encoder;
    int timestamp_ms;
    bool is_first_frame;
    unsigned canvas_width;
    unsigned canvas_height;
};

static sail_status_t alloc_webp_state(struct sail_io *io,
                                        const struct sail_load_options *load_options,
                                        const struct sail_save_options *save_options,
                                        struct webp_state **webp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct webp_state), &ptr));
    *webp_state = ptr;

    **webp_state = (struct webp_state) {
        .load_options = load_options,
        .save_options = save_options,

        .canvas_image         = NULL,
        .webp_demux           = NULL,
        .webp_iterator        = NULL,
        .frame_number         = 0,
        .background_color     = 0,
        .frame_count          = 0,
        .bytes_per_pixel      = 0,
        .frame_x              = 0,
        .frame_y              = 0,
        .frame_width          = 0,
        .frame_height         = 0,
        .frame_dispose_method = WEBP_MUX_DISPOSE_NONE,
        .frame_blend_method   = WEBP_MUX_NO_BLEND,

        .image_data      = NULL,
        .image_data_size = 0,

        .io            = io,
        .anim_encoder  = NULL,
        .timestamp_ms  = 0,
        .is_first_frame = true,
        .canvas_width  = 0,
        .canvas_height = 0,
    };

    return SAIL_OK;
}

static void destroy_webp_state(struct webp_state *webp_state) {

    if (webp_state == NULL) {
        return;
    }

    /* Load-specific cleanup. */
    if (webp_state->webp_iterator != NULL) {
        WebPDemuxReleaseIterator(webp_state->webp_iterator);
        sail_free(webp_state->webp_iterator);
    }

    sail_free(webp_state->image_data);

    WebPDemuxDelete(webp_state->webp_demux);

    sail_destroy_image(webp_state->canvas_image);

    /* Save-specific cleanup. */
    if (webp_state->anim_encoder != NULL) {
        WebPAnimEncoderDelete(webp_state->anim_encoder);
    }

    sail_free(webp_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_webp(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct webp_state *webp_state;
    SAIL_TRY(alloc_webp_state(io, load_options, NULL, &webp_state));
    *state = webp_state;

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

    /* Get bitstream features. */
    struct WebPBitstreamFeatures features;
    if (WebPGetFeatures(webp_state->image_data, webp_state->image_data_size, &features) != VP8_STATUS_OK) {
        SAIL_LOG_ERROR("WEBP: Failed to get bitstream features");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Validate format flags. */
    const uint32_t format_flags = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_FORMAT_FLAGS);

    if (features.has_alpha && !(format_flags & ALPHA_FLAG)) {
        SAIL_LOG_WARNING("WEBP: Bitstream has alpha channel but ALPHA_FLAG is not set");
    }
    if (!features.has_alpha && (format_flags & ALPHA_FLAG)) {
        SAIL_LOG_WARNING("WEBP: ALPHA_FLAG is set but bitstream has no alpha channel");
    }
    if (features.has_animation && !(format_flags & ANIMATION_FLAG)) {
        SAIL_LOG_WARNING("WEBP: Bitstream has animation but ANIMATION_FLAG is not set");
    }
    if (!features.has_animation && (format_flags & ANIMATION_FLAG)) {
        SAIL_LOG_WARNING("WEBP: ANIMATION_FLAG is set but bitstream has no animation");
    }

    /* Construct a canvas image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (webp_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        /* Set source image properties based on bitstream features. */
        if (features.format == 1) {
            /* Lossy (VP8). */
            image_local->source_image->pixel_format = features.has_alpha
                                                        ? SAIL_PIXEL_FORMAT_BPP32_YUVA
                                                        : SAIL_PIXEL_FORMAT_BPP24_YUV;
            image_local->source_image->chroma_subsampling = SAIL_CHROMA_SUBSAMPLING_420;
        } else if (features.format == 2) {
            /* Lossless (VP8L). */
            image_local->source_image->pixel_format = features.has_alpha
                                                        ? SAIL_PIXEL_FORMAT_BPP32_RGBA
                                                        : SAIL_PIXEL_FORMAT_BPP24_RGB;
            image_local->source_image->chroma_subsampling = SAIL_CHROMA_SUBSAMPLING_444;
        } else {
            /* Mixed or undefined format. */
            image_local->source_image->pixel_format = features.has_alpha
                                                        ? SAIL_PIXEL_FORMAT_BPP32_RGBA
                                                        : SAIL_PIXEL_FORMAT_BPP24_RGB;
            image_local->source_image->chroma_subsampling = SAIL_CHROMA_SUBSAMPLING_UNKNOWN;
        }

        image_local->source_image->compression = SAIL_COMPRESSION_WEBP;
    }

    image_local->width          = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_CANVAS_WIDTH);
    image_local->height         = WebPDemuxGetI(webp_state->webp_demux, WEBP_FF_CANVAS_HEIGHT);
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

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

        /* Store loop count for animated images. */
        SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(webp_private_store_loop_count(webp_state->webp_demux, image_local->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    webp_state->canvas_image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_webp(void *state, struct sail_image **image) {

    struct webp_state *webp_state = state;

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

    if (webp_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        image_local->source_image->pixel_format = webp_state->webp_iterator->has_alpha
                                                    ? SAIL_PIXEL_FORMAT_BPP32_YUVA
                                                    : SAIL_PIXEL_FORMAT_BPP24_YUV;
    }

    if (webp_state->frame_count > 1) {
        /* Fall back to 100 ms. when the duration is <= 0. */
        image_local->delay = webp_state->webp_iterator->duration <= 0 ? 100 : webp_state->webp_iterator->duration;
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_webp(void *state, struct sail_image *image) {

    struct webp_state *webp_state = state;

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

            uint8_t *dst_scanline = (uint8_t *)sail_scan_line(webp_state->canvas_image, webp_state->frame_y) + webp_state->frame_x * webp_state->bytes_per_pixel;
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

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_webp(void **state) {

    struct webp_state *webp_state = *state;

    *state = NULL;

    destroy_webp_state(webp_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_webp(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    *state = NULL;

    struct webp_state *webp_state;
    SAIL_TRY(alloc_webp_state(io, NULL, save_options, &webp_state));
    *state = webp_state;

    /* Validate compression. */
    if (webp_state->save_options->compression != SAIL_COMPRESSION_WEBP) {
        SAIL_LOG_ERROR("WEBP: Only WEBP compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_webp(void *state, const struct sail_image *image) {

    struct webp_state *webp_state = state;

    /* Validate pixel format. */
    SAIL_TRY(webp_private_supported_write_pixel_format(image->pixel_format));

    /* First frame - remember canvas dimensions. */
    if (webp_state->is_first_frame) {
        webp_state->canvas_width = image->width;
        webp_state->canvas_height = image->height;
    }

    /* Validate dimensions match canvas. */
    if (image->width != webp_state->canvas_width || image->height != webp_state->canvas_height) {
        SAIL_LOG_ERROR("WEBP: All frames must have the same dimensions (%ux%u)", webp_state->canvas_width, webp_state->canvas_height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_webp(void *state, const struct sail_image *image) {

    struct webp_state *webp_state = state;

    /* Determine if this is an animation (has delay) or static image. */
    const bool is_animation = (image->delay >= 0);

    /* For animations, initialize WebPAnimEncoder on first frame. */
    if (is_animation && webp_state->is_first_frame) {
        struct WebPAnimEncoderOptions anim_options;
        if (!WebPAnimEncoderOptionsInit(&anim_options)) {
            SAIL_LOG_ERROR("WEBP: Failed to initialize animation encoder options");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Set animation options. */
        anim_options.anim_params.loop_count = 0; /* Default: infinite loop. */
        anim_options.minimize_size = 1;
        anim_options.allow_mixed = 1; /* Allow mixed lossy/lossless. */

        webp_state->anim_encoder = WebPAnimEncoderNew(image->width, image->height, &anim_options);
        if (webp_state->anim_encoder == NULL) {
            SAIL_LOG_ERROR("WEBP: Failed to create animation encoder");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    /* Initialize WebPConfig. */
    struct WebPConfig config;
    if (!WebPConfigInit(&config)) {
        SAIL_LOG_ERROR("WEBP: Failed to initialize WebP config");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Set quality from compression level (0-100). */
    float quality = 75.0f; /* Default quality. */
    if (webp_state->save_options->compression_level >= 0 && webp_state->save_options->compression_level <= 100) {
        quality = (float)webp_state->save_options->compression_level;
    }

    config.quality = quality;
    config.method = 4; /* Compression method (0=fast, 6=slow). */

    if (!WebPValidateConfig(&config)) {
        SAIL_LOG_ERROR("WEBP: Invalid WebP config");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Initialize WebPPicture. */
    struct WebPPicture picture;
    if (!WebPPictureInit(&picture)) {
        SAIL_LOG_ERROR("WEBP: Failed to initialize WebP picture");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    picture.width = image->width;
    picture.height = image->height;
    picture.use_argb = 1; /* Use ARGB format for encoding. */

    /* Import pixels. */
    SAIL_TRY_OR_CLEANUP(webp_private_import_pixels(&picture, image),
                        /* cleanup */ WebPPictureFree(&picture));

    /* Encode frame. */
    if (is_animation) {
        /* Add frame to animation encoder. */
        if (!WebPAnimEncoderAdd(webp_state->anim_encoder, &picture, webp_state->timestamp_ms, &config)) {
            WebPPictureFree(&picture);
            SAIL_LOG_ERROR("WEBP: Failed to add frame to animation");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        webp_state->timestamp_ms += (image->delay > 0) ? image->delay : 100;
    } else {
        /* Static image - use WebPMemoryWriter to accumulate data. */
        struct WebPMemoryWriter writer;
        WebPMemoryWriterInit(&writer);

        picture.writer = WebPMemoryWrite;
        picture.custom_ptr = &writer;

        if (!WebPEncode(&config, &picture)) {
            WebPMemoryWriterClear(&writer);
            WebPPictureFree(&picture);
            SAIL_LOG_ERROR("WEBP: Failed to encode image");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Write encoded data to output. */
        SAIL_TRY_OR_CLEANUP(webp_state->io->strict_write(webp_state->io->stream, writer.mem, writer.size),
                            /* cleanup */ WebPMemoryWriterClear(&writer);
                                          WebPPictureFree(&picture));

        WebPMemoryWriterClear(&writer);
    }

    WebPPictureFree(&picture);

    webp_state->is_first_frame = false;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_webp(void **state) {

    struct webp_state *webp_state = *state;

    *state = NULL;

    /* For animations, finalize and write the output. */
    if (webp_state->anim_encoder != NULL) {
        /* Add NULL frame to signal end of animation. */
        if (!WebPAnimEncoderAdd(webp_state->anim_encoder, NULL, webp_state->timestamp_ms, NULL)) {
            destroy_webp_state(webp_state);
            SAIL_LOG_ERROR("WEBP: Failed to finalize animation");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Assemble animation into WebPData. */
        struct WebPData webp_data;
        WebPDataInit(&webp_data);

        if (!WebPAnimEncoderAssemble(webp_state->anim_encoder, &webp_data)) {
            WebPDataClear(&webp_data);
            destroy_webp_state(webp_state);
            SAIL_LOG_ERROR("WEBP: Failed to assemble animation");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Write animation data to output. */
        SAIL_TRY_OR_CLEANUP(webp_state->io->strict_write(webp_state->io->stream, webp_data.bytes, webp_data.size),
                            /* cleanup */ WebPDataClear(&webp_data);
                                          destroy_webp_state(webp_state));

        WebPDataClear(&webp_data);
    }

    destroy_webp_state(webp_state);

    return SAIL_OK;
}
