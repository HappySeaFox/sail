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

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_frame_v2(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)state;

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

    (*image)->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);

    /* Apply requested transformations. */
    png_read_update_info(png_state->png_ptr, png_state->info_ptr);

    (*image)->source_pixel_format = png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);

    if ((*image)->interlaced_passes > 1) {
        (*image)->source_properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
    }

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line),
                        /* cleanup */ sail_destroy_image(*image));

    /* Read meta info. */
    if (png_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        SAIL_TRY_OR_CLEANUP(read_png_text(png_state->png_ptr, png_state->info_ptr, &(*image)->meta_entry_node),
                            /* cleanup */ sail_destroy_image(*image));
    }

    /* Read ICC profile. */
    if (png_state->read_options->io_options & SAIL_IO_OPTION_ICC) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_icc(&(*image)->icc),
                            /* cleanup */ sail_destroy_image(*image));

        bool ok = png_get_iCCP(png_state->png_ptr,
                                png_state->info_ptr,
                                &(*image)->icc->name,
                                /* compression - not needed */ NULL,
                                (png_bytepp)&(*image)->icc->data,
                                &(*image)->icc->data_length) == PNG_INFO_iCCP;

        if (ok) {
            SAIL_LOG_DEBUG("PNG: ICC profile %u bytes length is found", (*image)->icc->data_length);
        } else {
            SAIL_LOG_DEBUG("PNG: ICC profile is not found");
        }
    }

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string((*image)->source_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(png_state->read_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Output pixel format is %s", pixel_format_str);

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

SAIL_EXPORT sail_error_t sail_plugin_read_finish_v2(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct png_state *png_state = (struct png_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

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

SAIL_EXPORT sail_error_t sail_plugin_write_init_v2(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

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

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_frame_v2(void *state, struct sail_io *io, const struct sail_image *image) {

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
    if (png_state->write_options->io_options & SAIL_IO_OPTION_ICC && image->icc != NULL) {
        png_set_iCCP(png_state->png_ptr,
                        png_state->info_ptr,
                        image->icc->name,
                        PNG_COMPRESSION_TYPE_BASE,
                        (const png_bytep)image->icc->data,
                        image->icc->data_length);

        SAIL_LOG_DEBUG("PNG: ICC profile has been set");
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

    struct png_state *png_state = (struct png_state *)state;

    if (png_state->libpng_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr))) {
        png_state->libpng_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    png_bytep row_pointer = (png_bytep)scanline;

    png_write_rows(png_state->png_ptr, &row_pointer, 1);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_finish_v2(void **state, struct sail_io *io) {

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
