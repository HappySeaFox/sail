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

static const char * const TGA_SIGNATURE   = "TRUEVISION-XFILE.";
static const int          TGA_FOOTER_SIZE = 26;

/*
 * Codec-specific state.
 */
struct tga_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    struct TgaFileHeader file_header;
    struct TgaFooter footer;

    bool frame_loaded;
    bool tga2;
    bool flipped_h;
    bool flipped_v;
};

static sail_status_t alloc_tga_state(struct tga_state **tga_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tga_state), &ptr));
    *tga_state = ptr;

    (*tga_state)->load_options = NULL;
    (*tga_state)->save_options = NULL;

    (*tga_state)->frame_loaded  = false;
    (*tga_state)->tga2          = false;
    (*tga_state)->flipped_h     = false;
    (*tga_state)->flipped_v     = false;

    return SAIL_OK;
}

static void destroy_tga_state(struct tga_state *tga_state) {

    if (tga_state == NULL) {
        return;
    }

    sail_destroy_load_options(tga_state->load_options);
    sail_destroy_save_options(tga_state->save_options);

    sail_free(tga_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_tga(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct tga_state *tga_state;
    SAIL_TRY(alloc_tga_state(&tga_state));
    *state = tga_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &tga_state->load_options));

    /* Read TGA footer. */
    SAIL_TRY(io->seek(io->stream, -TGA_FOOTER_SIZE, SEEK_END));
    SAIL_TRY(tga_private_read_file_footer(io, &tga_state->footer));
    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    tga_state->tga2 = strcmp(TGA_SIGNATURE, (const char *)tga_state->footer.signature) == 0;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_tga(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct tga_state *tga_state = (struct tga_state *)state;

    if (tga_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    tga_state->frame_loaded = true;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(tga_private_read_file_header(io, &tga_state->file_header),
                        /* cleanup */ sail_destroy_image(image_local));

    tga_state->flipped_h = tga_state->file_header.descriptor & 0x10;        /* 4th bit set = flipped H.   */
    tga_state->flipped_v = (tga_state->file_header.descriptor & 0x20) == 0; /* 5th bit unset = flipped V. */

    image_local->source_image->pixel_format = tga_private_sail_pixel_format(tga_state->file_header.image_type, tga_state->file_header.bpp);

    if (image_local->source_image->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (tga_state->flipped_h) {
        image_local->orientation               = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY;
        image_local->source_image->orientation = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY;
    }
    if (tga_state->flipped_v) {
        image_local->orientation               = SAIL_ORIENTATION_MIRRORED_VERTICALLY;
        image_local->source_image->orientation = SAIL_ORIENTATION_MIRRORED_VERTICALLY;
    }

    switch (tga_state->file_header.image_type) {
        case TGA_INDEXED_RLE:
        case TGA_TRUE_COLOR_RLE:
        case TGA_GRAY_RLE: {
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

    /* Extension area. */
    if (tga_state->tga2 && tga_state->footer.extension_area_offset > 0) {
        /* Seek to offset. */
        size_t offset;
        SAIL_TRY_OR_CLEANUP(io->tell(io->stream, &offset),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(io->seek(io->stream, (long)tga_state->footer.extension_area_offset, SEEK_SET),
                            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(tga_private_fetch_extension(io, &image_local->gamma, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(io->seek(io->stream, (long)offset, SEEK_SET),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Palette. */
    if (tga_state->file_header.color_map_type == TGA_HAS_COLOR_MAP) {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_palette(io, &tga_state->file_header, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_tga(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct tga_state *tga_state = (struct tga_state *)state;

    switch (tga_state->file_header.image_type) {
        case TGA_INDEXED:
        case TGA_TRUE_COLOR:
        case TGA_GRAY: {
            SAIL_TRY(io->strict_read(io->stream, image->pixels, (size_t)image->bytes_per_line * image->height));
            break;
        }
        case TGA_INDEXED_RLE:
        case TGA_TRUE_COLOR_RLE:
        case TGA_GRAY_RLE: {
            const unsigned pixel_size = (tga_state->file_header.bpp + 7) / 8;
            const unsigned pixels_num = image->width * image->height;

            unsigned char *pixels = image->pixels;

            for (unsigned i = 0; i < pixels_num;) {
                unsigned char marker;
                SAIL_TRY(io->strict_read(io->stream, &marker, 1));

                unsigned count = (marker & 0x7F) + 1;

                /* 7th bit set = RLE packet. */
                if (marker & 0x80) {
                    unsigned char pixel[4];

                    SAIL_TRY(io->strict_read(io->stream, pixel, pixel_size));

                    for (unsigned j = 0; j < count; j++, i++) {
                        memcpy(pixels, pixel, pixel_size);
                        pixels += pixel_size;
                    }
                } else {
                    for (unsigned j = 0; j < count; j++, i++) {
                        SAIL_TRY(io->strict_read(io->stream, pixels, pixel_size));
                        pixels += pixel_size;
                    }
                }
            }
            break;
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_tga(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct tga_state *tga_state = (struct tga_state *)(*state);

    *state = NULL;

    destroy_tga_state(tga_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_tga(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_tga(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_tga(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
