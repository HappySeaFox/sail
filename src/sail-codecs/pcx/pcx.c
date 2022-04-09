/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

/* PCX signature. */
static const unsigned SAIL_PCX_SIGNATURE = 0x0A;

/* RLE markers. */
static const uint8_t SAIL_PCX_RLE_MARKER     = 0xC0;
static const uint8_t SAIL_PCX_RLE_COUNT_MASK = 0x3F;

/*
 * Codec-specific state.
 */
struct pcx_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    struct SailPcxHeader pcx_header;
    unsigned char *scanline_buffer; /* buffer to read a single plane scan line. */

    bool frame_loaded;
};

static sail_status_t alloc_pcx_state(struct pcx_state **pcx_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct pcx_state), &ptr));
    *pcx_state = ptr;

    (*pcx_state)->load_options = NULL;
    (*pcx_state)->save_options = NULL;

    (*pcx_state)->scanline_buffer = NULL;
    (*pcx_state)->frame_loaded    = false;

    return SAIL_OK;
}

static void destroy_pcx_state(struct pcx_state *pcx_state) {

    if (pcx_state == NULL) {
        return;
    }

    sail_destroy_load_options(pcx_state->load_options);
    sail_destroy_save_options(pcx_state->save_options);

    sail_free(pcx_state->scanline_buffer);

    sail_free(pcx_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_pcx(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct pcx_state *pcx_state;
    SAIL_TRY(alloc_pcx_state(&pcx_state));
    *state = pcx_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &pcx_state->load_options));

    /* Read PCX header. */
    SAIL_TRY(pcx_private_read_header(io, &pcx_state->pcx_header));

    if (pcx_state->pcx_header.id != SAIL_PCX_SIGNATURE) {
        SAIL_LOG_ERROR("PCX: ID is %u, but must be %u", pcx_state->pcx_header.id, SAIL_PCX_SIGNATURE);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if (pcx_state->pcx_header.bytes_per_line == 0) {
        SAIL_LOG_ERROR("PCX: Bytes per line is 0");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SAIL_LOG_TRACE("PCX: planes(%u), bytes per line(%u), compressed(%s)", pcx_state->pcx_header.planes, pcx_state->pcx_header.bytes_per_line,
                    (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING) ? "no" : "yes");

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_pcx(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct pcx_state *pcx_state = (struct pcx_state *)state;

    if (pcx_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    pcx_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(pcx_private_sail_pixel_format(
                            pcx_state->pcx_header.bits_per_plane,
                            pcx_state->pcx_header.planes,
                            pcx_state->pcx_header.palette_info,
                            &image_local->source_image->pixel_format),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->compression = (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING) ? SAIL_COMPRESSION_NONE : SAIL_COMPRESSION_RLE;

    image_local->width = pcx_state->pcx_header.xmax - pcx_state->pcx_header.xmin + 1;
    image_local->height = pcx_state->pcx_header.ymax - pcx_state->pcx_header.ymin + 1;
    image_local->pixel_format = image_local->source_image->pixel_format;
    image_local->bytes_per_line = pcx_state->pcx_header.bytes_per_line * pcx_state->pcx_header.planes;

    /* Scan line buffer to store planes so we can merge them later into individual pixels. */
    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(image_local->bytes_per_line, &ptr),
                        /* cleanup */ sail_destroy_image(image_local));
    pcx_state->scanline_buffer = ptr;

    /* Build palette if needed. */
    SAIL_TRY_OR_CLEANUP(pcx_private_build_palette(image_local->pixel_format, io, pcx_state->pcx_header.palette, &image_local->palette),
                        /* cleanup */ sail_destroy_image(image_local));

    if (pcx_state->pcx_header.hdpi > 0 && pcx_state->pcx_header.vdpi > 0) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_resolution_from_data(SAIL_RESOLUTION_UNIT_INCH,
                                                            pcx_state->pcx_header.hdpi,
                                                            pcx_state->pcx_header.vdpi,
                                                            &image_local->resolution),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_pcx(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    const struct pcx_state *pcx_state = (struct pcx_state *)state;

    if (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING) {
        SAIL_TRY(pcx_private_read_uncompressed(io, pcx_state->pcx_header.bytes_per_line, pcx_state->pcx_header.planes, pcx_state->scanline_buffer, image));
    } else {
        for (unsigned row = 0; row < image->height; row++) {
            unsigned buffer_offset = 0;

            /* Decode all planes of a single scan line. */
            for (unsigned bytes = 0; bytes < image->bytes_per_line;) {
                uint8_t marker;
                SAIL_TRY(io->strict_read(io->stream, &marker, sizeof(marker)));

                uint8_t count;
                uint8_t value;

                /* RLE marker set. */
                if ((marker & SAIL_PCX_RLE_MARKER) == SAIL_PCX_RLE_MARKER) {
                    count = marker & SAIL_PCX_RLE_COUNT_MASK;
                    SAIL_TRY(io->strict_read(io->stream, &value, sizeof(value)));
                } else {
                    /* Pixel value. */
                    count = 1;
                    value = marker;
                }

                bytes += count;

                memset(pcx_state->scanline_buffer + buffer_offset, value, count);
                buffer_offset += count;
            }

            /* Merge planes into the image pixels. */
            unsigned char * const scan = (unsigned char *)image->pixels + image->bytes_per_line * row;

            for (unsigned plane = 0; plane < pcx_state->pcx_header.planes; plane++) {
                const unsigned buffer_plane_offset = plane * pcx_state->pcx_header.bytes_per_line;

                for (unsigned column = 0; column < pcx_state->pcx_header.bytes_per_line; column++) {
                    *(scan + column * pcx_state->pcx_header.planes + plane) = *(pcx_state->scanline_buffer + buffer_plane_offset + column);
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_pcx(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct pcx_state *pcx_state = (struct pcx_state *)(*state);

    *state = NULL;

    destroy_pcx_state(pcx_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_pcx(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_pcx(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_pcx(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_pcx(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
