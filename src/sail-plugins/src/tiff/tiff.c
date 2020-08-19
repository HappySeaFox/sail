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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    int write_compression;
    TIFFRGBAImage image;
    int line;
};

static sail_status_t alloc_tiff_state(struct tiff_state **tiff_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct tiff_state)));
    *tiff_state = ptr;

    if (*tiff_state == NULL) {
        return SAIL_ERROR_MEMORY_ALLOCATION;
    }

    (*tiff_state)->tiff              = NULL;
    (*tiff_state)->current_frame     = 0;
    (*tiff_state)->libtiff_error     = false;
    (*tiff_state)->read_options      = NULL;
    (*tiff_state)->write_options     = NULL;
    (*tiff_state)->write_compression = COMPRESSION_NONE;
    (*tiff_state)->line              = 0;

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
     * 'h': read TIFF header only
     * 'm': disable use of memory-mapped files
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
        sail_destroy_image(*image);
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Start reading the next image. */
    char emsg[1024];
    if (!TIFFRGBAImageBegin(&tiff_state->image, tiff_state->tiff, /* stop */ 1, emsg)) {
        SAIL_LOG_ERROR("TIFF: %s", emsg);
        sail_destroy_image(*image);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    tiff_state->image.req_orientation = ORIENTATION_TOPLEFT;

    /* Fill the image properties. */
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH,  &(*image)->width) || !TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, &(*image)->height)) {
        SAIL_LOG_ERROR("Failed to get the image dimensions");
        sail_destroy_image(*image);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* Fetch meta info and ICCP. */
    if (tiff_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        struct sail_meta_entry_node **last_meta_entry_node = &(*image)->meta_entry_node;

        SAIL_TRY_OR_CLEANUP(fetch_meta_info(tiff_state->tiff, &last_meta_entry_node),
                            /* cleanup */ sail_destroy_image(*image));
    }
    if (tiff_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(fetch_iccp(tiff_state->tiff, &(*image)->iccp),
                            /* cleanup */ sail_destroy_image(*image));
    }

    (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line),
                        /* cleanup */ sail_destroy_image(*image));

    /* Fill the source image properties. */
    int compression = COMPRESSION_NONE;
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_COMPRESSION, &compression)) {
        SAIL_LOG_ERROR("Failed to get the image compression type");
        sail_destroy_image(*image);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    (*image)->source_image->compression = tiff_compression_to_sail_compression(compression);
    (*image)->source_image->pixel_format = bpp_to_pixel_format(tiff_state->image.bitspersample * tiff_state->image.samplesperpixel);

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

    /* Sanity check. */
    SAIL_TRY(supported_write_output_pixel_format(tiff_state->write_options->output_pixel_format));
    SAIL_TRY(sail_compression_to_tiff_compression(tiff_state->write_options->compression, &tiff_state->write_compression));

    TIFFSetWarningHandler(my_warning_fn);
    TIFFSetErrorHandler(my_error_fn);

    /* Initialize TIFF.
     *
     * 'w': writing operation
     * 'm': disable use of memory-mapped files
     */
    tiff_state->tiff = TIFFClientOpen("tiff-sail-plugin",
                                      "wm",
                                      io,
                                      my_read_proc,
                                      my_write_proc,
                                      my_seek_proc,
                                      /* libsail will close for us. */ my_dummy_close_proc,
                                      my_dummy_size_proc,
                                      /* map */ NULL,
                                      /* unmap */ NULL);

    if (tiff_state->tiff == NULL) {
        tiff_state->libtiff_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

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

    tiff_state->line = 0;

    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH,  image->width);
    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, image->height);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tiff_state->tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
    TIFFSetField(tiff_state->tiff, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tiff_state->tiff, TIFFTAG_COMPRESSION, tiff_state->write_compression);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff_state->tiff, (uint32)-1));

    /* Write ICC profile. */
    if (tiff_state->write_options->io_options & SAIL_IO_OPTION_ICCP && image->iccp != NULL) {
        TIFFSetField(tiff_state->tiff, TIFFTAG_ICCPROFILE, image->iccp->data_length, image->iccp->data);
        SAIL_LOG_DEBUG("TIFF: ICC profile has been set");
    }

    /* Write meta info. */
    if (tiff_state->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {
#if 0
        SAIL_LOG_DEBUG("TIFF: Writing meta info");
        SAIL_TRY(write_meta_info(tiff_state->tiff, image->meta_entry_node));
#endif
    }

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("TIFF: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(tiff_state->write_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("TIFF: Output pixel format is %s", pixel_format_str);

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

    for (unsigned row = 0; row < image->height; row++) {
        if (TIFFWriteScanline(tiff_state->tiff, (unsigned char *)image->pixels + row * image->bytes_per_line, tiff_state->line++, 0) < 0) {
            return SAIL_ERROR_UNDERLYING_CODEC;
        }
    }

    if (!TIFFWriteDirectory(tiff_state->tiff)) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

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
