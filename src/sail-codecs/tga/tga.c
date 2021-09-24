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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

static const char * const TGA_SIGNATURE = "TRUEVISION-XFILE.";

/*
 * Codec-specific state.
 */
struct tga_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    struct TgaFileHeader file_header;
    struct TgaFooter footer;

    bool frame_read;
    bool tga2;
    bool flipped_h;
    bool flipped_v;
};

static sail_status_t alloc_tga_state(struct tga_state **tga_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tga_state), &ptr));
    *tga_state = ptr;

    (*tga_state)->read_options  = NULL;
    (*tga_state)->write_options = NULL;

    (*tga_state)->frame_read    = false;
    (*tga_state)->tga2          = false;
    (*tga_state)->flipped_h     = false;
    (*tga_state)->flipped_v     = false;

    return SAIL_OK;
}

static void destroy_tga_state(struct tga_state *tga_state) {

    if (tga_state == NULL) {
        return;
    }

    sail_destroy_read_options(tga_state->read_options);
    sail_destroy_write_options(tga_state->write_options);

    sail_free(tga_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v5_tga(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(read_options);

    /* Allocate a new state. */
    struct tga_state *tga_state;
    SAIL_TRY(alloc_tga_state(&tga_state));
    *state = tga_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &tga_state->read_options));

    SAIL_TRY(io->seek(io->stream, -(long)(sizeof(tga_state->footer.signature)), SEEK_END));
    SAIL_TRY(io->strict_read(io->stream, tga_state->footer.signature, sizeof(tga_state->footer.signature)));

    tga_state->tga2 = strcmp(TGA_SIGNATURE, (const char *)tga_state->footer.signature) == 0;

    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v5_tga(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct tga_state *tga_state = (struct tga_state *)state;

    if (tga_state->frame_read) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    tga_state->frame_read = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(tga_private_read_file_header(io, &tga_state->file_header),
                        /* cleanup */ sail_destroy_image(image_local));

    tga_state->flipped_h      = tga_state->file_header.descriptor & 0x10;        /* 4th bit set = flipped H.   */
    tga_state->flipped_v      = (tga_state->file_header.descriptor & 0x20) == 0; /* 5th bit unset = flipped V. */

    image_local->source_image->pixel_format = tga_private_sail_pixel_format(tga_state->file_header.image_type, tga_state->file_header.bpp);

    if (tga_state->flipped_h) {
        image_local->source_image->properties &= SAIL_IMAGE_PROPERTY_FLIPPED_HORIZONTALLY;
    }
    if (tga_state->flipped_v) {
        image_local->source_image->properties &= SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY;
    }

    if (image_local->source_image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    switch (tga_state->file_header.image_type) {
        case TGA_INDEXED_RLE:
        case TGA_TRUE_COLOR_RLE:
        case TGA_MONO_RLE: {
            image_local->source_image->compression = SAIL_COMPRESSION_RLE;
            break;
        }
        default: {
            image_local->source_image->compression = SAIL_COMPRESSION_NONE;
        }
    }

    image_local->width = tga_state->file_header.width;
    image_local->height = tga_state->file_header.height;
    image_local->pixel_format = image_local->source_image->pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Identificator. */
    if (tga_state->file_header.id_length > 0) {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_id(io, &tga_state->file_header, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Palette. */
    if (tga_state->file_header.color_map_type == TGA_HAS_COLOR_MAP) {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_palette(io, &tga_state->file_header, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* flip state. */

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v5_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v5_tga(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct tga_state *tga_state = (struct tga_state *)state;

    switch (tga_state->file_header.image_type) {
        case TGA_INDEXED:
        case TGA_TRUE_COLOR:
        case TGA_MONO: {
            SAIL_TRY(io->strict_read(io->stream, image->pixels, image->bytes_per_line * image->height));
            break;
        }
        default: {
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v5_tga(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct tga_state *tga_state = (struct tga_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_tga_state(tga_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v5_tga(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v5_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v5_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v5_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v5_tga(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
