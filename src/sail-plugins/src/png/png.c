/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#include <setjmp.h>
#include <stdbool.h>
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

    bool libpng_error;
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;
    bool frame_read;
    bool frame_written;
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
    (*png_state)->libpng_error   = false;
    (*png_state)->read_options   = NULL;
    (*png_state)->write_options  = NULL;
    (*png_state)->frame_read     = false;
    (*png_state)->frame_written  = false;

    return 0;
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_error_t sail_plugin_read_init_v2(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    SAIL_TRY(supported_pixel_format(read_options->output_pixel_format));

    /* Allocate a new state. */
    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_deep_copy_read_options(read_options, &png_state->read_options));

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

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_frame_v2(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)state;
    SAIL_CHECK_STATE_PTR(png_state);

    if (png_state->frame_read) {
        return SAIL_NO_MORE_FRAMES;
    }

    png_state->frame_read = true;
    SAIL_TRY(sail_alloc_image(image));

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        sail_destroy_image(*image);
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    png_get_IHDR(png_state->png_ptr,
                    png_state->info_ptr,
                    &(*image)->width,
                    &(*image)->height,
                    &png_state->bit_depth,
                    &png_state->color_type,
                    &png_state->interlace_type,
                    /* compression type */ NULL,
                    /* filter method */ NULL);

    /* Transform the PNG stream. */
    if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        (*image)->pixel_format = png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);

        /* Save palette. */
        if (png_state->color_type == PNG_COLOR_TYPE_PALETTE) {
            int palette_color_count;
            png_colorp palette;

            if (png_get_PLTE(png_state->png_ptr, png_state->info_ptr, &palette, &palette_color_count) != PNG_INFO_PLTE) {
                png_error(png_state->png_ptr, "The indexed image has no palette");
            }

            /* Always use RGB for palette. */
            (*image)->palette_pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
            (*image)->palette_color_count = palette_color_count;
            (*image)->palette = malloc(palette_color_count * 3);

            if ((*image)->palette == NULL) {
                sail_destroy_image(*image);
                return SAIL_MEMORY_ALLOCATION_FAILED;
            }

            unsigned char *palette_ptr = (*image)->palette;

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

        if (png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_state->png_ptr);
        }

        if (png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB ||
                png_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) {
            png_set_strip_alpha(png_state->png_ptr);
        }

        (*image)->pixel_format = png_state->read_options->output_pixel_format;
    }

    (*image)->passes = png_set_interlace_handling(png_state->png_ptr);

    /* Apply requested transformations. */
    png_read_update_info(png_state->png_ptr, png_state->info_ptr);

    (*image)->source_pixel_format = png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);

    if ((*image)->passes > 1) {
        (*image)->source_properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
    }

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line),
                        /* cleanup */ sail_destroy_image(*image));

    /* Read meta info. */
    if (png_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        SAIL_TRY_OR_CLEANUP(read_png_text(png_state->png_ptr, png_state->info_ptr, &(*image)->meta_entry_node),
                            /* cleanup */ sail_destroy_image(*image));
    }

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_pass_v2(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_scan_line_v2(void *state, struct sail_io *io, const struct sail_image *image, void *scanline) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_SCAN_LINE_PTR(scanline);

    struct png_state *png_state = (struct png_state *)state;
    SAIL_CHECK_STATE_PTR(png_state);

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    png_read_row(png_state->png_ptr, (png_bytep)scanline, NULL);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_alloc_scan_line_v2(void *state, struct sail_io *io, const struct sail_image *image, void **scanline) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct png_state *png_state = (struct png_state *)state;
    SAIL_CHECK_STATE_PTR(png_state);

    *scanline = malloc(image->bytes_per_line);

    if (*scanline == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    return sail_plugin_read_scan_line_v2(state, io, image, *scanline);
}

SAIL_EXPORT sail_error_t sail_plugin_read_finish_v2(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)(*state);
    SAIL_CHECK_STATE_PTR(png_state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    sail_destroy_read_options(png_state->read_options);

    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        free(png_state);
        return SAIL_UNDERLYING_CODEC_ERROR;
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

SAIL_EXPORT sail_error_t sail_plugin_write_init_v2(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);
#if 0

    struct png_state *png_state;
    SAIL_TRY(alloc_png_state(&png_state));

    *state = png_state;

    /* Deep copy write options. */
    SAIL_TRY(sail_deep_copy_write_options(write_options, &png_state->write_options));

    /* Sanity check. */
    if (pixel_format_to_color_space(png_state->write_options->output_pixel_format) == JCS_UNKNOWN) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (png_state->write_options->compression_type != 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    /* Error handling setup. */
    png_state->compress_context.err = jpeg_std_error(&png_state->error_context.jpeg_error_mgr);
    png_state->error_context.jpeg_error_mgr.error_exit = my_error_exit;
    png_state->error_context.jpeg_error_mgr.output_message = my_output_message;

    if (setjmp(png_state->error_context.setjmp_buffer) != 0) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* JPEG setup. */
    jpeg_create_compress(&png_state->compress_context);
    jpeg_sail_io_dest(&png_state->compress_context, io);
#endif

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_frame_v2(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

#if 0
    /* Sanity check. */
    if (pixel_format_to_color_space(image->pixel_format) == JCS_UNKNOWN) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    struct png_state *png_state = (struct png_state *)state;
    SAIL_CHECK_STATE_PTR(png_state);

    if (png_state->frame_written) {
        return SAIL_NO_MORE_FRAMES;
    }

    png_state->frame_written = true;

    /* Error handling setup. */
    if (setjmp(png_state->error_context.setjmp_buffer) != 0) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    int bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    png_state->compress_context.image_width = image->width;
    png_state->compress_context.image_height = image->height;
    png_state->compress_context.input_components = bits_per_pixel / 8;
    png_state->compress_context.in_color_space = pixel_format_to_color_space(image->pixel_format);

    jpeg_set_defaults(&png_state->compress_context);
    jpeg_set_colorspace(&png_state->compress_context, pixel_format_to_color_space(png_state->write_options->output_pixel_format));

    const int compression = (png_state->write_options->compression < COMPRESSION_MIN ||
                                png_state->write_options->compression > COMPRESSION_MAX)
                            ? COMPRESSION_DEFAULT
                            : png_state->write_options->compression;
    jpeg_set_quality(&png_state->compress_context, /* to quality */COMPRESSION_MAX-compression, true);

    jpeg_start_compress(&png_state->compress_context, true);

    /* Write meta info. */
    if (png_state->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {

        struct sail_meta_entry_node *meta_entry_node = image->meta_entry_node;

        while (meta_entry_node != NULL) {
            jpeg_write_marker(&png_state->compress_context,
                                JPEG_COM,
                                (JOCTET *)meta_entry_node->value,
                                (unsigned int)strlen(meta_entry_node->value));

            meta_entry_node = meta_entry_node->next;
        }
    }
#endif

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_pass_v2(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_scan_line_v2(void *state, struct sail_io *io, const struct sail_image *image, const void *scanline) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_SCAN_LINE_PTR(scanline);

#if 0
    struct png_state *png_state = (struct png_state *)state;
    SAIL_CHECK_STATE_PTR(png_state);

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (setjmp(png_state->error_context.setjmp_buffer) != 0) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    JSAMPROW row = (JSAMPROW)scanline;

    jpeg_write_scanlines(&png_state->compress_context, &row, 1);
#endif

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_finish_v2(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

#if 0
    struct png_state *png_state = (struct png_state *)(*state);
    SAIL_CHECK_STATE_PTR(png_state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    sail_destroy_write_options(png_state->write_options);

    if (setjmp(png_state->error_context.setjmp_buffer) != 0) {
        free(png_state);
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    jpeg_finish_compress(&png_state->compress_context);
    jpeg_destroy_compress(&png_state->compress_context);

    free(png_state);
#endif

    return 0;
}
