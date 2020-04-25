/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#include <jpeglib.h>

#include "sail-common.h"

#include "helpers.h"
#include "io_dest.h"
#include "io_src.h"

/*
 * Plugin-specific data types.
 */

static const int COMPRESSION_MIN     = 0;
static const int COMPRESSION_MAX     = 100;
static const int COMPRESSION_DEFAULT = 15;

/*
 * Plugin-specific PIMPL.
 */

struct pimpl {
    struct jpeg_decompress_struct decompress_context;
    struct jpeg_compress_struct compress_context;
    struct my_error_context error_context;
    JSAMPARRAY buffer;
    bool libjpeg_error;
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;
    bool frame_read;
    bool frame_written;
};

static int alloc_pimpl(struct pimpl **pimpl) {

    *pimpl = (struct pimpl *)malloc(sizeof(struct pimpl));

    if (*pimpl == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*pimpl)->buffer        = NULL;
    (*pimpl)->libjpeg_error = false;
    (*pimpl)->read_options  = NULL;
    (*pimpl)->write_options = NULL;
    (*pimpl)->frame_read    = false;
    (*pimpl)->frame_written = false;

    return 0;
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_error_t sail_plugin_read_init_v2(struct sail_io *io, const struct sail_read_options *read_options) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    struct pimpl *pimpl;
    SAIL_TRY(alloc_pimpl(&pimpl));

    io->pimpl = pimpl;

    /* Deep copy read options. */
    pimpl->read_options = (struct sail_read_options *)malloc(sizeof(struct sail_read_options));

    if (pimpl->read_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(pimpl->read_options, read_options, sizeof(struct sail_read_options));

    /* Error handling setup. */
    pimpl->decompress_context.err = jpeg_std_error(&pimpl->error_context.jpeg_error_mgr);
    pimpl->error_context.jpeg_error_mgr.error_exit = my_error_exit;
    pimpl->error_context.jpeg_error_mgr.output_message = my_output_message;

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* JPEG setup. */
    jpeg_create_decompress(&pimpl->decompress_context);
    jpeg_sail_io_src(&pimpl->decompress_context, io);

    if (pimpl->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        jpeg_save_markers(&pimpl->decompress_context, JPEG_COM, 0xffff);
    }

    jpeg_read_header(&pimpl->decompress_context, true);

    /* Handle the requested color space. */
    if (pimpl->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_SOURCE) {
        pimpl->decompress_context.out_color_space = pimpl->decompress_context.jpeg_color_space;
    } else {
        J_COLOR_SPACE requested_color_space = pixel_format_to_color_space(pimpl->read_options->output_pixel_format);

        if (requested_color_space == JCS_UNKNOWN) {
            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        pimpl->decompress_context.out_color_space = requested_color_space;
    }

    /* We don't want colormapped output. */
    pimpl->decompress_context.quantize_colors = false;

    /* Launch decompression! */
    jpeg_start_decompress(&pimpl->decompress_context);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_frame_v2(struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_IO(io);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    if (pimpl->frame_read) {
        return SAIL_NO_MORE_FRAMES;
    }

    pimpl->frame_read = true;
    SAIL_TRY(sail_alloc_image(image));

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    const int bytes_per_line = pimpl->decompress_context.output_width * pimpl->decompress_context.output_components;

    /* Buffer to put scan lines into. libjpeg will automatically free it. */
    pimpl->buffer = (*pimpl->decompress_context.mem->alloc_sarray)((j_common_ptr)&pimpl->decompress_context,
                                                                    JPOOL_IMAGE,
                                                                    bytes_per_line,
                                                                    1);

    if (pimpl->buffer == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    /* Image properties. */
    (*image)->width               = pimpl->decompress_context.output_width;
    (*image)->height              = pimpl->decompress_context.output_height;
    (*image)->bytes_per_line      = bytes_per_line;
    (*image)->pixel_format        = color_space_to_pixel_format(pimpl->decompress_context.out_color_space);
    (*image)->passes              = 1;
    (*image)->source_pixel_format = color_space_to_pixel_format(pimpl->decompress_context.jpeg_color_space);

    /* Read meta info. */
    if (pimpl->read_options->io_options & SAIL_IO_OPTION_META_INFO) {
        jpeg_saved_marker_ptr it = pimpl->decompress_context.marker_list;
        struct sail_meta_entry_node *last_meta_entry_node = NULL;

        while(it != NULL) {
            if(it->marker == JPEG_COM) {
                struct sail_meta_entry_node *meta_entry_node;

                SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
                SAIL_TRY_OR_CLEANUP(sail_strdup("Comment", &meta_entry_node->key),
                                    /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));
                SAIL_TRY_OR_CLEANUP(sail_strdup_length((const char *)it->data, it->data_length, &meta_entry_node->value),
                                    /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));

                if ((*image)->meta_entry_node == NULL) {
                    (*image)->meta_entry_node = last_meta_entry_node = meta_entry_node;
                } else {
                    last_meta_entry_node->next = meta_entry_node;
                    last_meta_entry_node = meta_entry_node;
                }
            }

            it = it->next;
        }
    }

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_pass_v2(struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_scan_line_v2(struct sail_io *io, const struct sail_image *image, void *scanline) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_SCAN_LINE_PTR(scanline);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    if (pimpl->libjpeg_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    const int color_components = pimpl->decompress_context.output_components;

    (void)jpeg_read_scanlines(&pimpl->decompress_context, pimpl->buffer, 1);

    memcpy(scanline, pimpl->buffer[0], image->width * color_components);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_read_alloc_scan_line_v2(struct sail_io *io, const struct sail_image *image, void **scanline) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    const int color_components = pimpl->decompress_context.output_components;

    *scanline = malloc(image->width * color_components);

    if (*scanline == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    return sail_plugin_read_scan_line_v2(io, image, *scanline);
}

SAIL_EXPORT sail_error_t sail_plugin_read_finish_v2(struct sail_io *io) {

    SAIL_CHECK_IO(io);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    sail_destroy_read_options(pimpl->read_options);

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    jpeg_abort_decompress(&pimpl->decompress_context);
    jpeg_destroy_decompress(&pimpl->decompress_context);

    return 0;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_error_t sail_plugin_write_init_v2(struct sail_io *io, const struct sail_write_options *write_options) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    struct pimpl *pimpl;

    SAIL_TRY(alloc_pimpl(&pimpl));

    io->pimpl = pimpl;

    /* Deep copy write options. */
    pimpl->write_options = (struct sail_write_options *)malloc(sizeof(struct sail_write_options));

    if (pimpl->write_options == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(pimpl->write_options, write_options, sizeof(struct sail_write_options));

    /* Sanity check. */
    if (pixel_format_to_color_space(pimpl->write_options->output_pixel_format) == JCS_UNKNOWN) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (pimpl->write_options->compression_type != 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    /* Error handling setup. */
    pimpl->compress_context.err = jpeg_std_error(&pimpl->error_context.jpeg_error_mgr);
    pimpl->error_context.jpeg_error_mgr.error_exit = my_error_exit;
    pimpl->error_context.jpeg_error_mgr.output_message = my_output_message;

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    /* JPEG setup. */
    jpeg_create_compress(&pimpl->compress_context);
    jpeg_sail_io_dest(&pimpl->compress_context, io);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_frame_v2(struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    /* Sanity check. */
    if (pixel_format_to_color_space(image->pixel_format) == JCS_UNKNOWN) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    if (pimpl->frame_written) {
        return SAIL_NO_MORE_FRAMES;
    }

    pimpl->frame_written = true;

    /* Error handling setup. */
    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    int bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    pimpl->compress_context.image_width = image->width;
    pimpl->compress_context.image_height = image->height;
    pimpl->compress_context.input_components = bits_per_pixel / 8;
    pimpl->compress_context.in_color_space = pixel_format_to_color_space(image->pixel_format);

    jpeg_set_defaults(&pimpl->compress_context);
    jpeg_set_colorspace(&pimpl->compress_context, pixel_format_to_color_space(pimpl->write_options->output_pixel_format));

    const int compression = (pimpl->write_options->compression < COMPRESSION_MIN ||
                                pimpl->write_options->compression > COMPRESSION_MAX)
                            ? COMPRESSION_DEFAULT
                            : pimpl->write_options->compression;
    jpeg_set_quality(&pimpl->compress_context, /* to quality */COMPRESSION_MAX-compression, true);

    jpeg_start_compress(&pimpl->compress_context, true);

    /* Write meta info. */
    if (pimpl->write_options->io_options & SAIL_IO_OPTION_META_INFO && image->meta_entry_node != NULL) {

        struct sail_meta_entry_node *meta_entry_node = image->meta_entry_node;

        while (meta_entry_node != NULL) {
            jpeg_write_marker(&pimpl->compress_context,
                                JPEG_COM,
                                (JOCTET *)meta_entry_node->value,
                                (unsigned int)strlen(meta_entry_node->value));

            meta_entry_node = meta_entry_node->next;
        }
    }

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_pass_v2(struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_scan_line_v2(struct sail_io *io, const struct sail_image *image, const void *scanline) {

    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_SCAN_LINE_PTR(scanline);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    if (pimpl->libjpeg_error) {
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    JSAMPROW row = (JSAMPROW)scanline;

    jpeg_write_scanlines(&pimpl->compress_context, &row, 1);

    return 0;
}

SAIL_EXPORT sail_error_t sail_plugin_write_finish_v2(struct sail_io *io) {

    SAIL_CHECK_IO(io);

    struct pimpl *pimpl = (struct pimpl *)io->pimpl;
    SAIL_CHECK_PIMPL_PTR(pimpl);

    sail_destroy_write_options(pimpl->write_options);

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return SAIL_UNDERLYING_CODEC_ERROR;
    }

    jpeg_finish_compress(&pimpl->compress_context);
    jpeg_destroy_compress(&pimpl->compress_context);

    return 0;
}
