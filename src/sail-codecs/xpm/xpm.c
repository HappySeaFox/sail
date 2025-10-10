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

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct xpm_codec_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    bool frame_loaded;

    unsigned width;
    unsigned height;
    unsigned num_colors;
    unsigned cpp;
    int x_hotspot;
    int y_hotspot;

    struct xpm_color *colors;
    bool has_transparency;

    struct xpm_state tuning_state;
};

static sail_status_t alloc_xpm_codec_state(struct sail_io *io,
                                           const struct sail_load_options *load_options,
                                           const struct sail_save_options *save_options,
                                           struct xpm_codec_state **xpm_codec_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct xpm_codec_state), &ptr));
    *xpm_codec_state = ptr;

    **xpm_codec_state = (struct xpm_codec_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,

        .width = 0,
        .height = 0,
        .num_colors = 0,
        .cpp = 0,
        .x_hotspot = -1,
        .y_hotspot = -1,

        .colors = NULL,
        .has_transparency = false,
    };

    (*xpm_codec_state)->tuning_state.var_name[0] = '\0';

    return SAIL_OK;
}

static void destroy_xpm_codec_state(struct xpm_codec_state *xpm_codec_state) {

    if (xpm_codec_state == NULL) {
        return;
    }

    sail_free(xpm_codec_state->colors);
    sail_free(xpm_codec_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_xpm(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct xpm_codec_state *xpm_codec_state;
    SAIL_TRY(alloc_xpm_codec_state(io, load_options, NULL, &xpm_codec_state));
    *state = xpm_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_xpm(void *state, struct sail_image **image) {

    struct xpm_codec_state *xpm_state = state;

    if (xpm_state->frame_loaded) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    xpm_state->frame_loaded = true;

    /* Parse XPM header. */
    SAIL_TRY(xpm_private_parse_xpm_header(xpm_state->io,
                                          &xpm_state->width,
                                          &xpm_state->height,
                                          &xpm_state->num_colors,
                                          &xpm_state->cpp,
                                          &xpm_state->x_hotspot,
                                          &xpm_state->y_hotspot));

    SAIL_LOG_TRACE("XPM: %ux%u, %u colors, %u cpp", xpm_state->width, xpm_state->height,
                   xpm_state->num_colors, xpm_state->cpp);

    /* Parse colors. */
    SAIL_TRY(xpm_private_parse_colors(xpm_state->io,
                                      xpm_state->num_colors,
                                      xpm_state->cpp,
                                      &xpm_state->colors,
                                      &xpm_state->has_transparency));

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width = xpm_state->width;
    image_local->height = xpm_state->height;

    /* Determine pixel format. */
    image_local->pixel_format = xpm_private_determine_pixel_format(xpm_state->num_colors,
                                                                    xpm_state->has_transparency);
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Create palette for indexed formats. */
    if (image_local->pixel_format != SAIL_PIXEL_FORMAT_BPP32_RGBA &&
        image_local->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB) {
        SAIL_TRY_OR_CLEANUP(xpm_private_build_palette(&image_local->palette,
                                                      xpm_state->colors,
                                                      xpm_state->num_colors),
                           /* cleanup */ sail_destroy_image(image_local));
    }

    /* Add source image info if requested. */
    if (xpm_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                           /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = image_local->pixel_format;
        image_local->source_image->compression = SAIL_COMPRESSION_NONE;
    }

    /* Store hotspot in special properties if present. */
    if (xpm_state->x_hotspot >= 0 && xpm_state->y_hotspot >= 0) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                           /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(xpm_private_store_hotspot(xpm_state->x_hotspot,
                                                      xpm_state->y_hotspot,
                                                      image_local->special_properties),
                           /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_xpm(void *state, struct sail_image *image) {

    const struct xpm_codec_state *xpm_codec_state = state;

    /* Read pixel data. */
    SAIL_TRY(xpm_private_read_pixels(xpm_codec_state->io,
                                     xpm_codec_state->width,
                                     xpm_codec_state->height,
                                     xpm_codec_state->cpp,
                                     xpm_codec_state->colors,
                                     xpm_codec_state->num_colors,
                                     image->pixels,
                                     image->pixel_format));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_xpm(void **state) {

    struct xpm_codec_state *xpm_codec_state = *state;

    *state = NULL;

    destroy_xpm_codec_state(xpm_codec_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_xpm(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    *state = NULL;

    /* Allocate a new state. */
    struct xpm_codec_state *xpm_codec_state;
    SAIL_TRY(alloc_xpm_codec_state(io, NULL, save_options, &xpm_codec_state));
    *state = xpm_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_xpm(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xpm_codec_state *xpm_codec_state = state;

    if (xpm_codec_state->frame_loaded) {
        SAIL_LOG_ERROR("XPM: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* XPM supports only indexed formats up to 256 colors. */
    unsigned num_colors = 0;
    int transparency_index = -1;

    switch (image->pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED: num_colors = 2; break;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED: num_colors = 4; break;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED: num_colors = 16; break;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED: num_colors = 256; break;
        default: {
            SAIL_LOG_ERROR("XPM: Only indexed pixel formats are supported for saving, got %s",
                          sail_pixel_format_to_string(image->pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    /* Check if we have a palette. */
    if (image->palette == NULL) {
        SAIL_LOG_ERROR("XPM: Palette is required for indexed images");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
    }

    /* Limit to actual palette size. */
    if (image->palette->color_count < num_colors) {
        num_colors = image->palette->color_count;
    }

    /* Check for transparency in palette. */
    bool has_transparency = false;
    SAIL_TRY(xpm_private_check_transparency(image->palette,
                                            num_colors,
                                            &has_transparency,
                                            &transparency_index));

    /* Calculate characters per pixel needed. */
    unsigned cpp = 1;
    unsigned max_colors = 92; /* Length of XPM_CHARS. */
    while (max_colors < num_colors) {
        cpp++;
        max_colors *= 92;
    }

    if (cpp > 7) {
        SAIL_LOG_ERROR("XPM: Too many colors (%u) for XPM format", num_colors);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Process tuning options. */
    if (xpm_codec_state->save_options->tuning != NULL) {
        sail_traverse_hash_map_with_user_data(xpm_codec_state->save_options->tuning,
                                              xpm_private_tuning_key_value_callback,
                                              &xpm_codec_state->tuning_state);
    }

    /* Extract hotspot from special properties if present. */
    int x_hotspot, y_hotspot;
    SAIL_TRY(xpm_private_fetch_hotspot(image->special_properties,
                                       &x_hotspot,
                                       &y_hotspot));

    /* Write XPM header. */
    const char *name = (xpm_codec_state->tuning_state.var_name[0] != '\0') ?
                       xpm_codec_state->tuning_state.var_name : NULL;

    SAIL_TRY(xpm_private_write_header(xpm_codec_state->io,
                                      image->width,
                                      image->height,
                                      num_colors,
                                      cpp,
                                      name,
                                      x_hotspot,
                                      y_hotspot));

    /* Convert palette to RGB if needed. */
    unsigned char *rgb_palette = NULL;
    SAIL_TRY(xpm_private_convert_palette_to_rgb(image->palette->data,
                                                image->palette->pixel_format,
                                                num_colors,
                                                &rgb_palette));

    const unsigned char *palette_to_use = (rgb_palette != NULL) ? rgb_palette : image->palette->data;

    /* Write colors. */
    SAIL_TRY_OR_CLEANUP(xpm_private_write_colors(xpm_codec_state->io,
                                                 palette_to_use,
                                                 num_colors,
                                                 cpp,
                                                 has_transparency,
                                                 transparency_index),
                       /* cleanup */ sail_free(rgb_palette));

    sail_free(rgb_palette);

    xpm_codec_state->width = image->width;
    xpm_codec_state->height = image->height;
    xpm_codec_state->num_colors = num_colors;
    xpm_codec_state->cpp = cpp;
    xpm_codec_state->frame_loaded = true;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_xpm(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xpm_codec_state *xpm_codec_state = state;

    /* Write pixel data. */
    SAIL_TRY(xpm_private_write_pixels(xpm_codec_state->io,
                                      image->pixels,
                                      xpm_codec_state->width,
                                      xpm_codec_state->height,
                                      xpm_codec_state->cpp,
                                      xpm_codec_state->num_colors));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_xpm(void **state) {

    SAIL_CHECK_PTR(state);

    struct xpm_codec_state *xpm_codec_state = *state;

    *state = NULL;

    destroy_xpm_codec_state(xpm_codec_state);

    return SAIL_OK;
}
