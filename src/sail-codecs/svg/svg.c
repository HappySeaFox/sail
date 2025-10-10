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

#ifdef SAIL_RESVG
#include <resvg.h>
#else
#define NANOSVG_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvgrast.h>
#endif

#include <sail-common/sail-common.h>

/*
 * Codec-specific state.
 */
struct svg_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    bool frame_loaded;

#ifdef SAIL_RESVG
    resvg_options* resvg_options;
    resvg_render_tree* resvg_tree;
#else
    NSVGimage* nsvg_image;
    NSVGrasterizer* nsvg_rasterizer;
#endif
};

static sail_status_t alloc_svg_state(const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct svg_state** svg_state)
{

    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct svg_state), &ptr));
    *svg_state = ptr;

    **svg_state = (struct svg_state){
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,

#ifdef SAIL_RESVG
        .resvg_options = NULL,
        .resvg_tree    = NULL,
#else
        .nsvg_image      = NULL,
        .nsvg_rasterizer = NULL,
#endif
    };

    return SAIL_OK;
}

static void destroy_svg_state(struct svg_state* svg_state)
{

    if (svg_state == NULL)
    {
        return;
    }

#ifdef SAIL_RESVG
    if (svg_state->resvg_options != NULL)
    {
        resvg_options_destroy(svg_state->resvg_options);
    }
    if (svg_state->resvg_tree != NULL)
    {
        resvg_tree_destroy(svg_state->resvg_tree);
    }
#else
    nsvgDeleteRasterizer(svg_state->nsvg_rasterizer);
    nsvgDelete(svg_state->nsvg_image);
#endif

    sail_free(svg_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_svg(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{

    *state = NULL;

    /* Allocate a new state. */
    struct svg_state* svg_state;
    SAIL_TRY(alloc_svg_state(load_options, NULL, &svg_state));
    *state = svg_state;

    /* Read the entire image. */
    size_t image_size;
    SAIL_TRY(sail_io_size(io, &image_size));

    void* image_data;
    SAIL_TRY(sail_malloc(image_size + 1, &image_data)); /* Allocate +1 byte for '\0' (for nsvgParse). */

    SAIL_TRY_OR_CLEANUP(sail_io_contents_into_data(io, image_data),
                        /* cleanup */ sail_free(image_data));

#ifdef SAIL_RESVG
    svg_state->resvg_options = resvg_options_create();

    const int result =
        resvg_parse_tree_from_data(image_data, image_size, svg_state->resvg_options, &svg_state->resvg_tree);

    sail_free(image_data);

    if (result != RESVG_OK)
    {
        SAIL_LOG_ERROR("SVG: Failed to load image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }
#else
    ((char*)image_data)[image_size] = '\0';

    svg_state->nsvg_image = nsvgParse(image_data, "px", 96.0f);

    sail_free(image_data);

    if (svg_state->nsvg_image == NULL)
    {
        SAIL_LOG_ERROR("SVG: Failed to load image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    svg_state->nsvg_rasterizer = nsvgCreateRasterizer();

    if (svg_state->nsvg_rasterizer == NULL)
    {
        SAIL_LOG_ERROR("SVG: Failed to create NanoSVG rasterizer");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_svg(void* state, struct sail_image** image)
{

    struct svg_state* svg_state = state;

    if (svg_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    svg_state->frame_loaded = true;

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (svg_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

#ifdef SAIL_RESVG
    const resvg_size image_size = resvg_get_image_size(svg_state->resvg_tree);

    image_local->width  = (unsigned)image_size.width;
    image_local->height = (unsigned)image_size.height;
#else
    image_local->width  = (unsigned)svg_state->nsvg_image->width;
    image_local->height = (unsigned)svg_state->nsvg_image->height;
#endif
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_svg(void* state, struct sail_image* image)
{

    const struct svg_state* svg_state = state;

    memset(image->pixels, 0, (size_t)image->bytes_per_line * image->height);

#ifdef SAIL_RESVG
#ifdef SAIL_HAVE_RESVG_FIT_TO
    const resvg_fit_to resvg_fit_to = {RESVG_FIT_TO_ORIGINAL, 0};
    resvg_render(svg_state->resvg_tree, resvg_fit_to, image->width, image->height, image->pixels);
#else
    resvg_render(svg_state->resvg_tree, resvg_transform_identity(), image->width, image->height, image->pixels);
#endif
#else
    nsvgRasterize(svg_state->nsvg_rasterizer, svg_state->nsvg_image, /* x */ 0, /* y */ 0, /* scale */ 1, image->pixels,
                  (int)image->width, (int)image->height, (int)image->bytes_per_line);
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_svg(void** state)
{

    struct svg_state* svg_state = *state;

    *state = NULL;

    destroy_svg_state(svg_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_svg(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_svg(void* state, const struct sail_image* image)
{

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_svg(void* state, const struct sail_image* image)
{

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_svg(void** state)
{

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
