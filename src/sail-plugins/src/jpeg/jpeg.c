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
 * Plugin-specific data types.
 */

static const double COMPRESSION_MIN     = 0;
static const double COMPRESSION_MAX     = 100;
static const double COMPRESSION_DEFAULT = 15;

/*
 * Plugin-specific state.
 */

struct jpeg_state {
    struct jpeg_decompress_struct *decompress_context;
    struct jpeg_compress_struct *compress_context;
    struct my_error_context error_context;
    bool libjpeg_error;
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;
    bool frame_read;
    bool frame_written;
    bool started_compress;

    /* Extra scan line used as a buffer when reading CMYK/YCCK images. */
    bool extra_scan_line_needed_for_cmyk;
    void *extra_scan_line;
};

static sail_status_t alloc_jpeg_state(struct jpeg_state **jpeg_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct jpeg_state)));
    *jpeg_state = ptr;

    (*jpeg_state)->decompress_context              = NULL;
    (*jpeg_state)->compress_context                = NULL;
    (*jpeg_state)->libjpeg_error                   = false;
    (*jpeg_state)->read_options                    = NULL;
    (*jpeg_state)->write_options                   = NULL;
    (*jpeg_state)->frame_read                      = false;
    (*jpeg_state)->frame_written                   = false;
    (*jpeg_state)->started_compress                = false;
    (*jpeg_state)->extra_scan_line_needed_for_cmyk = false;
    (*jpeg_state)->extra_scan_line                 = NULL;

    return SAIL_OK;
}

static void destroy_jpeg_state(struct jpeg_state *jpeg_state) {

    if (jpeg_state == NULL) {
        return;
    }

    sail_free(jpeg_state->decompress_context);
    sail_free(jpeg_state->compress_context);

    sail_destroy_read_options(jpeg_state->read_options);
    sail_destroy_write_options(jpeg_state->write_options);

    sail_free(jpeg_state->extra_scan_line);

    sail_free(jpeg_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_plugin_read_init_v3(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    /* Allocate a new state. */
    struct jpeg_state *jpeg_state;
    SAIL_TRY(alloc_jpeg_state(&jpeg_state));

    *state = jpeg_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &jpeg_state->read_options));

    /* Create decompress context. */
    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct jpeg_decompress_struct)));
    jpeg_state->decompress_context = ptr;

    /* Error handling setup. */
    jpeg_state->decompress_context->err = jpeg_std_error(&jpeg_state->error_context.jpeg_error_mgr);
    jpeg_state->error_context.jpeg_error_mgr.error_exit = my_error_exit;
    jpeg_state->error_context.jpeg_error_mgr.output_message = my_output_message;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* JPEG setup. */
    jpeg_create_decompress(jpeg_state->decompress_context);
    jpeg_sail_io_src(jpeg_state->decompress_context, io);

    if (jpeg_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        jpeg_save_markers(jpeg_state->decompress_context, JPEG_COM, 0xffff);
    }
    if (jpeg_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        jpeg_save_markers(jpeg_state->decompress_context, JPEG_APP0 + 2, 0xFFFF);
    }

    jpeg_read_header(jpeg_state->decompress_context, true);

    /* Handle the requested color space. */
    if (jpeg_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        jpeg_state->decompress_context->out_color_space = jpeg_state->decompress_context->jpeg_color_space;
    } else {
        J_COLOR_SPACE requested_color_space = pixel_format_to_color_space(jpeg_state->read_options->output_pixel_format);

        if (requested_color_space == JCS_UNKNOWN) {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }

        if (jpeg_state->decompress_context->jpeg_color_space == JCS_YCCK || jpeg_state->decompress_context->jpeg_color_space == JCS_CMYK) {
            SAIL_LOG_DEBUG("JPEG: Requesting to convert to CMYK and only then to RGB/RGBA");
            jpeg_state->extra_scan_line_needed_for_cmyk = true;
            jpeg_state->decompress_context->out_color_space = JCS_CMYK;
        } else {
            jpeg_state->decompress_context->out_color_space = requested_color_space;
        }
    }

    /* We don't want colormapped output. */
    jpeg_state->decompress_context->quantize_colors = false;

    /* Launch decompression! */
    jpeg_start_decompress(jpeg_state->decompress_context);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_seek_next_frame_v3(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE_PTR(image);

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->frame_read) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    jpeg_state->frame_read = true;
    SAIL_TRY(sail_alloc_image(image));

    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&(*image)->source_image),
                        /* cleanup */ sail_destroy_image(*image));

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        sail_destroy_image(*image);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* Image properties. */
    unsigned bytes_per_line;
    if (jpeg_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        bytes_per_line = jpeg_state->decompress_context->output_width * jpeg_state->decompress_context->output_components;
    } else {
        SAIL_TRY(sail_bytes_per_line(jpeg_state->decompress_context->output_width,
                                        jpeg_state->read_options->output_pixel_format,
                                        &bytes_per_line));
    }

    (*image)->width                      = jpeg_state->decompress_context->output_width;
    (*image)->height                     = jpeg_state->decompress_context->output_height;
    (*image)->bytes_per_line             = bytes_per_line;
    (*image)->source_image->pixel_format = color_space_to_pixel_format(jpeg_state->decompress_context->jpeg_color_space);

    if (jpeg_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        (*image)->pixel_format           = (*image)->source_image->pixel_format;
    } else {
        (*image)->pixel_format           = jpeg_state->read_options->output_pixel_format;
    }

    /* Extra scan line used as a buffer when reading CMYK/YCCK images. */
    if (jpeg_state->extra_scan_line_needed_for_cmyk) {
        unsigned src_bytes_per_line;
        SAIL_TRY(sail_bytes_per_line((*image)->width,
                                        (*image)->source_image->pixel_format,
                                        &src_bytes_per_line));

        SAIL_TRY(sail_malloc(&jpeg_state->extra_scan_line, src_bytes_per_line));
    }

    /* Read meta info. */
    if (jpeg_state->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        SAIL_TRY_OR_CLEANUP(fetch_meta_info(jpeg_state->decompress_context, &(*image)->meta_entry_node),
                            /* cleanup */ sail_destroy_image(*image));
    }

    /* Read ICC profile. */
#ifdef HAVE_JPEG_ICCP
    if (jpeg_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        if (jpeg_state->extra_scan_line_needed_for_cmyk) {
            SAIL_LOG_DEBUG("JPEG: Skipping the ICC profile (if any) as we convert from CMYK");
        } else {
            SAIL_TRY_OR_CLEANUP(fetch_iccp(jpeg_state->decompress_context, &(*image)->iccp),
                                /* cleanup */ sail_destroy_image(*image));
        }
    }
#endif

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string((*image)->source_image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("JPEG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(jpeg_state->read_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("JPEG: Output pixel format is %s", pixel_format_str);

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

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->libjpeg_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *scanline = (unsigned char *)image->pixels + row * image->bytes_per_line;

        /* Convert the CMYK image to BPP32-RGBA/BPP32-BGRA/etc. */
        if (jpeg_state->extra_scan_line_needed_for_cmyk) {
            JSAMPROW samprow = (JSAMPROW)jpeg_state->extra_scan_line;
            (void)jpeg_read_scanlines(jpeg_state->decompress_context, &samprow, 1);
            SAIL_TRY(convert_cmyk(jpeg_state->extra_scan_line, scanline, image->width, image->pixel_format));
        } else {
            JSAMPROW samprow = (JSAMPROW)scanline;
            (void)jpeg_read_scanlines(jpeg_state->decompress_context, &samprow, 1);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_read_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct jpeg_state *jpeg_state = (struct jpeg_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        destroy_jpeg_state(jpeg_state);
        return SAIL_ERROR_UNDERLYING_CODEC;
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

SAIL_EXPORT sail_status_t sail_plugin_write_init_v3(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    struct jpeg_state *jpeg_state;
    SAIL_TRY(alloc_jpeg_state(&jpeg_state));

    *state = jpeg_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_write_options(write_options, &jpeg_state->write_options));

    /* Create decompress context. */
    void *ptr;
    SAIL_TRY(sail_malloc(&ptr, sizeof(struct jpeg_compress_struct)));
    jpeg_state->compress_context = ptr;

    /* Sanity check. */
    if (jpeg_state->write_options->compression != SAIL_COMPRESSION_JPEG) {
        return SAIL_ERROR_UNSUPPORTED_COMPRESSION_TYPE;
    }

    /* Error handling setup. */
    jpeg_state->compress_context->err = jpeg_std_error(&jpeg_state->error_context.jpeg_error_mgr);
    jpeg_state->error_context.jpeg_error_mgr.error_exit = my_error_exit;
    jpeg_state->error_context.jpeg_error_mgr.output_message = my_output_message;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* JPEG setup. */
    jpeg_create_compress(jpeg_state->compress_context);
    jpeg_sail_io_dest(jpeg_state->compress_context, io);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_seek_next_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->frame_written) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    jpeg_state->frame_written = true;

    /* Error handling setup. */
    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    /* Initialize compression. */
    jpeg_state->compress_context->image_width = image->width;
    jpeg_state->compress_context->image_height = image->height;
    jpeg_state->compress_context->input_components = bits_per_pixel / 8;
    jpeg_state->compress_context->in_color_space = pixel_format_to_color_space(image->pixel_format);

    jpeg_set_defaults(jpeg_state->compress_context);

    /* Compute output pixel format. */
    if (jpeg_state->write_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        J_COLOR_SPACE output_color_space = pixel_format_to_color_space(image->pixel_format);

        if (output_color_space == JCS_UNKNOWN) {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }

        jpeg_set_colorspace(jpeg_state->compress_context, output_color_space);
    } else if (jpeg_state->write_options->output_pixel_format == SAIL_PIXEL_FORMAT_AUTO) {
        J_COLOR_SPACE output_color_space;
        SAIL_TRY(auto_output_color_space(image->pixel_format, &output_color_space));

        jpeg_set_colorspace(jpeg_state->compress_context, output_color_space);
    } else {
        jpeg_set_colorspace(jpeg_state->compress_context, pixel_format_to_color_space(jpeg_state->write_options->output_pixel_format));
    }

    /* Compute image quality. */
    const double compression = (jpeg_state->write_options->compression_level < COMPRESSION_MIN ||
                                jpeg_state->write_options->compression_level > COMPRESSION_MAX)
                                ? COMPRESSION_DEFAULT
                                : jpeg_state->write_options->compression_level;
    jpeg_set_quality(jpeg_state->compress_context, /* to quality */ (int)(COMPRESSION_MAX-compression), true);

    /* Start compression. */
    jpeg_start_compress(jpeg_state->compress_context, true);
    jpeg_state->started_compress = true;

    /* Write meta info. */
    if (jpeg_state->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {
        struct sail_meta_entry_node *meta_entry_node = image->meta_entry_node;

        while (meta_entry_node != NULL) {
            jpeg_write_marker(jpeg_state->compress_context,
                                JPEG_COM,
                                (JOCTET *)meta_entry_node->value,
                                (unsigned int)strlen(meta_entry_node->value));

            meta_entry_node = meta_entry_node->next;
        }
    }

    /* Write ICC profile. */
#ifdef HAVE_JPEG_ICCP
    if (jpeg_state->write_options->io_options & SAIL_IO_OPTION_ICCP && image->iccp != NULL) {
        SAIL_LOG_DEBUG("JPEG: Writing ICC profile");
        jpeg_write_icc_profile(jpeg_state->compress_context, image->iccp->data, image->iccp->data_length);
    }
#endif

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("JPEG: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(jpeg_state->write_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("JPEG: Output pixel format is %s", pixel_format_str);

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

    struct jpeg_state *jpeg_state = (struct jpeg_state *)state;

    if (jpeg_state->libjpeg_error) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        jpeg_state->libjpeg_error = true;
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    for (unsigned row = 0; row < image->height; row++) {
        JSAMPROW samprow = (JSAMPROW)((const unsigned char *)image->pixels + row * image->bytes_per_line);
        jpeg_write_scanlines(jpeg_state->compress_context, &samprow, 1);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_plugin_write_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct jpeg_state *jpeg_state = (struct jpeg_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (setjmp(jpeg_state->error_context.setjmp_buffer) != 0) {
        destroy_jpeg_state(jpeg_state);
        return SAIL_ERROR_UNDERLYING_CODEC;
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
