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
struct xwd_codec_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    bool frame_loaded;

    struct XWDFileHeader header;
    struct XWDColor *colormap;
};

static sail_status_t alloc_xwd_codec_state(struct sail_io *io,
                                           const struct sail_load_options *load_options,
                                           const struct sail_save_options *save_options,
                                           struct xwd_codec_state **xwd_codec_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct xwd_codec_state), &ptr));
    *xwd_codec_state = ptr;

    **xwd_codec_state = (struct xwd_codec_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,

        .colormap = NULL,
    };

    memset(&(*xwd_codec_state)->header, 0, sizeof(struct XWDFileHeader));

    return SAIL_OK;
}

static void destroy_xwd_codec_state(struct xwd_codec_state *xwd_codec_state) {

    if (xwd_codec_state == NULL) {
        return;
    }

    sail_free(xwd_codec_state->colormap);
    sail_free(xwd_codec_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_xwd(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct xwd_codec_state *xwd_codec_state;
    SAIL_TRY(alloc_xwd_codec_state(io, load_options, NULL, &xwd_codec_state));
    *state = xwd_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_xwd(void *state, struct sail_image **image) {

    struct xwd_codec_state *xwd_state = state;

    if (xwd_state->frame_loaded) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    xwd_state->frame_loaded = true;

    /* Read XWD header. */
    SAIL_TRY(xwd_private_read_header(xwd_state->io, &xwd_state->header));

    SAIL_LOG_TRACE("XWD: %ux%u, depth %u, %u bpp, visual class %u",
                   xwd_state->header.pixmap_width, xwd_state->header.pixmap_height,
                   xwd_state->header.pixmap_depth, xwd_state->header.bits_per_pixel,
                   xwd_state->header.visual_class);

    /* Read colormap if present. */
    bool byte_swap = !xwd_private_is_native_byte_order(xwd_state->header.byte_order);
    SAIL_TRY(xwd_private_read_colormap(xwd_state->io,
                                       xwd_state->header.ncolors,
                                       byte_swap,
                                       &xwd_state->colormap));

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width = xwd_state->header.pixmap_width;
    image_local->height = xwd_state->header.pixmap_height;

    /* Determine pixel format. */
    image_local->pixel_format = xwd_private_pixel_format_from_header(&xwd_state->header);

    if (image_local->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_destroy_image(image_local);
        SAIL_LOG_ERROR("XWD: Unsupported pixel format combination: format=%u, depth=%u, bpp=%u, visual=%u",
                      xwd_state->header.pixmap_format, xwd_state->header.pixmap_depth,
                      xwd_state->header.bits_per_pixel, xwd_state->header.visual_class);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Create palette for indexed formats. */
    if (xwd_state->colormap != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette(&image_local->palette),
                           /* cleanup */ sail_destroy_image(image_local));

        image_local->palette->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
        image_local->palette->color_count = xwd_state->header.ncolors;

        SAIL_TRY_OR_CLEANUP(sail_malloc(image_local->palette->color_count * 3, &image_local->palette->data),
                           /* cleanup */ sail_destroy_image(image_local));

        /* Convert XWD colormap to RGB palette. */
        unsigned char *pal = image_local->palette->data;
        for (uint32_t i = 0; i < xwd_state->header.ncolors; i++) {
            /* XWD uses 16-bit values (0-65535), convert to 8-bit. */
            pal[i * 3 + 0] = (unsigned char)(xwd_state->colormap[i].red   / 257);
            pal[i * 3 + 1] = (unsigned char)(xwd_state->colormap[i].green / 257);
            pal[i * 3 + 2] = (unsigned char)(xwd_state->colormap[i].blue  / 257);
        }
    }

    /* Add source image info if requested. */
    if (xwd_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                           /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = image_local->pixel_format;
        image_local->source_image->compression = SAIL_COMPRESSION_NONE;
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_xwd(void *state, struct sail_image *image) {

    const struct xwd_codec_state *xwd_codec_state = state;

    /* Read pixel data. */
    SAIL_TRY(xwd_private_read_pixels(xwd_codec_state->io,
                                     &xwd_codec_state->header,
                                     xwd_codec_state->colormap,
                                     image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_xwd(void **state) {

    struct xwd_codec_state *xwd_codec_state = *state;

    *state = NULL;

    destroy_xwd_codec_state(xwd_codec_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_xwd(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    *state = NULL;

    /* Allocate a new state. */
    struct xwd_codec_state *xwd_codec_state;
    SAIL_TRY(alloc_xwd_codec_state(io, NULL, save_options, &xwd_codec_state));
    *state = xwd_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_xwd(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xwd_codec_state *xwd_codec_state = state;

    if (xwd_codec_state->frame_loaded) {
        SAIL_LOG_ERROR("XWD: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Create XWD header from image. */
    SAIL_TRY(xwd_private_header_from_image(image, &xwd_codec_state->header));

    /* Write XWD header. */
    SAIL_TRY(xwd_private_write_header(xwd_codec_state->io, &xwd_codec_state->header));

    /* Convert and write colormap if needed. */
    if (image->palette != NULL) {
        uint32_t ncolors;
        SAIL_TRY(xwd_private_palette_to_colormap(image->palette,
                                                 &xwd_codec_state->colormap,
                                                 &ncolors));

        SAIL_TRY(xwd_private_write_colormap(xwd_codec_state->io,
                                            xwd_codec_state->colormap,
                                            ncolors));
    }

    xwd_codec_state->frame_loaded = true;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_xwd(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xwd_codec_state *xwd_codec_state = state;

    /* Write pixel data. */
    SAIL_TRY(xwd_private_write_pixels(xwd_codec_state->io,
                                      &xwd_codec_state->header,
                                      image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_xwd(void **state) {

    SAIL_CHECK_PTR(state);

    struct xwd_codec_state *xwd_codec_state = *state;

    *state = NULL;

    destroy_xwd_codec_state(xwd_codec_state);

    return SAIL_OK;
}
