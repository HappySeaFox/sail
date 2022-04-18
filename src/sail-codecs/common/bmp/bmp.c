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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "bmp.h"
#include "helpers.h"

/*  Compression types.  */
static const uint32_t SAIL_BI_RGB            = 0;
static const uint32_t SAIL_BI_RLE8           = 1;
static const uint32_t SAIL_BI_RLE4           = 2;
static const uint32_t SAIL_BI_BITFIELDS      = 3;
#if 0
static const uint32_t SAIL_BI_JPEG           = 4;
static const uint32_t SAIL_BI_PNG            = 5;
static const uint32_t SAIL_BI_ALPHABITFIELDS = 6;
static const uint32_t SAIL_BI_CMYK           = 11;
static const uint32_t SAIL_BI_CMYKRLE8       = 12;
static const uint32_t SAIL_BI_CMYKRLE4       = 13;
#endif

/* BMP identifiers. */
static const uint16_t SAIL_DDB_IDENTIFIER = 0x02;
static const uint16_t SAIL_DIB_IDENTIFIER = 0x4D42;

/* ICC profile types. */
#if 0
static const char SAIL_PROFILE_LINKED[4]   = { 'L', 'I', 'N', 'K' };
#endif
static const char SAIL_PROFILE_EMBEDDED[4] = { 'M', 'B', 'E', 'D' };

/* Sizes of DIB header structs. */
#define SAIL_BITMAP_DIB_HEADER_V2_SIZE 12
#define SAIL_BITMAP_DIB_HEADER_V3_SIZE 40
#define SAIL_BITMAP_DIB_HEADER_V4_SIZE 108
#define SAIL_BITMAP_DIB_HEADER_V5_SIZE 124

/*
 * Codec-specific state.
 */
struct bmp_state {
    /* These two are external. */
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    int bmp_load_options;

    enum SailPixelFormat source_pixel_format;

    enum SailBmpVersion version;

    struct SailBmpDdbFileHeader ddb_file_header;
    struct SailBmpDdbBitmap v1;

    struct SailBmpDibFileHeader dib_file_header;
    struct SailBmpDibHeaderV2 v2;
    struct SailBmpDibHeaderV3 v3;
    struct SailBmpDibHeaderV4 v4;
    struct SailBmpDibHeaderV5 v5;

    struct sail_iccp *iccp;

    sail_rgb24_t *palette;
    unsigned palette_count;
    unsigned bytes_in_row;
    /* Number of bytes to pad scan lines to 4-byte boundary. */
    unsigned pad_bytes;
    bool flipped;
};

static sail_status_t alloc_bmp_state(struct bmp_state **bmp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct bmp_state), &ptr));
    *bmp_state = ptr;

    if (*bmp_state == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    (*bmp_state)->load_options     = NULL;
    (*bmp_state)->save_options     = NULL;
    (*bmp_state)->bmp_load_options = 0;
    (*bmp_state)->iccp             = NULL;
    (*bmp_state)->palette          = NULL;
    (*bmp_state)->palette_count    = 0;
    (*bmp_state)->bytes_in_row     = 0;
    (*bmp_state)->pad_bytes        = 0;
    (*bmp_state)->flipped          = false;

    return SAIL_OK;
}

static void destroy_bmp_state(struct bmp_state *bmp_state) {

    if (bmp_state == NULL) {
        return;
    }

    sail_destroy_iccp(bmp_state->iccp);

    sail_free(bmp_state->palette);

    sail_free(bmp_state);
}

static sail_status_t read_bmp_headers(struct sail_io *io, struct bmp_state *bmp_state) {

    size_t offset_of_bitmap_header;
    SAIL_TRY(io->tell(io->stream, &offset_of_bitmap_header));

    SAIL_TRY(bmp_private_read_v2(io, &bmp_state->v2));

    /* If the height is negative, the bitmap is top-to-bottom. */
    if (bmp_state->v2.height < 0) {
        bmp_state->v2.height = -bmp_state->v2.height;
        bmp_state->flipped = false;
    } else {
        bmp_state->flipped = true;
    }

    switch (bmp_state->v2.size) {
        case SAIL_BITMAP_DIB_HEADER_V2_SIZE: {
            bmp_state->version = SAIL_BMP_V2;
            break;
        }
        case SAIL_BITMAP_DIB_HEADER_V3_SIZE: {
            bmp_state->version = SAIL_BMP_V3;
            SAIL_TRY(bmp_private_read_v3(io, &bmp_state->v3));
            break;
        }
        case SAIL_BITMAP_DIB_HEADER_V4_SIZE: {
            bmp_state->version = SAIL_BMP_V4;
            SAIL_TRY(bmp_private_read_v3(io, &bmp_state->v3));
            SAIL_TRY(bmp_private_read_v4(io, &bmp_state->v4));
            break;
        }
        case SAIL_BITMAP_DIB_HEADER_V5_SIZE: {
            bmp_state->version = SAIL_BMP_V5;
            SAIL_TRY(bmp_private_read_v3(io, &bmp_state->v3));
            SAIL_TRY(bmp_private_read_v4(io, &bmp_state->v4));
            SAIL_TRY(bmp_private_read_v5(io, &bmp_state->v5));

            if (memcmp(&bmp_state->v4.color_space_type, SAIL_PROFILE_EMBEDDED, sizeof(SAIL_PROFILE_EMBEDDED)) == 0) {
                SAIL_TRY(bmp_private_fetch_iccp(io, (long)(offset_of_bitmap_header + bmp_state->v5.profile_data), bmp_state->v5.profile_size, &bmp_state->iccp));
            }

            break;
        }
        default: {
            SAIL_LOG_ERROR("BMP: Unsupported file header size %u", bmp_state->v2.size);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
        }
    }

    return SAIL_OK;
}

/*
 * Decoding functions.
 */

sail_status_t bmp_private_read_init(struct sail_io *io, const struct sail_load_options *load_options, void **state, int bmp_load_options) {

    /* Allocate a new state. */
    struct bmp_state *bmp_state;
    SAIL_TRY(alloc_bmp_state(&bmp_state));
    *state = bmp_state;

    /* Shallow copy load options. */
    bmp_state->load_options = load_options;

    bmp_state->bmp_load_options = bmp_load_options;

    if (bmp_load_options & SAIL_READ_BMP_FILE_HEADER) {
        /* "BM" or 0x02. */
        uint16_t magic;
        SAIL_TRY(io->strict_read(io->stream, &magic, sizeof(magic)));
        SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

        if (magic == SAIL_DDB_IDENTIFIER) {
            bmp_state->version = SAIL_BMP_V1;

            SAIL_TRY(bmp_private_read_ddb_file_header(io, &bmp_state->ddb_file_header));
            SAIL_TRY(bmp_private_read_v1(io, &bmp_state->v1));
        } else if (magic == SAIL_DIB_IDENTIFIER) {
            SAIL_TRY(bmp_private_read_dib_file_header(io, &bmp_state->dib_file_header));
            SAIL_TRY(read_bmp_headers(io, bmp_state));
        } else {
            SAIL_LOG_ERROR("BMP: 0x%x is not a valid magic number", magic);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
        }
    } else {
        SAIL_TRY(read_bmp_headers(io, bmp_state));
    }

    /* Check BMP restrictions. */
    if (bmp_state->version == SAIL_BMP_V1) {
        if (bmp_state->v1.type != 0) {
            SAIL_LOG_ERROR("BMP: DDB type must always be 0");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        if (bmp_state->v1.planes != 1) {
            SAIL_LOG_ERROR("BMP: DDB planes must always be 1");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        if (bmp_state->v1.pixels != 0) {
            SAIL_LOG_ERROR("BMP: DDB pixels must always be 0");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        if (bmp_state->v1.bit_count != 1 && bmp_state->v1.bit_count != 4 && bmp_state->v1.bit_count != 8) {
            SAIL_LOG_ERROR("BMP: DDB bpp must be 1, 4, or 8");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    } else if (bmp_state->version >= SAIL_BMP_V3) {
        if (bmp_state->v3.compression == SAIL_BI_BITFIELDS && bmp_state->v2.bit_count != 16 && bmp_state->v2.bit_count != 32) {
            SAIL_LOG_ERROR("BMP: BitFields compression is allowed only for 16 or 32 bpp");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        if (bmp_state->v3.compression != SAIL_BI_RGB && bmp_state->v3.compression != SAIL_BI_RLE4 && bmp_state->v3.compression != SAIL_BI_RLE8) {
            SAIL_LOG_ERROR("BMP: Only RGB, RLE4, and RLE8 compressions are supported");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
        }
        if (bmp_state->v3.compression == SAIL_BI_RLE4 && bmp_state->v2.bit_count != 4) {
            SAIL_LOG_ERROR("BMP: RLE4 compression must only be used with 4 bpp");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        if (bmp_state->v3.compression == SAIL_BI_RLE8 && bmp_state->v2.bit_count != 8) {
            SAIL_LOG_ERROR("BMP: RLE8 compression must only be used with 8 bpp");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    SAIL_TRY(bmp_private_bit_count_to_pixel_format(bmp_state->version == SAIL_BMP_V1 ? bmp_state->v1.bit_count : bmp_state->v2.bit_count,
                                                    &bmp_state->source_pixel_format));

    if (bmp_state->version < SAIL_BMP_V3) {
        SAIL_LOG_DEBUG("BMP: Version(%d)", bmp_state->version);
    } else {
        SAIL_LOG_DEBUG("BMP: Version(%d), compression(%u)", bmp_state->version, bmp_state->v3.compression);
    }

    /* Read palette.  */
    if (bmp_state->version == SAIL_BMP_V1) {
        SAIL_TRY(bmp_private_fill_system_palette(bmp_state->v1.bit_count, &bmp_state->palette, &bmp_state->palette_count));
    } else if (bmp_state->v2.bit_count < 16) {
        if (bmp_state->version == SAIL_BMP_V2) {
            bmp_state->palette_count = 1 << bmp_state->v2.bit_count;
        } else {
            bmp_state->palette_count = (bmp_state->v3.colors_used == 0) ? (1U << bmp_state->v2.bit_count) : bmp_state->v3.colors_used;
        }

        if (bmp_state->palette_count == 0) {
            SAIL_LOG_ERROR("BMP: Indexed image has no palette");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
        }

        void *ptr;
        SAIL_TRY(sail_malloc(sizeof(sail_rgba32_t) * bmp_state->palette_count, &ptr));
        bmp_state->palette = ptr;

        if (bmp_state->version == SAIL_BMP_V2) {
            sail_rgb24_t rgb;

            for (unsigned i = 0; i < bmp_state->palette_count; i++) {
                SAIL_TRY(sail_read_pixel3_uint8(io, &rgb));

                bmp_state->palette[i].component1 = rgb.component1;
                bmp_state->palette[i].component2 = rgb.component2;
                bmp_state->palette[i].component3 = rgb.component3;
            }
        } else {
            sail_rgba32_t rgba;

            for (unsigned i = 0; i < bmp_state->palette_count; i++) {
                SAIL_TRY(sail_read_pixel4_uint8(io, &rgba));

                bmp_state->palette[i].component1 = rgba.component1;
                bmp_state->palette[i].component2 = rgba.component2;
                bmp_state->palette[i].component3 = rgba.component3;
            }
        }
    }

    /* Calculate the number of pad bytes to align scan lines to 4-byte boundary. */
    if (bmp_state->version == SAIL_BMP_V1) {
        SAIL_TRY(bmp_private_bytes_in_row(bmp_state->v1.width, bmp_state->v1.bit_count, &bmp_state->bytes_in_row));
        bmp_state->pad_bytes = bmp_state->v1.byte_width - bmp_state->bytes_in_row;
    } else {
        SAIL_TRY(bmp_private_bytes_in_row(bmp_state->v2.width, bmp_state->v2.bit_count, &bmp_state->bytes_in_row));
        bmp_state->pad_bytes = bmp_private_pad_bytes(bmp_state->bytes_in_row);
    }

    return SAIL_OK;
}

sail_status_t bmp_private_read_seek_next_frame(void *state, struct sail_io *io, struct sail_image **image) {

    struct bmp_state *bmp_state = (struct bmp_state *)state;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = bmp_state->source_pixel_format;
    image_local->source_image->orientation = bmp_state->flipped ? SAIL_ORIENTATION_MIRRORED_VERTICALLY : SAIL_ORIENTATION_NORMAL;
    image_local->source_image->compression = (bmp_state->v3.compression == SAIL_BI_RLE4 || bmp_state->v3.compression == SAIL_BI_RLE8)
                                             ? SAIL_COMPRESSION_RLE : SAIL_COMPRESSION_NONE;
    image_local->width = (bmp_state->version == SAIL_BMP_V1) ? bmp_state->v1.width : bmp_state->v2.width;
    image_local->height = (bmp_state->version == SAIL_BMP_V1) ? bmp_state->v1.height : bmp_state->v2.height;

    if (bmp_state->version >= SAIL_BMP_V3 && bmp_state->v3.compression == SAIL_BI_RLE4) {
        /* We expand RLE-encoded 4-bit pixels to 8-bit. TODO: Unpack into 4-bit indexed image. */
        image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        image_local->bytes_per_line = bmp_state->bytes_in_row * 2;
    } else {
        image_local->pixel_format = bmp_state->source_pixel_format;
        image_local->bytes_per_line = bmp_state->bytes_in_row;
    }

    if (bmp_state->palette != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, bmp_state->palette_count, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));

        unsigned char *palette_ptr = image_local->palette->data;

        for (unsigned i = 0; i < bmp_state->palette_count; i++) {
            *palette_ptr++ = bmp_state->palette[i].component3;
            *palette_ptr++ = bmp_state->palette[i].component2;
            *palette_ptr++ = bmp_state->palette[i].component1;
        }
    }

    /* Resolution. */
    if (bmp_state->version >= SAIL_BMP_V3) {
        SAIL_TRY_OR_CLEANUP(
            sail_alloc_resolution_from_data(SAIL_RESOLUTION_UNIT_METER, bmp_state->v3.x_pixels_per_meter, bmp_state->v3.y_pixels_per_meter, &image_local->resolution),
                        /* cleanup */ sail_destroy_image(image_local));
    }

    /* Seek to the bitmap data if we have the file header. */
    if (bmp_state->bmp_load_options & SAIL_READ_BMP_FILE_HEADER) {
        if (bmp_state->version > SAIL_BMP_V1) {
            SAIL_TRY_OR_CLEANUP(io->seek(io->stream, bmp_state->dib_file_header.offset, SEEK_SET),
                                /* cleanup */ sail_destroy_image(image_local));
        }
    }

    *image = image_local;

    return SAIL_OK;
}

sail_status_t bmp_private_read_frame(void *state, struct sail_io *io, struct sail_image *image) {

    struct bmp_state *bmp_state = (struct bmp_state *)state;

    /* RLE-encoded images don't need to skip pad bytes. */
    bool skip_pad_bytes = true;

    for (unsigned i = image->height; i > 0; i--) {
        unsigned char *scan = (unsigned char *)image->pixels + image->bytes_per_line * (bmp_state->flipped ? (i - 1) : (image->height - i));

        for (unsigned pixel_index = 0; pixel_index < image->width;) {
            if (bmp_state->version >= SAIL_BMP_V3 && bmp_state->v3.compression == SAIL_BI_RLE4) {
                skip_pad_bytes = false;

                uint8_t marker;
                SAIL_TRY(io->strict_read(io->stream, &marker, sizeof(marker)));

                if (marker == SAIL_UNENCODED_RUN_MARKER) {
                    uint8_t count_or_marker;
                    SAIL_TRY(io->strict_read(io->stream, &count_or_marker, sizeof(count_or_marker)));

                    if (count_or_marker == SAIL_END_OF_SCAN_LINE_MARKER) {
                        /* Jump to the end of scan line. +1 to avoid reading end-of-scan-line marker twice below. */
                        pixel_index = image->width + 1;
                    } else if (count_or_marker == SAIL_END_OF_RLE_DATA_MARKER) {
                        SAIL_LOG_ERROR("BMP: Unexpected end-of-rle-data marker");
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
                    } else if (count_or_marker == SAIL_DELTA_MARKER) {
                        SAIL_LOG_ERROR("BMP: Delta marker is not supported");
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
                    } else {
                        bool read_byte = true;
                        uint8_t byte = 0;
                        uint8_t index;

                        for (uint8_t k = 0; k < count_or_marker; k++) {
                            if (read_byte) {
                                SAIL_TRY(io->strict_read(io->stream, &byte, sizeof(byte)));
                                index = (byte >> 4) & 0xf;
                                read_byte = false;
                            } else {
                                index = byte & 0xf;
                                read_byte = true;
                            }

                            *scan++ = index;
                        }

                        /* Odd number of bytes is accompanied with an additional byte. */
                        uint8_t number_of_unencoded_bytes = (count_or_marker + 1) / 2;
                        if ((number_of_unencoded_bytes % 2) != 0) {
                            SAIL_TRY(io->seek(io->stream, 1, SEEK_CUR));
                        }

                        pixel_index += count_or_marker;
                    }
                } else {
                    /* Normal RLE: count + value. */
                    bool high_4_bits = true;
                    uint8_t index;

                    uint8_t byte;
                    SAIL_TRY(io->strict_read(io->stream, &byte, sizeof(byte)));

                    for (uint8_t k = 0; k < marker; k++) {
                        if (high_4_bits) {
                            index = (byte >> 4) & 0xf;
                            high_4_bits = false;
                        } else {
                            index = byte & 0xf;
                            high_4_bits = true;
                        }

                        *scan++ = index;
                    }

                    pixel_index += marker;
                }

                /* Read a possible end-of-scan-line marker at the end of line. */
                if (pixel_index == image->width) {
                    SAIL_TRY(bmp_private_skip_end_of_scan_line(io));
                }
            } else if (bmp_state->version >= SAIL_BMP_V3 && bmp_state->v3.compression == SAIL_BI_RLE8) {
                skip_pad_bytes = false;

                uint8_t marker;
                SAIL_TRY(io->strict_read(io->stream, &marker, sizeof(marker)));

                if (marker == SAIL_UNENCODED_RUN_MARKER) {
                    uint8_t count_or_marker;
                    SAIL_TRY(io->strict_read(io->stream, &count_or_marker, sizeof(count_or_marker)));

                    if (count_or_marker == SAIL_END_OF_SCAN_LINE_MARKER) {
                        /* Jump to the end of scan line. +1 to avoid reading end-of-scan-line marker twice below. */
                        pixel_index = image->width + 1;
                    } else if (count_or_marker == SAIL_END_OF_RLE_DATA_MARKER) {
                        SAIL_LOG_ERROR("BMP: Unexpected end-of-rle-data marker");
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
                    } else if (count_or_marker == SAIL_DELTA_MARKER) {
                        SAIL_LOG_ERROR("BMP: Delta marker is not supported");
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
                    } else {
                        for (uint8_t k = 0; k < count_or_marker; k++) {
                            uint8_t index;
                            SAIL_TRY(io->strict_read(io->stream, &index, sizeof(index)));

                            *scan++ = index;
                        }

                        /* Odd number of pixels is accompanied with an additional byte. */
                        if ((count_or_marker % 2) != 0) {
                            SAIL_TRY(io->seek(io->stream, 1, SEEK_CUR));
                        }

                        pixel_index += count_or_marker;
                    }
                } else {
                    /* Normal RLE: count + value. */
                    uint8_t index;
                    SAIL_TRY(io->strict_read(io->stream, &index, sizeof(index)));

                    for (uint8_t k = 0; k < marker; k++) {
                        *scan++ = index;
                    }

                    pixel_index += marker;
                }

                /* Read a possible end-of-scan-line marker at the end of line. */
                if (pixel_index == image->width) {
                    SAIL_TRY(bmp_private_skip_end_of_scan_line(io));
                }
            } else {
                /* Read a whole scan line. */
                SAIL_TRY(io->strict_read(io->stream, scan, bmp_state->bytes_in_row));
                pixel_index += image->width;
            }
        }

        /* Skip pad bytes. */
        if (skip_pad_bytes) {
            SAIL_TRY(io->seek(io->stream, bmp_state->pad_bytes, SEEK_CUR));
        }
    }

    return SAIL_OK;
}

sail_status_t bmp_private_read_finish(void **state, struct sail_io *io) {

    (void)io;

    struct bmp_state *bmp_state = (struct bmp_state *)(*state);

    *state = NULL;

    destroy_bmp_state(bmp_state);

    return SAIL_OK;
}
