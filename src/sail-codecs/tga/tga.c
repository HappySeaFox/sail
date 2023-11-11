/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <sail-common/sail-common.h>

#include "helpers.h"

static const char * const TGA_SIGNATURE   = "TRUEVISION-XFILE.";
static const int          TGA_FOOTER_SIZE = 26;

/*
 * Codec-specific state.
 */
struct tga_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    struct TgaFileHeader file_header;
    struct TgaFooter footer;

    bool frame_loaded;
    bool tga2;
    bool flipped_h;
    bool flipped_v;
};

static sail_status_t alloc_tga_state(struct sail_io *io,
                                        const struct sail_load_options *load_options,
                                        const struct sail_save_options *save_options,
                                        struct tga_state **tga_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tga_state), &ptr));
    *tga_state = ptr;

    **tga_state = (struct tga_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded  = false,
        .tga2          = false,
        .flipped_h     = false,
        .flipped_v     = false,
    };

    return SAIL_OK;
}

static void destroy_tga_state(struct tga_state *tga_state) {

    if (tga_state == NULL) {
        return;
    }

    sail_free(tga_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_tga(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct tga_state *tga_state;
    SAIL_TRY(alloc_tga_state(io, load_options, NULL, &tga_state));
    *state = tga_state;

    /* Read TGA footer. */
    SAIL_TRY(tga_state->io->seek(tga_state->io->stream, -TGA_FOOTER_SIZE, SEEK_END));
    SAIL_TRY(tga_private_read_file_footer(io, &tga_state->footer));
    SAIL_TRY(tga_state->io->seek(tga_state->io->stream, 0, SEEK_SET));

    tga_state->tga2 = strcmp(TGA_SIGNATURE, (const char *)tga_state->footer.signature) == 0;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_tga(void *state, struct sail_image **image) {

    struct tga_state *tga_state = state;

    if (tga_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    tga_state->frame_loaded = true;

    SAIL_TRY(tga_private_read_file_header(tga_state->io, &tga_state->file_header));

    tga_state->flipped_h = tga_state->file_header.descriptor & 0x10;        /* 4th bit set = flipped H.   */
    tga_state->flipped_v = (tga_state->file_header.descriptor & 0x20) == 0; /* 5th bit unset = flipped V. */

    enum SailPixelFormat pixel_format = tga_private_sail_pixel_format(tga_state->file_header.image_type, tga_state->file_header.bpp);

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (tga_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        if (tga_state->flipped_h) {
            image_local->source_image->orientation = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY;
        } else if (tga_state->flipped_v) {
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
    }

    image_local->width          = tga_state->file_header.width;
    image_local->height         = tga_state->file_header.height;
    image_local->pixel_format   = pixel_format;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Identificator. */
    if (tga_state->file_header.id_length > 0) {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_id(tga_state->io, &tga_state->file_header, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Extension area. */
    if (tga_state->tga2 && tga_state->footer.extension_area_offset > 0) {
        /* Seek to offset. */
        size_t offset;
        SAIL_TRY_OR_CLEANUP(tga_state->io->tell(tga_state->io->stream, &offset),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(tga_state->io->seek(tga_state->io->stream, (long)tga_state->footer.extension_area_offset, SEEK_SET),
                            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(tga_private_fetch_extension(tga_state->io, &image_local->gamma, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(tga_state->io->seek(tga_state->io->stream, (long)offset, SEEK_SET),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Palette. */
    if (tga_state->file_header.color_map_type == TGA_HAS_COLOR_MAP) {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_palette(tga_state->io, &tga_state->file_header, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_tga(void *state, struct sail_image *image) {

    struct tga_state *tga_state = state;

    switch (tga_state->file_header.image_type) {
        case TGA_INDEXED:
        case TGA_TRUE_COLOR:
        case TGA_GRAY: {
            SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, image->pixels, (size_t)image->bytes_per_line * image->height));
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
                SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, &marker, 1));

                unsigned count = (marker & 0x7F) + 1;

                /* 7th bit set = RLE packet. */
                if (marker & 0x80) {
                    unsigned char pixel[4];

                    SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, pixel, pixel_size));

                    for (unsigned j = 0; j < count; j++, i++) {
                        memcpy(pixels, pixel, pixel_size);
                        pixels += pixel_size;
                    }
                } else {
                    for (unsigned j = 0; j < count; j++, i++) {
                        SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, pixels, pixel_size));
                        pixels += pixel_size;
                    }
                }
            }
            break;
        }
    }

    /* TODO: We can avoid this by putting pixels in reverse order like in the BMP codec. */
    if (tga_state->flipped_v) {
        sail_mirror_vertically(image);
    }
    if (tga_state->flipped_h) {
        sail_mirror_horizontally(image);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_tga(void **state) {

    struct tga_state *tga_state = *state;

    *state = NULL;

    destroy_tga_state(tga_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_tga(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_tga(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_tga(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_tga(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
