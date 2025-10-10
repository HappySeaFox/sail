/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct hdr_codec_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    bool frame_loaded;

    struct hdr_header header;
    struct hdr_write_context write_ctx;
};

static sail_status_t alloc_hdr_codec_state(struct sail_io *io,
                                              const struct sail_load_options *load_options,
                                              const struct sail_save_options *save_options,
                                              struct hdr_codec_state **hdr_codec_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct hdr_codec_state), &ptr));
    *hdr_codec_state = ptr;

    **hdr_codec_state = (struct hdr_codec_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,

        .header = {
            .width        = 0,
            .height       = 0,
            .y_increasing = false,
            .x_increasing = true,
            .exposure     = 1.0f,
            .gamma        = 1.0f,
            .software     = NULL,
            .view         = NULL,
            .primaries    = NULL,
            .colorcorr    = {1.0f, 1.0f, 1.0f},
        },

        .write_ctx = {
            .use_rle = true,
            .header  = &(*hdr_codec_state)->header,
        },
    };

    return SAIL_OK;
}

static void destroy_hdr_codec_state(struct hdr_codec_state *hdr_codec_state) {

    if (hdr_codec_state == NULL) {
        return;
    }

    hdr_private_destroy_header(&hdr_codec_state->header);

    sail_free(hdr_codec_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_hdr(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct hdr_codec_state *hdr_codec_state;
    SAIL_TRY(alloc_hdr_codec_state(io, load_options, NULL, &hdr_codec_state));
    *state = hdr_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_hdr(void *state, struct sail_image **image) {

    struct hdr_codec_state *hdr_state = state;

    if (hdr_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    hdr_state->frame_loaded = true;

    /* Read HDR header. */
    SAIL_TRY(hdr_private_read_header(hdr_state->io, &hdr_state->header));

    SAIL_LOG_TRACE("HDR: %dx%d, Y%s X%s",
                   hdr_state->header.width, hdr_state->header.height,
                   hdr_state->header.y_increasing ? "+" : "-",
                   hdr_state->header.x_increasing ? "+" : "-");

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width = hdr_state->header.width;
    image_local->height = hdr_state->header.height;

    /* HDR uses 32-bit float RGB (96 bits total). */
    image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP96;

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Add source image info if requested. */
    if (hdr_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                           /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = image_local->pixel_format;
        image_local->source_image->compression = SAIL_COMPRESSION_RLE;
    }

    /* Store HDR-specific properties. */
    SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                       /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(hdr_private_store_properties(&hdr_state->header, image_local->special_properties),
                       /* cleanup */ sail_destroy_image(image_local));

    /* Store software in meta_data. */
    if (hdr_state->header.software != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&image_local->meta_data_node),
                           /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_SOFTWARE,
                                                                            &image_local->meta_data_node->meta_data),
                           /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(sail_set_variant_string(image_local->meta_data_node->meta_data->value,
                                                      hdr_state->header.software),
                           /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_hdr(void *state, struct sail_image *image) {

    const struct hdr_codec_state *hdr_codec_state = state;

    float *scanline = NULL;
    void *ptr;

    SAIL_TRY(sail_malloc(hdr_codec_state->header.width * 3 * sizeof(float), &ptr));
    scanline = ptr;

    /* Read scanlines. */
    for (int y = 0; y < hdr_codec_state->header.height; y++) {
        int target_y;

        if (hdr_codec_state->header.y_increasing) {
            target_y = hdr_codec_state->header.height - 1 - y;
        } else {
            target_y = y;
        }

        SAIL_TRY_OR_CLEANUP(hdr_private_read_scanline(hdr_codec_state->io,
                                                        hdr_codec_state->header.width,
                                                        scanline),
                           /* cleanup */ sail_free(scanline));

        /* Copy to image buffer. */
        float *dest = (float *)((uint8_t *)image->pixels + target_y * image->bytes_per_line);

        if (hdr_codec_state->header.x_increasing) {
            memcpy(dest, scanline, hdr_codec_state->header.width * 3 * sizeof(float));
        } else {
            /* Reverse X direction. */
            for (int x = 0; x < hdr_codec_state->header.width; x++) {
                int src_x = hdr_codec_state->header.width - 1 - x;
                dest[x * 3 + 0] = scanline[src_x * 3 + 0];
                dest[x * 3 + 1] = scanline[src_x * 3 + 1];
                dest[x * 3 + 2] = scanline[src_x * 3 + 2];
            }
        }
    }

    sail_free(scanline);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_hdr(void **state) {

    struct hdr_codec_state *hdr_codec_state = *state;

    *state = NULL;

    destroy_hdr_codec_state(hdr_codec_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_hdr(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    *state = NULL;

    /* Allocate a new state. */
    struct hdr_codec_state *hdr_codec_state;
    SAIL_TRY(alloc_hdr_codec_state(io, NULL, save_options, &hdr_codec_state));
    *state = hdr_codec_state;

    /* Handle tuning options. */
    if (hdr_codec_state->save_options != NULL && hdr_codec_state->save_options->tuning != NULL) {
        sail_traverse_hash_map_with_user_data(hdr_codec_state->save_options->tuning,
                                               hdr_private_tuning_key_value_callback,
                                               &hdr_codec_state->write_ctx);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_hdr(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct hdr_codec_state *hdr_codec_state = state;

    if (hdr_codec_state->frame_loaded) {
        SAIL_LOG_ERROR("HDR: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* HDR only supports BPP96 (32-bit float RGB). */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP96) {
        SAIL_LOG_ERROR("HDR: Only BPP96 (32-bit float RGB) pixel format is supported for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    hdr_codec_state->frame_loaded = true;
    hdr_codec_state->header.width = image->width;
    hdr_codec_state->header.height = image->height;

    /* Fetch properties from special_properties. */
    if (image->special_properties != NULL) {
        SAIL_TRY(hdr_private_fetch_properties(image->special_properties, &hdr_codec_state->header));
    }

    /* Write header. */
    SAIL_TRY(hdr_private_write_header(hdr_codec_state->io, &hdr_codec_state->header, image->meta_data_node));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_hdr(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct hdr_codec_state *hdr_codec_state = state;

    /* HDR only supports BPP96 (32-bit float RGB). */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP96) {
        SAIL_LOG_ERROR("HDR: Only BPP96 (32-bit float RGB) pixel format is supported for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Write scanlines. */
    for (int y = 0; y < hdr_codec_state->header.height; y++) {
        const float *scanline = (const float *)((const uint8_t *)image->pixels + y * image->bytes_per_line);

        SAIL_TRY(hdr_private_write_scanline(hdr_codec_state->io,
                                              hdr_codec_state->header.width,
                                              scanline,
                                              hdr_codec_state->write_ctx.use_rle));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_hdr(void **state) {

    SAIL_CHECK_PTR(state);

    struct hdr_codec_state *hdr_codec_state = *state;

    *state = NULL;

    destroy_hdr_codec_state(hdr_codec_state);

    return SAIL_OK;
}
