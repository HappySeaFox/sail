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
 * Codec-specific state.
 */
struct tiff_state {
    TIFF *tiff;
    uint16_t current_frame;
    bool libtiff_error;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;
    int save_compression;
    TIFFRGBAImage image;
    int line;
};

static sail_status_t alloc_tiff_state(struct tiff_state **tiff_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tiff_state), &ptr));
    *tiff_state = ptr;

    (*tiff_state)->tiff             = NULL;
    (*tiff_state)->current_frame    = 0;
    (*tiff_state)->libtiff_error    = false;
    (*tiff_state)->load_options     = NULL;
    (*tiff_state)->save_options     = NULL;
    (*tiff_state)->save_compression = COMPRESSION_NONE;
    (*tiff_state)->line             = 0;

    tiff_private_zero_tiff_image(&(*tiff_state)->image);

    return SAIL_OK;
}

static void destroy_tiff_state(struct tiff_state *tiff_state) {

    if (tiff_state == NULL) {
        return;
    }

    sail_destroy_load_options(tiff_state->load_options);
    sail_destroy_save_options(tiff_state->save_options);

    TIFFRGBAImageEnd(&tiff_state->image);

    sail_free(tiff_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_tiff(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    TIFFSetWarningHandler(tiff_private_my_warning_fn);
    TIFFSetErrorHandler(tiff_private_my_error_fn);

    /* Allocate a new state. */
    struct tiff_state *tiff_state;
    SAIL_TRY(alloc_tiff_state(&tiff_state));

    *state = tiff_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &tiff_state->load_options));

    /* Initialize TIFF.
     *
     * 'r': reading operation
     * 'h': read TIFF header only
     * 'm': disable use of memory-mapped files
     */
    tiff_state->tiff = TIFFClientOpen("sail-codec-tiff",
                                      "rhm",
                                      io,
                                      tiff_private_my_read_proc,
                                      tiff_private_my_write_proc,
                                      tiff_private_my_seek_proc,
                                      tiff_private_my_dummy_close_proc,
                                      tiff_private_my_dummy_size_proc,
                                      /* map */ NULL,
                                      /* unmap */ NULL);

    if (tiff_state->tiff == NULL) {
        tiff_state->libtiff_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_tiff(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Start reading the next directory. */
    if (!TIFFSetDirectory(tiff_state->tiff, tiff_state->current_frame++)) {
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Start reading the next image. */
    char emsg[1024];
    if (!TIFFRGBAImageBegin(&tiff_state->image, tiff_state->tiff, /* stop */ 1, emsg)) {
        SAIL_LOG_ERROR("TIFF: %s", emsg);
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    tiff_state->image.req_orientation = ORIENTATION_TOPLEFT;

    /* Fill the image properties. */
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH,  &image_local->width) || !TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, &image_local->height)) {
        SAIL_LOG_ERROR("TIFF: Failed to get the image dimensions");
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Fetch meta data. */
    if (tiff_state->load_options->options & SAIL_OPTION_META_DATA) {
        struct sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

        SAIL_TRY_OR_CLEANUP(tiff_private_fetch_meta_data(tiff_state->tiff, &last_meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch ICC profile. */
    if (tiff_state->load_options->options & SAIL_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(tiff_private_fetch_iccp(tiff_state->tiff, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch resolution. */
    SAIL_TRY_OR_CLEANUP(tiff_private_fetch_resolution(tiff_state->tiff, &image_local->resolution),
                            /* cleanup */ sail_destroy_image(image_local));

    image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Fill the source image properties. */
    int compression = COMPRESSION_NONE;
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_COMPRESSION, &compression)) {
        SAIL_LOG_ERROR("TIFF: Failed to get the image compression type");
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    image_local->source_image->pixel_format = tiff_private_bpp_to_pixel_format(tiff_state->image.bitspersample * tiff_state->image.samplesperpixel);
    image_local->source_image->compression = tiff_private_compression_to_sail_compression(compression);

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_tiff(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (!TIFFRGBAImageGet(&tiff_state->image, image->pixels, image->width, image->height)) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    TIFFRGBAImageEnd(&tiff_state->image);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_tiff(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct tiff_state *tiff_state = (struct tiff_state *)(*state);

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

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_tiff(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    struct tiff_state *tiff_state;
    SAIL_TRY(alloc_tiff_state(&tiff_state));

    *state = tiff_state;

    /* Deep copy save options. */
    SAIL_TRY(sail_copy_save_options(save_options, &tiff_state->save_options));

    /* Sanity check. */
    SAIL_TRY_OR_EXECUTE(tiff_private_sail_compression_to_compression(tiff_state->save_options->compression, &tiff_state->save_compression),
                        /* cleanup */ SAIL_LOG_ERROR("TIFF: %s compression is not supported for saving", sail_compression_to_string(tiff_state->save_options->compression));
                                      return __sail_error_result);

    TIFFSetWarningHandler(tiff_private_my_warning_fn);
    TIFFSetErrorHandler(tiff_private_my_error_fn);

    /* Initialize TIFF.
     *
     * 'w': writing operation
     * 'm': disable use of memory-mapped files
     */
    tiff_state->tiff = TIFFClientOpen("tiff-sail-codec",
                                      "wm",
                                      io,
                                      tiff_private_my_read_proc,
                                      tiff_private_my_write_proc,
                                      tiff_private_my_seek_proc,
                                      /* libsail will close for us. */ tiff_private_my_dummy_close_proc,
                                      tiff_private_my_dummy_size_proc,
                                      /* map */ NULL,
                                      /* unmap */ NULL);

    if (tiff_state->tiff == NULL) {
        tiff_state->libtiff_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_tiff(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    tiff_state->line = 0;

    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH,  image->width);
    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, image->height);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tiff_state->tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
    TIFFSetField(tiff_state->tiff, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tiff_state->tiff, TIFFTAG_COMPRESSION, tiff_state->save_compression);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff_state->tiff, (uint32_t)-1));

    /* Save ICC profile. */
    if (tiff_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL) {
        TIFFSetField(tiff_state->tiff, TIFFTAG_ICCPROFILE, image->iccp->data_length, image->iccp->data);
        SAIL_LOG_DEBUG("TIFF: ICC profile has been saved");
    }

    /* Save meta data. */
    if (tiff_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL) {
        SAIL_LOG_DEBUG("TIFF: Saving meta data");
        SAIL_TRY(tiff_private_write_meta_data(tiff_state->tiff, image->meta_data_node));
    }

    /* Save resolution. */
    SAIL_TRY(tiff_private_write_resolution(tiff_state->tiff, image->resolution));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_tiff(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct tiff_state *tiff_state = (struct tiff_state *)state;

    if (tiff_state->libtiff_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (unsigned row = 0; row < image->height; row++) {
        if (TIFFWriteScanline(tiff_state->tiff, (unsigned char *)image->pixels + row * image->bytes_per_line, tiff_state->line++, 0) < 0) {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    if (!TIFFWriteDirectory(tiff_state->tiff)) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_tiff(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct tiff_state *tiff_state = (struct tiff_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Destroy internal TIFF objects. */
    if (tiff_state->tiff != NULL) {
        TIFFCleanup(tiff_state->tiff);
    }

    destroy_tiff_state(tiff_state);

    return SAIL_OK;
}
