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

#include <sail-common/sail-common.h>

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

    uint16_t channels;
    uint16_t depth;
    enum SailPsdCompression compression;
    unsigned bytes_per_channel;
    unsigned char *scan_buffer;
    struct sail_palette *palette;
};

static sail_status_t alloc_psd_state(struct psd_state **psd_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct psd_state), &ptr));
    *psd_state = ptr;

    (*psd_state)->io           = NULL;
    (*psd_state)->load_options = NULL;
    (*psd_state)->save_options = NULL;

    (*psd_state)->frame_loaded = false;

    (*psd_state)->channels          = 0;
    (*psd_state)->depth             = 0;
    (*psd_state)->compression       = SAIL_PSD_COMPRESSION_NONE;
    (*psd_state)->bytes_per_channel = 0;
    (*psd_state)->scan_buffer       = NULL;
    (*psd_state)->palette           = NULL;

    return SAIL_OK;
}

static void destroy_psd_state(struct psd_state *psd_state) {

    if (psd_state == NULL) {
        return;
    }

    sail_free(psd_state->scan_buffer);

    sail_destroy_load_options(psd_state->load_options);
    sail_destroy_save_options(psd_state->save_options);

    sail_destroy_palette(psd_state->palette);

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

    /* Init decoder. PSD spec: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_89817 */
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

    struct psd_state *psd_state = state;

    if (psd_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    psd_state->frame_loaded = true;

    /* Skip dummy bytes. */
    SAIL_TRY(psd_state->io->seek(psd_state->io->stream, 6, SEEK_CUR));

    /* Read PSD header. */
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &psd_state->channels));

    uint32_t height;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &height));

    uint32_t width;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &width));

    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &psd_state->depth));

    uint16_t mode;
    SAIL_TRY(psd_private_get_big_endian_uint16_t(psd_state->io, &mode));

    uint32_t data_size;
    SAIL_TRY(psd_private_get_big_endian_uint32_t(psd_state->io, &data_size));

    /* Palette. */
    if (data_size > 0) {
        SAIL_LOG_TRACE("PSD: Palette data size: %u", data_size);
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 256, &psd_state->palette));

        /* Merge RR GG BB... to RGB RGB... */
        unsigned char buf[256*3];
        SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, buf, sizeof(buf)));

        unsigned char *palette_data = psd_state->palette->data;

        for (unsigned i = 0; i < 256; i++) {
            for (unsigned channel = 0; channel < 3; channel++) {
                *palette_data++ = buf[256 * channel + i];
            }
        }
    } else if (mode == SAIL_PSD_MODE_BITMAP) {
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, &psd_state->palette));

        unsigned char *palette_data = psd_state->palette->data;

        *palette_data++ = 255;
        *palette_data++ = 255;
        *palette_data++ = 255;

        *palette_data++ = 0;
        *palette_data++ = 0;
        *palette_data++ = 0;
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

    psd_state->compression = compression;

    /* Skip byte counts for all the scan lines. */
    if (psd_state->compression == SAIL_PSD_COMPRESSION_RLE) {
        SAIL_TRY(psd_state->io->seek(psd_state->io->stream, (long)height * psd_state->channels * 2, SEEK_CUR));
    }

    /* Used to optimize uncompressed readings. */
    if (psd_state->compression == SAIL_PSD_COMPRESSION_NONE) {
        psd_state->bytes_per_channel = ((unsigned)width * psd_state->depth + 7) / 8;

        void *ptr;
        SAIL_TRY(sail_malloc(psd_state->bytes_per_channel, &ptr));
        psd_state->scan_buffer = ptr;
    }

    SAIL_LOG_TRACE("PSD: mode(%u), channels(%u), depth(%u)", mode, psd_state->channels, psd_state->depth);

    enum SailPixelFormat pixel_format;
    SAIL_TRY(psd_private_sail_pixel_format(mode, psd_state->channels, psd_state->depth, &pixel_format));

    /* Allocate image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (psd_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression  = psd_private_sail_compression(psd_state->compression);
    }

    image_local->width          = width;
    image_local->height         = height;
    image_local->pixel_format   = pixel_format;
    image_local->palette        = psd_state->palette;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Palette has been moved. */
    psd_state->palette = NULL;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_psd(void *state, struct sail_image *image) {

    const struct psd_state *psd_state = state;

    const unsigned bpp = (psd_state->channels * psd_state->depth + 7) / 8;

    if (psd_state->compression == SAIL_PSD_COMPRESSION_RLE) {
        for (unsigned channel = 0; channel < psd_state->channels; channel++) {
            for (unsigned row = 0; row < image->height; row++) {
                for (unsigned count = 0; count < image->width; ) {
                    unsigned char c;
                    SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, &c, sizeof(c)));

                    if (c > 128) {
                        c ^= 0xff;
                        c += 2;

                        unsigned char value;
                        SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, &value, sizeof(value)));

                        for (unsigned i = count; i < count + c; i++) {
                            unsigned char *scan = (unsigned char *)sail_scan_line(image, row) + i * bpp;
                            *(scan + channel) = value;
                        }
                    } else if (c < 128) {
                        c++;

                        for (unsigned i = count; i < count + c; i++) {
                            unsigned char value;
                            SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, &value, sizeof(value)));

                            unsigned char *scan = (unsigned char *)sail_scan_line(image, row) + i * bpp;
                            *(scan + channel) = value;
                        }
                    }

                    count += c;
                }
            }
        }
    } else {
        for (unsigned channel = 0; channel < psd_state->channels; channel++) {
            for (unsigned row = 0; row < image->height; row++) {
                SAIL_TRY(psd_state->io->strict_read(psd_state->io->stream, psd_state->scan_buffer, psd_state->bytes_per_channel));

                for (unsigned count = 0; count < psd_state->bytes_per_channel; count++) {
                    unsigned char *scan = (unsigned char *)sail_scan_line(image, row) + count * bpp;
                    *(scan + channel) = *(psd_state->scan_buffer + count);
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_psd(void **state) {

    struct psd_state *psd_state = *state;

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
