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
 * Plugin-specific data types.
 */

static const int COMPRESSION_MIN     = 1;
static const int COMPRESSION_MAX     = 9;
static const int COMPRESSION_DEFAULT = 6;

/*
 * Plugin-specific state.
 */
struct png_state {
    png_structp png_ptr;
    png_infop info_ptr;
    int color_type;
    int bit_depth;
    int interlace_type;

    struct sail_image *first_image;
    struct sail_iccp *iccp;
    bool libpng_error;
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;
    bool frame_written;
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

static int alloc_png_state(struct png_state **png_state) {

    *png_state = (struct png_state *)malloc(sizeof(struct png_state));

    if (*png_state == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*png_state)->png_ptr        = NULL;
    (*png_state)->info_ptr       = NULL;
    (*png_state)->color_type     = 0;
    (*png_state)->bit_depth      = 0;
    (*png_state)->interlace_type = 0;
    (*png_state)->first_image    = NULL;
    (*png_state)->iccp           = NULL;
    (*png_state)->libpng_error   = false;
    (*png_state)->read_options   = NULL;
    (*png_state)->write_options  = NULL;
    (*png_state)->frame_written  = false;
    (*png_state)->frames         = 0;
    (*png_state)->current_frame  = 0;

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

    return 0;
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_error_t sail_plugin_read_init_v3(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    SAIL_TRY(supported_read_output_pixel_format(read_options->output_pixel_format));

    /* Allocate a new state. */
    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &png_state->read_options));

    /* Initialize PNG. */
    if ((png_state->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, my_error_fn, my_warning_fn)) == NULL) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    png_set_read_fn(png_state->png_ptr, io, my_read_fn);
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

    /* Transform the PNG stream. */
    if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        /* Expand 1, 2, and 4 bpp images to 8 bpp. */
        if (png_state->color_type == PNG_COLOR_TYPE_GRAY && png_state->bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png_state->png_ptr);
            png_state->first_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        } else {
            png_state->first_image->pixel_format = png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);
        }

        /* Save palette. */
        if (png_state->color_type == PNG_COLOR_TYPE_PALETTE) {
            int palette_color_count;
            png_colorp palette;

            if (png_get_PLTE(png_state->png_ptr, png_state->info_ptr, &palette, &palette_color_count) == 0) {
                SAIL_LOG_ERROR("The indexed image has no palette");
                return SAIL_MISSING_PALETTE;
            }

            /* Always use RGB palette. */
            SAIL_TRY(sail_alloc_palette(&png_state->first_image->palette));
            png_state->first_image->palette->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
            png_state->first_image->palette->color_count = palette_color_count;
            png_state->first_image->palette->data = malloc(palette_color_count * 3);

            if (png_state->first_image->palette->data == NULL) {
                return SAIL_MEMORY_ALLOCATION_FAILED;
            }

            unsigned char *palette_ptr = png_state->first_image->palette->data;

            for (int i = 0; i < palette_color_count; i++) {
                *palette_ptr++ = palette[i].red;
                *palette_ptr++ = palette[i].green;
                *palette_ptr++ = palette[i].blue;
            }
        }
    } else {
        if (png_state->bit_depth == 16) {
            png_set_strip_16(png_state->png_ptr);
        }

        /* Unpack packed pixels. */
        if (png_state->bit_depth < 8) {
            png_set_packing(png_state->png_ptr);
        }

        if (png_state->color_type == PNG_COLOR_TYPE_GRAY && png_state->bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png_state->png_ptr);
        }

        if (png_state->color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png_state->png_ptr);
        }

        if (png_state->color_type == PNG_COLOR_TYPE_GRAY || png_state->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png_state->png_ptr);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB ||
                 png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR) {
            png_set_swap_alpha(png_state->png_ptr);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
            png_set_bgr(png_state->png_ptr);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
            png_set_filler(png_state->png_ptr, 0xff, PNG_FILLER_AFTER);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR) {
            png_set_filler(png_state->png_ptr, 0xff, PNG_FILLER_BEFORE);
        }

        if (png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_tRNS) != 0) {
            png_set_tRNS_to_alpha(png_state->png_ptr);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) {
            png_set_strip_alpha(png_state->png_ptr);
        }

        png_state->first_image->pixel_format = png_state->read_options->output_pixel_format;
    }

    png_state->first_image->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);

    SAIL_TRY(sail_bytes_per_line(png_state->first_image->width,
                                 png_state->first_image->pixel_format,
                                 &png_state->first_image->bytes_per_line));
    /* Apply requested transformations. */
    png_read_update_info(png_state->png_ptr, png_state->info_ptr);

#ifdef PNG_APNG_SUPPORTED
    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(png_state->first_image->pixel_format, &bits_per_pixel));
    png_state->bytes_per_pixel = bits_per_pixel / 8;

    png_state->is_apng = png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_acTL) != 0;
    png_state->frames = png_state->is_apng ? png_get_num_frames(png_state->png_ptr, png_state->info_ptr) : 1;

    if (png_state->frames == 0) {
        return SAIL_NO_MORE_FRAMES;
    }

    if (png_state->is_apng) {
        SAIL_TRY(alloc_rows(&png_state->prev, png_state->first_image->bytes_per_line, png_state->first_image->height));
    }
#else
    png_state->frames = 1;
#endif

    png_state->first_image->source_image->pixel_format = png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);

    if (png_state->first_image->interlaced_passes > 1) {
        png_state->first_image->source_image->properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
    }

    /* Read meta info. */
    if (png_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        SAIL_TRY(read_png_text(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->meta_entry_node));
    }

    /* Read ICC profile. */
    if (png_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        SAIL_TRY(fetch_iccp(png_state->png_ptr, png_state->info_ptr, &png_state->iccp));
    }

#ifdef PNG_APNG_SUPPORTED
    png_state->temp_scanline = malloc(png_state->first_image->width * png_state->bytes_per_pixel);

    if (png_state->temp_scanline == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }
#endif

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(png_state->first_image->source_image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(png_state->read_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Output pixel format is %s", pixel_format_str);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_frame_v3(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE_PTR(image);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (png_state->current_frame == png_state->frames) {
        return SAIL_NO_MORE_FRAMES;
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        sail_destroy_image(*image);
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    SAIL_TRY(sail_copy_image(png_state->first_image, image));

    /* Only the first frame can have ICCP (if any). */
    if (png_state->current_frame == 0) {
        if (png_state->iccp != NULL) {
            SAIL_TRY(sail_copy_iccp(png_state->iccp, &(*image)->iccp));
        }
    }

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng) {
        (*image)->animated = true;

        /* APNG feature: a hidden frame. */
        if (!png_state->skipped_hidden && png_get_first_frame_is_hidden(png_state->png_ptr, png_state->info_ptr)) {
            SAIL_LOG_DEBUG("PNG: Skipping hidden frame");
            SAIL_TRY(skip_hidden_frame(png_state->first_image->bytes_per_line,
                                       png_state->first_image->height,
                                       png_state->png_ptr,
                                       png_state->info_ptr,
                                       &png_state->scanline_for_skipping));

            png_state->skipped_hidden = true;
            png_state->frames--;

            /* We have just a single frame left - continue to reading scan lines. */
            if (png_state->frames == 1) {
                png_read_frame_head(png_state->png_ptr, png_state->info_ptr);
                (*image)->animated = false;

                png_state->next_frame_width  = png_state->first_image->width;
                png_state->next_frame_height = png_state->first_image->height;
            } else if (png_state->frames == 0) {
                return SAIL_NO_MORE_FRAMES;
            }
        } else {
            png_state->skipped_hidden = true;
            png_read_frame_head(png_state->png_ptr, png_state->info_ptr);

            if (png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_fcTL) != 0) {
                png_get_next_frame_fcTL(png_state->png_ptr, png_state->info_ptr,
                                        &png_state->next_frame_width, &png_state->next_frame_height,
                                        &png_state->next_frame_x_offset, &png_state->next_frame_y_offset,
                                        &png_state->next_frame_delay_num, &png_state->next_frame_delay_den,
                                        &png_state->next_frame_dispose_op, &png_state->next_frame_blend_op);
            } else {
                png_state->next_frame_width      = (*image)->width;
                png_state->next_frame_height     = (*image)->height;
                png_state->next_frame_x_offset   = 0;
                png_state->next_frame_y_offset   = 0;
                png_state->next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND;
                png_state->next_frame_blend_op   = PNG_BLEND_OP_SOURCE;
            }

            if (png_state->next_frame_width + png_state->next_frame_x_offset > (*image)->width ||
                    png_state->next_frame_height + png_state->next_frame_y_offset > (*image)->height) {
                return SAIL_INCORRECT_IMAGE_DIMENSIONS;
            }

            if (!png_state->next_frame_delay_den) {
                png_state->next_frame_delay_den = 100;
            }

            (*image)->delay = (int)(((double)png_state->next_frame_delay_num / png_state->next_frame_delay_den) * 1000);
        }
    }
#endif

    png_state->current_frame++;

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_frame_v3(void *state, struct sail_io *io, const struct sail_image *image, void *bits) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_BITS_PTR(bits);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng) {
        for (unsigned row = 0; row < image->height; row++) {
            unsigned char *scanline = (unsigned char *)bits + row * image->bytes_per_line;

            memcpy(scanline, png_state->prev[row], png_state->first_image->width * png_state->bytes_per_pixel);

            if (row >= png_state->next_frame_y_offset && row < png_state->next_frame_y_offset + png_state->next_frame_height) {
                png_read_row(png_state->png_ptr, (png_bytep)png_state->temp_scanline, NULL);

                /* Copy all pixel values including alpha. */
                if (png_state->current_frame == 1 || png_state->next_frame_blend_op == PNG_BLEND_OP_SOURCE) {
                    SAIL_TRY(blend_source(png_state->bytes_per_pixel,
                                            scanline,
                                            png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                                            png_state->temp_scanline,
                                            png_state->next_frame_width * png_state->bytes_per_pixel));
                } else { /* PNG_BLEND_OP_OVER */
                    SAIL_TRY(blend_over(png_state->bytes_per_pixel,
                                        png_state->next_frame_width,
                                        png_state->temp_scanline,
                                        scanline,
                                        png_state->next_frame_x_offset * png_state->bytes_per_pixel));
                }

                if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_BACKGROUND) {
                    memset(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                            0,
                            png_state->next_frame_width * png_state->bytes_per_pixel);
                } else if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_NONE) {
                    memcpy(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                            scanline,
                            png_state->next_frame_width * png_state->bytes_per_pixel);
                } else { /* PNG_DISPOSE_OP_PREVIOUS */
                }
            }
        }
    } else {
        for (unsigned row = 0; row < image->height; row++) {
            png_read_row(png_state->png_ptr, (unsigned char *)bits + row * image->bytes_per_line, NULL);
        }
    }
#else
    for (unsigned row = 0; row < image->height; row++) {
        png_read_row(png_state->png_ptr, (unsigned char *)bits + row * image->bytes_per_line, NULL);
    }
#endif

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

#ifdef PNG_APNG_SUPPORTED
    free(png_state->temp_scanline);
    free(png_state->scanline_for_skipping);
    destroy_rows(&png_state->prev, png_state->first_image->height);
#endif

    sail_destroy_image(png_state->first_image);
    sail_destroy_iccp(png_state->iccp);
    sail_destroy_read_options(png_state->read_options);

    if (png_state->png_ptr != NULL) {
        if (setjmp(png_jmpbuf(png_state->png_ptr))) {
            free(png_state);
            return SAIL_UNDERLYING_CODEC_ERROR;
        }
    }

    if (png_state->png_ptr != NULL) {
        png_destroy_read_struct(&png_state->png_ptr, &png_state->info_ptr, NULL);
    }

    free(png_state);

    return 0;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_error_t sail_plugin_write_init_v3(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy write options. */
    SAIL_TRY(sail_copy_write_options(write_options, &png_state->write_options));

    /* Sanity check. */
    SAIL_TRY(supported_write_output_pixel_format(png_state->write_options->output_pixel_format));

    if (png_state->write_options->compression_type != 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    /* Initialize PNG. */
    if ((png_state->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, my_error_fn, my_warning_fn)) == NULL) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    png_set_write_fn(png_state->png_ptr, io, my_write_fn, my_flush_fn);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->frame_written) {
        return SAIL_NO_MORE_FRAMES;
    }

    png_state->frame_written = true;

    /* Sanity check. */
    SAIL_TRY(supported_write_input_pixel_format(image->pixel_format));

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    int color_type;
    int bit_depth;
    SAIL_TRY(pixel_format_to_png_color_type(image->pixel_format, &color_type, &bit_depth));

    /* Write meta info. */
    if (png_state->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {
        SAIL_LOG_DEBUG("PNG: Writing meta info");
        SAIL_TRY(write_png_text(png_state->png_ptr, png_state->info_ptr, image->meta_entry_node));
    }

    png_set_IHDR(png_state->png_ptr,
                 png_state->info_ptr,
                 image->width,
                 image->height,
                 bit_depth,
                 color_type,
                 (png_state->write_options->io_options & SAIL_IO_OPTION_INTERLACED) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    /* Write ICC profile. */
    if (png_state->write_options->io_options & SAIL_IO_OPTION_ICCP && image->iccp != NULL) {
        png_set_iCCP(png_state->png_ptr,
                        png_state->info_ptr,
                        "ICC profile",
                        PNG_COMPRESSION_TYPE_BASE,
                        (const png_bytep)image->iccp->data,
                        image->iccp->data_length);

        SAIL_LOG_DEBUG("PNG: ICC profile has been set");
    }

    /* Write palette. */
    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED) {
        if (image->palette == NULL) {
            SAIL_LOG_ERROR("The indexed image has no palette");
            return SAIL_MISSING_PALETTE;
        }

        if (image->palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB) {
            SAIL_LOG_ERROR("Palettes not in BPP24-RGB format are not supported");
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        /* Palette is deep copied. */
        png_set_PLTE(png_state->png_ptr, png_state->info_ptr, image->palette->data, image->palette->color_count);
    }

    const int compression = (png_state->write_options->compression < COMPRESSION_MIN ||
                                png_state->write_options->compression > COMPRESSION_MAX)
                            ? COMPRESSION_DEFAULT
                            : png_state->write_options->compression;

    png_set_compression_level(png_state->png_ptr, compression);

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

    if (png_state->write_options->io_options & SAIL_IO_OPTION_INTERLACED) {
        png_set_interlace_handling(png_state->png_ptr);
    }

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(png_state->write_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Output pixel format is %s", pixel_format_str);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_frame_v3(void *state, struct sail_io *io, const struct sail_image *image, const void *bits) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_BITS_PTR(bits);

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    for (unsigned row = 0; row < image->height; row++) {
        png_write_row(png_state->png_ptr, (const unsigned char *)bits + row * image->bytes_per_line);
    }

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    sail_destroy_write_options(png_state->write_options);

    /* Error handling setup. */
    if (png_state->png_ptr != NULL) {
        if (setjmp(png_jmpbuf(png_state->png_ptr))) {
            free(png_state);
            return SAIL_UNDERLYING_CODEC_ERROR;
        }
    }

    if (png_state->png_ptr != NULL && !png_state->libpng_error) {
        png_write_end(png_state->png_ptr, png_state->info_ptr);
    }

    if (png_state->png_ptr != NULL) {
        png_destroy_write_struct(&png_state->png_ptr, &png_state->info_ptr);
    }

    free(png_state);

    return 0;
}
