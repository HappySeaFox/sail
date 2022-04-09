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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

#include "sail-common.h"

#include "helpers.h"
#include "io_dest.h"
#include "io_src.h"

/*
 * Codec-specific data types.
 */

static const double COMPRESSION_MIN     = 0;
static const double COMPRESSION_MAX     = 100;
static const double COMPRESSION_DEFAULT = 15;

/*
 * Codec-specific state.
 */

struct jpeg_state {
    struct jpeg_decompress_struct *decompress_context;
    struct jpeg_compress_struct *compress_context;
    struct jpeg_private_my_error_context error_context;
    bool libjpeg_error;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;
    bool frame_loaded;
    bool frame_saved;
    bool started_compress;
};

static sail_status_t alloc_jpeg_state(struct jpeg_state **jpeg_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg_state), &ptr));
    *jpeg_state = ptr;

    (*jpeg_state)->decompress_context = NULL;
    (*jpeg_state)->compress_context   = NULL;
    (*jpeg_state)->libjpeg_error      = false;
    (*jpeg_state)->load_options       = NULL;
    (*jpeg_state)->save_options       = NULL;
    (*jpeg_state)->frame_loaded       = false;
    (*jpeg_state)->frame_saved        = false;
    (*jpeg_state)->started_compress   = false;

    return SAIL_OK;
}

static void destroy_jpeg_state(struct jpeg_state *jpeg_state) {

    if (jpeg_state == NULL) {
        return;
    }

    sail_free(jpeg_state->decompress_context);
    sail_free(jpeg_state->compress_context);

    sail_destroy_load_options(jpeg_state->load_options);
    sail_destroy_save_options(jpeg_state->save_options);

    sail_free(jpeg_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_jpeg(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct jpeg_state *jpeg_state;
    SAIL_TRY(alloc_jpeg_state(&jpeg_state));

    *state = jpeg_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &jpeg_state->load_options));

    /* Create decompress context. */
    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg_decompress_struct), &ptr));
    jpeg_state->decompress_context = ptr;

    /* Error handling setup. */
    jpeg_state->decompress_context->err = jpeg_std_error(&jpeg_state->error_context.jpeg_error_mgr);
    jpeg_state->error_context.jpeg_error_mgr.error_exit = jpeg_private_my_error_exit;
    jpeg_state->error_context.jpeg_error_mgr.output_message = jpeg_private_my_output_message;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* JPEG setup. */
    jpeg_create_decompress(jpeg_state->decompress_context);
    jpeg_private_sail_io_src(jpeg_state->decompress_context, io);

    if (jpeg_state->load_options->options & SAIL_OPTION_META_DATA) {
        jpeg_save_markers(jpeg_state->decompress_context, JPEG_COM, 0xffff);
    }
    if (jpeg_state->load_options->options & SAIL_OPTION_ICCP) {
        jpeg_save_markers(jpeg_state->decompress_context, JPEG_APP0 + 2, 0xFFFF);
    }

    jpeg_read_header(jpeg_state->decompress_context, true);

    /* Handle the requested color space. */
    if (jpeg_state->decompress_context->jpeg_color_space == JCS_YCbCr) {
        jpeg_state->decompress_context->out_color_space = JCS_RGB;
    } else {
        jpeg_state->decompress_context->out_color_space = jpeg_state->decompress_context->jpeg_color_space;
    }

    /* We don't want colormapped output. */
    jpeg_state->decompress_context->quantize_colors = false;

    /* Launch decompression! */
    jpeg_start_decompress(jpeg_state->decompress_context);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_jpeg(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jpeg_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Image properties. */
    image_local->width                      = jpeg_state->decompress_context->output_width;
    image_local->height                     = jpeg_state->decompress_context->output_height;
    image_local->pixel_format               = jpeg_private_color_space_to_pixel_format(jpeg_state->decompress_context->out_color_space);
    image_local->source_image->pixel_format = jpeg_private_color_space_to_pixel_format(jpeg_state->decompress_context->jpeg_color_space);
    image_local->source_image->compression  = SAIL_COMPRESSION_JPEG;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Read meta data. */
    if (jpeg_state->load_options->options & SAIL_OPTION_META_DATA) {
        SAIL_TRY_OR_CLEANUP(jpeg_private_fetch_meta_data(jpeg_state->decompress_context, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch resolution. */
    SAIL_TRY_OR_CLEANUP(jpeg_private_fetch_resolution(jpeg_state->decompress_context, &image_local->resolution),
                            /* cleanup */ sail_destroy_image(image_local));

    /* Fetch ICC profile. */
#ifdef SAIL_HAVE_JPEG_ICCP
    if (jpeg_state->load_options->options & SAIL_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(jpeg_private_fetch_iccp(jpeg_state->decompress_context, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }
#endif

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_jpeg(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->libjpeg_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *scanline = (unsigned char *)image->pixels + row * image->bytes_per_line;

        JSAMPROW samprow = (JSAMPROW)scanline;
        (void)jpeg_read_scanlines(jpeg_state->decompress_context, &samprow, 1);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_jpeg(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct jpeg_state *jpeg_state = (struct jpeg_state *)(*state);

    *state = NULL;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        destroy_jpeg_state(jpeg_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (jpeg_state->decompress_context != NULL) {
        jpeg_abort_decompress(jpeg_state->decompress_context);
        jpeg_destroy_decompress(jpeg_state->decompress_context);
    }

    destroy_jpeg_state(jpeg_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_jpeg(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    struct jpeg_state *jpeg_state;
    SAIL_TRY(alloc_jpeg_state(&jpeg_state));

    *state = jpeg_state;

    /* Deep copy save options. */
    SAIL_TRY(sail_copy_save_options(save_options, &jpeg_state->save_options));

    /* Create compress context. */
    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg_compress_struct), &ptr));
    jpeg_state->compress_context = ptr;

    /* Sanity check. */
    if (jpeg_state->save_options->compression != SAIL_COMPRESSION_JPEG) {
        SAIL_LOG_ERROR("JPEG: Only JPEG compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Error handling setup. */
    jpeg_state->compress_context->err = jpeg_std_error(&jpeg_state->error_context.jpeg_error_mgr);
    jpeg_state->error_context.jpeg_error_mgr.error_exit = jpeg_private_my_error_exit;
    jpeg_state->error_context.jpeg_error_mgr.output_message = jpeg_private_my_output_message;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* JPEG setup. */
    jpeg_create_compress(jpeg_state->compress_context);
    jpeg_private_sail_io_dest(jpeg_state->compress_context, io);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_jpeg(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->frame_saved) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jpeg_state->frame_saved = true;

    /* Error handling setup. */
    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    /* Compute output pixel format. */
    const J_COLOR_SPACE color_space = jpeg_private_pixel_format_to_color_space(image->pixel_format);

    if (color_space == JCS_UNKNOWN) {
        SAIL_LOG_ERROR("JPEG: %s pixel format is not currently supported for saving", sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Initialize compression. */
    jpeg_state->compress_context->image_width      = image->width;
    jpeg_state->compress_context->image_height     = image->height;
    jpeg_state->compress_context->input_components = bits_per_pixel / 8;
    jpeg_state->compress_context->in_color_space   = color_space;
    jpeg_state->compress_context->input_gamma      = image->gamma;

    jpeg_set_defaults(jpeg_state->compress_context);
    jpeg_set_colorspace(jpeg_state->compress_context, color_space);

    /* Save resolution. */
    SAIL_TRY(jpeg_private_write_resolution(jpeg_state->compress_context, image->resolution));

    /* Compute image quality. */
    const double compression = (jpeg_state->save_options->compression_level < COMPRESSION_MIN ||
                                jpeg_state->save_options->compression_level > COMPRESSION_MAX)
                                ? COMPRESSION_DEFAULT
                                : jpeg_state->save_options->compression_level;
    jpeg_set_quality(jpeg_state->compress_context, /* to quality */ (int)(COMPRESSION_MAX-compression), true);

    /* Handle tuning. */
    if (jpeg_state->save_options->tuning != NULL) {
        sail_traverse_hash_map_with_user_data(jpeg_state->save_options->tuning, jpeg_private_tuning_key_value_callback, jpeg_state->compress_context);
    }

    /* Start compression. */
    jpeg_start_compress(jpeg_state->compress_context, true);
    jpeg_state->started_compress = true;

    /* Save meta data. */
    if (jpeg_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL) {
        SAIL_TRY(jpeg_private_write_meta_data(jpeg_state->compress_context, image->meta_data_node));
        SAIL_LOG_DEBUG("JPEG: Meta data has been written");
    }

    /* Save ICC profile. */
#ifdef SAIL_HAVE_JPEG_ICCP
    if (jpeg_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL) {
        jpeg_write_icc_profile(jpeg_state->compress_context, image->iccp->data, image->iccp->data_length);
        SAIL_LOG_DEBUG("JPEG: ICC profile has been written");
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_jpeg(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->libjpeg_error) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (unsigned row = 0; row < image->height; row++) {
        JSAMPROW samprow = (JSAMPROW)((const unsigned char *)image->pixels + row * image->bytes_per_line);
        jpeg_write_scanlines(jpeg_state->compress_context, &samprow, 1);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_jpeg(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct jpeg_state *jpeg_state = (struct jpeg_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        destroy_jpeg_state(jpeg_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (jpeg_state->compress_context != NULL) {
        if (jpeg_state->started_compress) {
            jpeg_finish_compress(jpeg_state->compress_context);
        }

        jpeg_destroy_compress(jpeg_state->compress_context);
    }

    destroy_jpeg_state(jpeg_state);

    return SAIL_OK;
}
