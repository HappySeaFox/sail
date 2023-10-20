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
#include <stddef.h>
#include <string.h>

#include <resvg.h>

#include "sail-common.h"

/*
 * Codec-specific state.
 */
struct svg_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;
    resvg_options *resvg_options;
    resvg_render_tree *resvg_tree;
};

static sail_status_t alloc_svg_state(struct svg_state **svg_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct svg_state), &ptr));
    *svg_state = ptr;

    (*svg_state)->load_options = NULL;
    (*svg_state)->save_options = NULL;

    (*svg_state)->frame_loaded  = false;
    (*svg_state)->resvg_options = NULL;
    (*svg_state)->resvg_tree    = NULL;

    return SAIL_OK;
}

static void destroy_svg_state(struct svg_state *svg_state) {

    if (svg_state == NULL) {
        return;
    }

    sail_destroy_load_options(svg_state->load_options);
    sail_destroy_save_options(svg_state->save_options);

    if (svg_state->resvg_options != NULL) {
        resvg_options_destroy(svg_state->resvg_options);
    }

    if (svg_state->resvg_tree != NULL) {
        resvg_tree_destroy(svg_state->resvg_tree);
    }

    sail_free(svg_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_svg(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct svg_state *svg_state;
    SAIL_TRY(alloc_svg_state(&svg_state));
    *state = svg_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &svg_state->load_options));

    /* Read the entire image as the resvg API requires. */
    void *image_data;
    size_t image_size;
    SAIL_TRY(sail_alloc_data_from_io_contents(io, &image_data, &image_size));

    svg_state->resvg_options = resvg_options_create();

    const int result = resvg_parse_tree_from_data(image_data, image_size, svg_state->resvg_options, &svg_state->resvg_tree);

    sail_free(image_data);

    if (result != RESVG_OK) {
        SAIL_LOG_ERROR("SVG: Failed to load image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_svg(void *state, struct sail_image **image) {

    struct svg_state *svg_state = state;

    if (svg_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    svg_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (svg_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

    const resvg_size image_size = resvg_get_image_size(svg_state->resvg_tree);

    image_local->width          = (unsigned)image_size.width;
    image_local->height         = (unsigned)image_size.height;
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_svg(void *state, struct sail_image *image) {

    const struct svg_state *svg_state = state;

    memset(image->pixels, 0, (size_t)image->bytes_per_line * image->height);

    const resvg_fit_to resvg_fit_to = { RESVG_FIT_TO_ORIGINAL, 0 };

    resvg_render(svg_state->resvg_tree, resvg_fit_to, image->width, image->height, image->pixels);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_svg(void **state) {

    struct svg_state *svg_state = *state;

    *state = NULL;

    destroy_svg_state(svg_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_svg(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_svg(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_svg(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_svg(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
