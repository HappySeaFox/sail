/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific data types.
 */

static const double COMPRESSION_MIN     = 1;
static const double COMPRESSION_MAX     = 9;
static const double COMPRESSION_DEFAULT = 6;

/*
 * Codec-specific state.
 */
struct png_state {
    png_structp png_ptr;
    png_infop info_ptr;
    int color_type;
    int bit_depth;
    int interlace_type;

    struct sail_image *first_image;
    int interlaced_passes;
    bool libpng_error;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;
    bool frame_saved;
    int frames;
    int current_frame;

    /* APNG-specific. */
#ifdef PNG_APNG_SUPPORTED
    bool is_apng;
    unsigned bytes_per_pixel;

    png_uint_32 next_frame_width;
    png_uint_32 next_frame_height;
    png_uint_32 next_frame_x_offset;
    png_uint_32 next_frame_y_offset;
    png_uint_16 next_frame_delay_num;
    png_uint_16 next_frame_delay_den;
    png_byte next_frame_dispose_op;
    png_byte next_frame_blend_op;

    bool skipped_hidden;
    png_bytep *prev;
    /* Temporary scanline to read into. We need it for blending. */
    void *temp_scanline;
    /* Scan line for skipping a first hidden frame. */
    void *scanline_for_skipping;
#endif
};

static sail_status_t alloc_png_state(struct png_state **png_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct png_state), &ptr));
    *png_state = ptr;

    (*png_state)->png_ptr           = NULL;
    (*png_state)->info_ptr          = NULL;
    (*png_state)->color_type        = 0;
    (*png_state)->bit_depth         = 0;
    (*png_state)->interlace_type    = 0;
    (*png_state)->first_image       = NULL;
    (*png_state)->interlaced_passes = 0;
    (*png_state)->libpng_error      = false;
    (*png_state)->load_options      = NULL;
    (*png_state)->save_options      = NULL;
    (*png_state)->frame_saved       = false;
    (*png_state)->frames            = 0;
    (*png_state)->current_frame     = 0;

    /* APNG-specific. */
#ifdef PNG_APNG_SUPPORTED
    (*png_state)->is_apng               = false;
    (*png_state)->bytes_per_pixel       = 0;

    (*png_state)->next_frame_width      = 0;
    (*png_state)->next_frame_height     = 0;
    (*png_state)->next_frame_x_offset   = 0;
    (*png_state)->next_frame_y_offset   = 0;
    (*png_state)->next_frame_delay_num  = 0;
    (*png_state)->next_frame_delay_den  = 0;
    (*png_state)->next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND;
    (*png_state)->next_frame_blend_op   = PNG_BLEND_OP_SOURCE;

    (*png_state)->skipped_hidden        = false;
    (*png_state)->prev                  = NULL;
    (*png_state)->temp_scanline         = NULL;
    (*png_state)->scanline_for_skipping = NULL;
#endif

    return SAIL_OK;
}

static void destroy_png_state(struct png_state *png_state) {

    if (png_state == NULL) {
        return;
    }

    sail_destroy_load_options(png_state->load_options);
    sail_destroy_save_options(png_state->save_options);

#ifdef PNG_APNG_SUPPORTED
    sail_free(png_state->temp_scanline);
    sail_free(png_state->scanline_for_skipping);

    if (png_state->first_image != NULL) {
        png_private_destroy_rows(&png_state->prev, png_state->first_image->height);
    }
#endif

    sail_destroy_image(png_state->first_image);

    sail_free(png_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_png(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &png_state->load_options));

    /* Initialize PNG. */
    if ((png_state->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_private_my_error_fn, png_private_my_warning_fn)) == NULL) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    png_set_read_fn(png_state->png_ptr, io, png_private_my_read_fn);
    png_read_info(png_state->png_ptr, png_state->info_ptr);

    SAIL_TRY(sail_alloc_image(&png_state->first_image));
    SAIL_TRY(sail_alloc_source_image(&png_state->first_image->source_image));

    png_get_IHDR(png_state->png_ptr,
                    png_state->info_ptr,
                    &png_state->first_image->width,
                    &png_state->first_image->height,
                    &png_state->bit_depth,
                    &png_state->color_type,
                    &png_state->interlace_type,
                    /* compression type */ NULL,
                    /* filter method */ NULL);

    /* Pixel format. */
    png_state->first_image->pixel_format = png_private_png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);

    SAIL_TRY(sail_bytes_per_line(png_state->first_image->width,
                                 png_state->first_image->pixel_format,
                                 &png_state->first_image->bytes_per_line));

    /* Fetch palette. */
    if (png_state->color_type == PNG_COLOR_TYPE_PALETTE) {
        SAIL_TRY(png_private_fetch_palette(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->palette));
    }

    /* Fetch resolution. */
    SAIL_TRY(png_private_fetch_resolution(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->resolution));

    png_state->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);

    SAIL_LOG_TRACE("PNG: Interlaced passes: %d", png_state->interlaced_passes);

#ifdef PNG_APNG_SUPPORTED
    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(png_state->first_image->pixel_format, &bits_per_pixel));
    png_state->bytes_per_pixel = bits_per_pixel / 8;

    png_state->is_apng = png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_acTL) != 0;
    png_state->frames = png_state->is_apng ? png_get_num_frames(png_state->png_ptr, png_state->info_ptr) : 1;

    if (png_state->frames == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    if (png_state->is_apng) {
        SAIL_TRY(png_private_alloc_rows(&png_state->prev, png_state->first_image->bytes_per_line, png_state->first_image->height));
    }
#else
    png_state->frames = 1;
#endif

    png_state->first_image->source_image->pixel_format = png_private_png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);
    png_state->first_image->source_image->compression = SAIL_COMPRESSION_DEFLATE;

    if (png_state->interlaced_passes > 1) {
        png_state->first_image->source_image->interlaced = true;
    }

    /* Read meta data. */
    if (png_state->load_options->options & SAIL_OPTION_META_DATA) {
        SAIL_TRY(png_private_fetch_meta_data(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->meta_data_node));
    }

    /* Fetch ICC profile. */
    if (png_state->load_options->options & SAIL_OPTION_ICCP) {
        SAIL_TRY(png_private_fetch_iccp(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->iccp));
    }

    /* Fetch gamma. */
    if (png_get_gAMA(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->gamma) == 0) {
        SAIL_LOG_TRACE("PNG: Failed to read the image gamma so it stays default");
    }

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng) {
        SAIL_TRY(sail_malloc((size_t)png_state->first_image->width * png_state->bytes_per_pixel, &png_state->temp_scanline));
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_png(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (png_state->current_frame == png_state->frames) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_copy_image(png_state->first_image, &image_local));

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng) {
        /* APNG feature: a hidden frame. */
        if (!png_state->skipped_hidden && png_get_first_frame_is_hidden(png_state->png_ptr, png_state->info_ptr)) {
            SAIL_LOG_DEBUG("PNG: Skipping hidden frame");
            SAIL_TRY_OR_CLEANUP(png_private_skip_hidden_frame(png_state->first_image->bytes_per_line,
                                                               png_state->first_image->height,
                                                               png_state->png_ptr,
                                                               png_state->info_ptr,
                                                               &png_state->scanline_for_skipping),
                                /* cleanup */ sail_destroy_image(image_local));

            png_state->frames--;

            /* We have just a single frame left - continue to reading scan lines. */
            if (png_state->frames == 1) {
                png_read_frame_head(png_state->png_ptr, png_state->info_ptr);
                png_state->next_frame_width  = png_state->first_image->width;
                png_state->next_frame_height = png_state->first_image->height;
            } else if (png_state->frames == 0) {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
            }
        }

        png_state->skipped_hidden = true;
        png_read_frame_head(png_state->png_ptr, png_state->info_ptr);

        if (png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_fcTL) != 0) {
            png_get_next_frame_fcTL(png_state->png_ptr, png_state->info_ptr,
                                    &png_state->next_frame_width, &png_state->next_frame_height,
                                    &png_state->next_frame_x_offset, &png_state->next_frame_y_offset,
                                    &png_state->next_frame_delay_num, &png_state->next_frame_delay_den,
                                    &png_state->next_frame_dispose_op, &png_state->next_frame_blend_op);
        } else {
            png_state->next_frame_width      = image_local->width;
            png_state->next_frame_height     = image_local->height;
            png_state->next_frame_x_offset   = 0;
            png_state->next_frame_y_offset   = 0;
            png_state->next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND;
            png_state->next_frame_blend_op   = PNG_BLEND_OP_SOURCE;
        }

        if (png_state->next_frame_width + png_state->next_frame_x_offset > image_local->width ||
                png_state->next_frame_height + png_state->next_frame_y_offset > image_local->height) {
            sail_destroy_image(image_local);
            SAIL_LOG_ERROR("PNG: Frame (%u,%u %ux%u) doesn't fit into the image (%ux%u)",
                            png_state->next_frame_x_offset, png_state->next_frame_y_offset,
                            png_state->next_frame_width, png_state->next_frame_height,
                            image_local->width, image_local->height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS);
        }

        if (!png_state->next_frame_delay_den) {
            png_state->next_frame_delay_den = 100;
        }

        image_local->delay = (int)(((double)png_state->next_frame_delay_num / png_state->next_frame_delay_den) * 1000);
    }
#endif

    png_state->current_frame++;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_png(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (int current_pass = 0; current_pass < png_state->interlaced_passes; current_pass++) {
    #ifdef PNG_APNG_SUPPORTED
        if (png_state->is_apng) {
            for (unsigned row = 0; row < image->height; row++) {
                unsigned char *scanline = (unsigned char *)image->pixels + row * image->bytes_per_line;

                memcpy(scanline, png_state->prev[row], (size_t)png_state->first_image->width * png_state->bytes_per_pixel);

                if (row >= png_state->next_frame_y_offset && row < png_state->next_frame_y_offset + png_state->next_frame_height) {
                    png_read_row(png_state->png_ptr, (png_bytep)png_state->temp_scanline, NULL);

                    /* Copy all pixel values including alpha. */
                    if (png_state->current_frame == 1 || png_state->next_frame_blend_op == PNG_BLEND_OP_SOURCE) {
                        SAIL_TRY(png_private_blend_source(scanline,
                                                png_state->next_frame_x_offset,
                                                png_state->temp_scanline,
                                                png_state->next_frame_width,
                                                png_state->bytes_per_pixel));
                    } else { /* PNG_BLEND_OP_OVER */
                        SAIL_TRY(png_private_blend_over(scanline,
                                            png_state->next_frame_x_offset,
                                            png_state->temp_scanline,
                                            png_state->next_frame_width,
                                            png_state->bytes_per_pixel));
                    }

                    if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_BACKGROUND) {
                        memset(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                                0,
                                (size_t)png_state->next_frame_width * png_state->bytes_per_pixel);
                    } else if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_NONE) {
                        memcpy(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                                scanline,
                                (size_t)png_state->next_frame_width * png_state->bytes_per_pixel);
                    } else { /* PNG_DISPOSE_OP_PREVIOUS */
                    }
                }
            }
        } else {
            for (unsigned row = 0; row < image->height; row++) {
                png_read_row(png_state->png_ptr, (unsigned char *)image->pixels + row * image->bytes_per_line, NULL);
            }
        }
    #else
        for (unsigned row = 0; row < image->height; row++) {
            png_read_row(png_state->png_ptr, (unsigned char *)image->pixels + row * image->bytes_per_line, NULL);
        }
    #endif
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_png(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct png_state *png_state = (struct png_state *)(*state);

    *state = NULL;

    if (png_state->png_ptr != NULL) {
        if (setjmp(png_jmpbuf(png_state->png_ptr))) {
            destroy_png_state(png_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    if (png_state->png_ptr != NULL) {
        png_destroy_read_struct(&png_state->png_ptr, &png_state->info_ptr, NULL);
    }

    destroy_png_state(png_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_png(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy save options. */
    SAIL_TRY(sail_copy_save_options(save_options, &png_state->save_options));

    if (png_state->save_options->compression != SAIL_COMPRESSION_DEFLATE) {
        SAIL_LOG_ERROR("PNG: Only DEFLATE compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Initialize PNG. */
    if ((png_state->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_private_my_error_fn, png_private_my_warning_fn)) == NULL) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Handle tuning. */
    if (png_state->save_options->tuning != NULL) {
        sail_traverse_hash_map_with_user_data(png_state->save_options->tuning, png_private_tuning_key_value_callback, png_state->png_ptr);
    }

    png_set_write_fn(png_state->png_ptr, io, png_private_my_write_fn, png_private_my_flush_fn);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_png(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->frame_saved) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    png_state->frame_saved = true;

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    int color_type;
    int bit_depth;
    SAIL_TRY_OR_CLEANUP(png_private_pixel_format_to_png_color_type(image->pixel_format, &color_type, &bit_depth),
                        /* cleanup */ SAIL_LOG_ERROR("PNG: %s pixel format is not currently supported for saving", sail_pixel_format_to_string(image->pixel_format)));

    /* Save meta data. */
    if (png_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL) {
        SAIL_TRY(png_private_write_meta_data(png_state->png_ptr, png_state->info_ptr, image->meta_data_node));
        SAIL_LOG_DEBUG("PNG: Meta data has been written");
    }

    png_set_IHDR(png_state->png_ptr,
                 png_state->info_ptr,
                 image->width,
                 image->height,
                 bit_depth,
                 color_type,
                 (png_state->save_options->options & SAIL_OPTION_INTERLACED) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    /* Save resolution. */
    SAIL_TRY(png_private_write_resolution(png_state->png_ptr, png_state->info_ptr, image->resolution));

    /* Save ICC profile. */
    if (png_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL) {
        png_set_iCCP(png_state->png_ptr,
                        png_state->info_ptr,
                        "ICC profile",
                        PNG_COMPRESSION_TYPE_BASE,
                        (const png_bytep)image->iccp->data,
                        image->iccp->data_length);

        SAIL_LOG_DEBUG("PNG: ICC profile has been written");
    }

    /* Save palette. */
    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED) {
        if (image->palette == NULL) {
            SAIL_LOG_ERROR("PNG: The indexed image has no palette");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
        }

        if (image->palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB) {
            SAIL_LOG_ERROR("PNG: Only BPP24-RGB palette is currently supported");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }

        /* Deep copy palette. */
        png_set_PLTE(png_state->png_ptr, png_state->info_ptr, image->palette->data, image->palette->color_count);
    }

    /* Save gamma. */
    png_set_gAMA(png_state->png_ptr, png_state->info_ptr, image->gamma);

    /* Set compression. */
    const double compression = (png_state->save_options->compression_level < COMPRESSION_MIN ||
                                png_state->save_options->compression_level > COMPRESSION_MAX)
                                ? COMPRESSION_DEFAULT
                                : png_state->save_options->compression_level;

    png_set_compression_level(png_state->png_ptr, (int)compression);

    png_write_info(png_state->png_ptr, png_state->info_ptr);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR      ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR  ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR) {
        png_set_bgr(png_state->png_ptr);
    }

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB     ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ARGB ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR) {
        png_set_swap_alpha(png_state->png_ptr);
    }

    if (png_state->save_options->options & SAIL_OPTION_INTERLACED) {
        png_state->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);
    } else {
        png_state->interlaced_passes = 1;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_png(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (int current_pass = 0; current_pass < png_state->interlaced_passes; current_pass++) {
        for (unsigned row = 0; row < image->height; row++) {
            png_write_row(png_state->png_ptr, (const unsigned char *)image->pixels + row * image->bytes_per_line);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_png(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct png_state *png_state = (struct png_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Error handling setup. */
    if (png_state->png_ptr != NULL) {
        if (setjmp(png_jmpbuf(png_state->png_ptr))) {
            destroy_png_state(png_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    if (png_state->png_ptr != NULL && !png_state->libpng_error) {
        png_write_end(png_state->png_ptr, png_state->info_ptr);
    }

    if (png_state->png_ptr != NULL) {
        png_destroy_write_struct(&png_state->png_ptr, &png_state->info_ptr);
    }

    destroy_png_state(png_state);

    return SAIL_OK;
}
