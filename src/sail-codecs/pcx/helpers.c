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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

/* 256-color palette signature. */
static const unsigned SAIL_PCX_PALETTE_SIGNATURE = 0x0C;

sail_status_t pcx_private_read_header(struct sail_io *io, struct SailPcxHeader *header) {

    SAIL_TRY(io->strict_read(io->stream, &header->id,             sizeof(header->id)));
    SAIL_TRY(io->strict_read(io->stream, &header->version,        sizeof(header->version)));
    SAIL_TRY(io->strict_read(io->stream, &header->encoding,       sizeof(header->encoding)));
    SAIL_TRY(io->strict_read(io->stream, &header->bits_per_plane, sizeof(header->bits_per_plane)));
    SAIL_TRY(io->strict_read(io->stream, &header->xmin,           sizeof(header->xmin)));
    SAIL_TRY(io->strict_read(io->stream, &header->ymin,           sizeof(header->ymin)));
    SAIL_TRY(io->strict_read(io->stream, &header->xmax,           sizeof(header->xmax)));
    SAIL_TRY(io->strict_read(io->stream, &header->ymax,           sizeof(header->ymax)));
    SAIL_TRY(io->strict_read(io->stream, &header->hdpi,           sizeof(header->hdpi)));
    SAIL_TRY(io->strict_read(io->stream, &header->vdpi,           sizeof(header->vdpi)));
    SAIL_TRY(io->strict_read(io->stream, header->palette,         sizeof(header->palette)));
    SAIL_TRY(io->strict_read(io->stream, &header->reserved,       sizeof(header->reserved)));
    SAIL_TRY(io->strict_read(io->stream, &header->planes,         sizeof(header->planes)));
    SAIL_TRY(io->strict_read(io->stream, &header->bytes_per_line, sizeof(header->bytes_per_line)));
    SAIL_TRY(io->strict_read(io->stream, &header->palette_info,   sizeof(header->palette_info)));
    SAIL_TRY(io->strict_read(io->stream, &header->hscreen_size,   sizeof(header->hscreen_size)));
    SAIL_TRY(io->strict_read(io->stream, &header->vscreen_size,   sizeof(header->vscreen_size)));
    SAIL_TRY(io->strict_read(io->stream, header->filler,          sizeof(header->filler)));

    return SAIL_OK;
}

sail_status_t pcx_private_sail_pixel_format(unsigned bits_per_plane, unsigned planes, enum SailPcxPaletteInfo palette_info, enum SailPixelFormat *result) {

    switch (planes) {
        case 1: {
            switch (bits_per_plane) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED; return SAIL_OK;
                case 4: *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
                case 8: *result = (palette_info == SAIL_PCX_PALETTE_COLOR) ? SAIL_PIXEL_FORMAT_BPP8_INDEXED : SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE; return SAIL_OK;
            }
            break;
        }
        case 3: {
            switch (bits_per_plane) {
                case 8: *result = SAIL_PIXEL_FORMAT_BPP24_RGB; return SAIL_OK;
            }
            break;
        }
        case 4: {
            switch (bits_per_plane) {
                case 1: *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
                case 8: *result = SAIL_PIXEL_FORMAT_BPP32_RGBA;   return SAIL_OK;
            }
            break;
        }
    }

    SAIL_LOG_ERROR("PCX: Unsuppored combination of bits per plane(%u) and planes(%u)", bits_per_plane, planes);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

sail_status_t pcx_private_build_palette(enum SailPixelFormat pixel_format, struct sail_io *io, uint8_t palette16[], struct sail_palette **palette) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED: {
            SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, palette));

            unsigned char *palette_data = (*palette)->data;

            *palette_data++ = 0;
            *palette_data++ = 0;
            *palette_data++ = 0;
            *palette_data++ = 255;
            *palette_data++ = 255;
            *palette_data++ = 255;

            break;
        }
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED: {
            const int palette_colors = 16; /* 256 RGB entries. */
            const int palette_size = 16 * 3; /* 256 RGB entries. */

            SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, palette_colors, palette));
            memcpy((*palette)->data, palette16, palette_size);
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED: {
            const int palette_colors = 256; /* 256 RGB entries. */
            const int palette_size = 256 * 3; /* 256 RGB entries. */

            /* Seek to offset. */
            size_t saved_offset;
            SAIL_TRY(io->tell(io->stream, &saved_offset));
            SAIL_TRY(io->seek(io->stream, (long)-(palette_size + 1), SEEK_END));

            uint8_t signature;
            SAIL_TRY(io->strict_read(io->stream, &signature, sizeof(signature)));

            if (signature != SAIL_PCX_PALETTE_SIGNATURE) {
                SAIL_LOG_ERROR("PCX: Palette has invalid signature %u, must be %u", signature, SAIL_PCX_PALETTE_SIGNATURE);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }

            struct sail_palette *palette_local;
            SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, palette_colors, &palette_local));

            SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, palette_local->data, palette_size),
                                /* cleanup */ sail_destroy_palette(palette_local));

            SAIL_TRY_OR_CLEANUP(io->seek(io->stream, (long)saved_offset, SEEK_SET),
                                /* cleanup */ sail_destroy_palette(palette_local));

            *palette = palette_local;

            break;
        }
        default: {
            break;
        }
    }

    return SAIL_OK;
}

sail_status_t pcx_private_read_uncompressed(struct sail_io *io, unsigned bytes_per_plane_to_read, unsigned planes, unsigned char *buffer, struct sail_image *image) {

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *target_scan = (unsigned char *)image->pixels + image->bytes_per_line * row;

        /* Read plane by plane and then merge them into the image pixels. */
        for (unsigned plane = 0; plane < planes; plane++) {
            SAIL_TRY(io->strict_read(io->stream, buffer, bytes_per_plane_to_read));

            for (unsigned column = 0; column < bytes_per_plane_to_read; column++) {
                *(target_scan + column * planes + plane) = *(buffer + column);
            }
        }
    }

    return SAIL_OK;
}
