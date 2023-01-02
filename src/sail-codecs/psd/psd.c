/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#include "sail-common.h"

#include "helpers.h"

static const unsigned SAIL_PSD_MAGIC = 0x38425053;

/*
 * Codec-specific state.
 */
struct psd_state {
    struct sail_io *io;
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;
/*
	u32 width, height;
	u16 channels, depth, mode, compression;
	RGBA **last;
	u8 *L;
	RGB pal[256];
*/
};

static sail_status_t alloc_psd_state(struct psd_state **psd_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct psd_state), &ptr));
    *psd_state = ptr;

    (*psd_state)->io           = NULL;
    (*psd_state)->load_options = NULL;
    (*psd_state)->save_options = NULL;

    (*psd_state)->frame_loaded = false;

    return SAIL_OK;
}

static void destroy_psd_state(struct psd_state *psd_state) {

    if (psd_state == NULL) {
        return;
    }

    sail_destroy_load_options(psd_state->load_options);
    sail_destroy_save_options(psd_state->save_options);

    sail_free(psd_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_psd(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct psd_state *psd_state;
    SAIL_TRY(alloc_psd_state(&psd_state));
    *state = psd_state;

    /* Save I/O for further operations. */
    psd_state->io = io;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &psd_state->load_options));

    /* Init decoder here. */
    uint32_t magic;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &magic));

    if (magic != SAIL_PSD_MAGIC) {
        SAIL_LOG_ERROR("PSD: Invalid magic 0x%X (expected 0x%X)", magic, SAIL_PSD_MAGIC);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    uint16_t version;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &version));

    if (version != 1) {
        SAIL_LOG_ERROR("PSD: Invalid version %u", version);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_psd(void *state, struct sail_image **image) {

    struct psd_state *psd_state = (struct psd_state *)state;

    if (psd_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    psd_state->frame_loaded = true;

    /* Skip dummy bytes. */
    SAIL_TRY(psd_state->io->seek(psd_state->io->stream, 6, SEEK_CUR));

    /* Read PSD header. */
    uint16_t channels;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &channels));

    uint32_t height;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &height));

    uint32_t width;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &width));

    uint16_t depth;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &depth));

    if (depth != 8) {
        SAIL_LOG_ERROR("PSD: Bit depth %u is not supported", depth);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    uint16_t mode;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &mode));

    uint32_t data_size;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &data_size));

    /* Palette. */
    struct sail_palette *palette = NULL;

    if (data_size > 0) {
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 256, &palette));
        SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, palette->data, 256*3));
    }

    /* Skip the image resources. */
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &data_size));
    SAIL_TRY(psd_state->io->seek(psd_state->io->stream, data_size, SEEK_CUR));
    /* Skip the layer and mask information. */
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &data_size));
    SAIL_TRY(psd_state->io->seek(psd_state->io->stream, data_size, SEEK_CUR));

    /* Compression. */
    uint16_t compression;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &compression));

    if (compression != SAIL_PSD_COMPRESSION_NONE && compression != SAIL_PSD_COMPRESSION_RLE) {
        SAIL_LOG_ERROR("PSD: Unsuppored compression value #%u", compression);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Allocate image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(psd_private_sail_pixel_format(mode, channels, &image_local->source_image->pixel_format),
                        /* cleanup */ sail_destroy_image(image_local));
    image_local->source_image->compression  = (compression == SAIL_PSD_COMPRESSION_NONE) ? SAIL_COMPRESSION_NONE : SAIL_COMPRESSION_RLE;

    image_local->width          = width;
    image_local->height         = height;
    image_local->pixel_format   = image_local->source_image->pixel_format;
    image_local->palette        = palette;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_psd(void *state, struct sail_image *image) {

    const struct psd_state *psd_state = (struct psd_state *)state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_psd(void **state) {

    struct psd_state *psd_state = (struct psd_state *)(*state);

    *state = NULL;

    destroy_psd_state(psd_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_psd(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_psd(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_psd(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_psd(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
