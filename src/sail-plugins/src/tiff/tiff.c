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

#include <tiffio.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Plugin-specific state.
 */
struct tiff_state {
    TIFF *tiff;
    uint16_t current_frame;
    bool libtiff_error;
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;
    TIFFRGBAImage image;
};

static sail_status_t alloc_tiff_state(struct tiff_state **tiff_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct tiff_state)));
    *tiff_state = ptr;

    if (*tiff_state == NULL) {
        return SAIL_ERROR_MEMORY_ALLOCATION;
    }

    (*tiff_state)->tiff          = NULL;
    (*tiff_state)->current_frame = 0;
    (*tiff_state)->libtiff_error = false;
    (*tiff_state)->read_options  = NULL;
    (*tiff_state)->write_options = NULL;

    zero_tiff_image(&(*tiff_state)->image);

    return SAIL_OK;
}

static void destroy_tiff_state(struct tiff_state *tiff_state) {

    if (tiff_state == NULL) {
        return;
    }

    sail_destroy_read_options(tiff_state->read_options);
    sail_destroy_write_options(tiff_state->write_options);

    TIFFRGBAImageEnd(&tiff_state->image);

    sail_free(tiff_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_plugin_read_init_v3(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    TIFFSetWarningHandler(my_warning_fn);
    TIFFSetErrorHandler(my_error_fn);

    SAIL_TRY(supported_read_output_pixel_format(read_options->output_pixel_format));

    /* Allocate a new state. */
    struct tiff_state *tiff_state;
    SAIL_TRY(alloc_tiff_state(&tiff_state));

    *state = tiff_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &tiff_state->read_options));

    /* Initialize TIFF.
     *
     * 'r': reading operation
     * 'm': disable use of memory-mapped files
     * 'h': read TIFF header only
     */
    tiff_state->tiff = TIFFClientOpen("tiff-sail-plugin",
                                        "rhm",
	                                    io,
	                                    my_read_proc,
                                        my_write_proc,
                                	    my_seek_proc,
                                        my_dummy_close_proc,
                                	    my_dummy_size_proc,
                                	    /* map */ NULL,
                                        /* unmap */ NULL);

    if (tiff_state->tiff == NULL) {
        tiff_state->libtiff_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_seek_next_frame_v3(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE_PTR(image);

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    SAIL_TRY(sail_alloc_image(image));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&(*image)->source_image),
                        /* cleanup */ sail_destroy_image(*image));

    /* Start reading the next directory. */
    if (!TIFFSetDirectory(tiff_state->tiff, tiff_state->current_frame++)) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Start reading the next image. */
    char emsg[1024];
    if (!TIFFRGBAImageBegin(&tiff_state->image, tiff_state->tiff, /* stop */ 1, emsg)) {
        SAIL_LOG_ERROR("TIFF: %s", emsg);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    tiff_state->image.req_orientation = ORIENTATION_TOPLEFT;

    /* Fill the image properties. */
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH,  &(*image)->width) || !TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, &(*image)->height)) {
        SAIL_LOG_ERROR("Failed to get the image dimensions");
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    SAIL_TRY(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line));

    /* Fill the source image properties. */
    int compression = COMPRESSION_NONE;
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_COMPRESSION, &compression)) {
        SAIL_LOG_ERROR("Failed to get the image compression type");
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    SAIL_TRY(tiff_compression_to_sail_compression_type(compression, &(*image)->source_image->compression_type));
    SAIL_TRY(bpp_to_pixel_format(tiff_state->image.bitspersample * tiff_state->image.samplesperpixel, &(*image)->source_image->pixel_format));

    const char *pixel_format_str;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string((*image)->source_image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("TIFF: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(tiff_state->read_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("TIFF: Output pixel format is %s", pixel_format_str);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_frame_v3(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    if (!TIFFRGBAImageGet(&tiff_state->image, image->pixels, image->width, image->height)) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    TIFFRGBAImageEnd(&tiff_state->image);

    /* Swap colors. */
    if (tiff_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
        unsigned char *pixels = image->pixels;
        const unsigned pixels_count = image->width * image->height;
        unsigned char tmp;

        for (unsigned i = 0; i < pixels_count; i++, pixels += 4) {
            tmp = *pixels;
            *pixels = *(pixels+2);
            *(pixels+2) = tmp;
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct tiff_state *tiff_state = (struct tiff_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (tiff_state->tiff != NULL) {
        TIFFCleanup(tiff_state->tiff);
    }

    destroy_tiff_state(tiff_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_plugin_write_init_v3(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    struct tiff_state *tiff_state;
    SAIL_TRY(alloc_tiff_state(&tiff_state));

    *state = tiff_state;

    /* Deep copy write options. */
    SAIL_TRY(sail_copy_write_options(write_options, &tiff_state->write_options));

#if 0
    /* Sanity check. */
    SAIL_TRY(supported_write_output_pixel_format(tiff_state->write_options->output_pixel_format));

    if (tiff_state->write_options->compression_type != 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    /* Initialize PNG. */
    if ((tiff_state->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, my_error_fn, my_warning_fn)) == NULL) {
        tiff_state->libpng_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    if ((tiff_state->info_ptr = png_create_info_struct(tiff_state->png_ptr)) == NULL) {
        tiff_state->libpng_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(tiff_state->png_ptr))) {
        tiff_state->libpng_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    png_set_write_fn(tiff_state->png_ptr, io, my_write_fn, my_flush_fn);
#endif
    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_seek_next_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

#if 0
    /* Sanity check. */
    SAIL_TRY(supported_write_input_pixel_format(image->pixel_format));

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(tiff_state->png_ptr))) {
        tiff_state->libpng_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    int color_type;
    int bit_depth;
    SAIL_TRY(pixel_format_to_png_color_type(image->pixel_format, &color_type, &bit_depth));

    /* Write meta info. */
    if (tiff_state->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {
        SAIL_LOG_DEBUG("PNG: Writing meta info");
        SAIL_TRY(write_png_text(tiff_state->png_ptr, tiff_state->info_ptr, image->meta_entry_node));
    }

    png_set_IHDR(tiff_state->png_ptr,
                 tiff_state->info_ptr,
                 image->width,
                 image->height,
                 bit_depth,
                 color_type,
                 (tiff_state->write_options->io_options & SAIL_IO_OPTION_INTERLACED) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    /* Write ICC profile. */
    if (tiff_state->write_options->io_options & SAIL_IO_OPTION_ICCP && image->iccp != NULL) {
        png_set_iCCP(tiff_state->png_ptr,
                        tiff_state->info_ptr,
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
        png_set_PLTE(tiff_state->png_ptr, tiff_state->info_ptr, image->palette->data, image->palette->color_count);
    }

    const int compression = (tiff_state->write_options->compression < COMPRESSION_MIN ||
                                tiff_state->write_options->compression > COMPRESSION_MAX)
                            ? COMPRESSION_DEFAULT
                            : tiff_state->write_options->compression;

    png_set_compression_level(tiff_state->png_ptr, compression);

    png_write_info(tiff_state->png_ptr, tiff_state->info_ptr);

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR      ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR  ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR) {
        png_set_bgr(tiff_state->png_ptr);
    }

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB     ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ARGB ||
            image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR) {
        png_set_swap_alpha(tiff_state->png_ptr);
    }

    if (tiff_state->write_options->io_options & SAIL_IO_OPTION_INTERLACED) {
        png_set_interlace_handling(tiff_state->png_ptr);
    }

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(tiff_state->write_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("PNG: Output pixel format is %s", pixel_format_str);
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

#if 0
    for (unsigned row = 0; row < image->height; row++) {
        png_write_row(tiff_state->png_ptr, (const unsigned char *)image->pixels + row * image->bytes_per_line);
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct tiff_state *tiff_state = (struct tiff_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Error handling setup. */
    if (tiff_state->tiff != NULL) {
        TIFFCleanup(tiff_state->tiff);
    }

    destroy_tiff_state(tiff_state);

    return SAIL_OK;
}
