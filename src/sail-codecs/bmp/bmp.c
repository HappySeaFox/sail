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

#include "helpers.h"
#include "io.h"

/*  Compression types.  */
static const int SAIL_BI_RGB            = 0;
static const int SAIL_BI_RLE8           = 1;
static const int SAIL_BI_RLE4           = 2;
static const int SAIL_BI_BITFIELDS      = 3;
static const int SAIL_BI_JPEG           = 4;
static const int SAIL_BI_PNG            = 5;
static const int SAIL_BI_ALPHABITFIELDS = 6;
static const int SAIL_BI_CMYK           = 11;
static const int SAIL_BI_CMYKRLE8       = 12;
static const int SAIL_BI_CMYKRLE4       = 13;

/* BMP identifiers. */
static const uint16_t SAIL_DDB_IDENTIFIER = 0x02;
static const uint16_t SAIL_DIB_IDENTIFIER = 0x4D42;

/* Sizes of DIB header structs. */
#define SAIL_BITMAP_DIB_HEADER_V2_SIZE 12
#define SAIL_BITMAP_DIB_HEADER_V3_SIZE 40
#define SAIL_BITMAP_DIB_HEADER_V4_SIZE 108
#define SAIL_BITMAP_DIB_HEADER_V5_SIZE 124

/*
 * V1: Device-Dependent Bitmap (DDB).
 */

/* File header. */
struct SailBmpDdbFileHeader
{
    uint16_t type; /* Always 2. Top bit set if discardable. */
};

/* Bitmap16. */
struct SailBmpDdbBitmap
{
    uint16_t type; /* Always 0. */
    uint16_t width;
    uint16_t height;
    uint16_t byte_width;
    uint8_t  planes; /* Always 1. */
    uint8_t  bit_count;
    uint32_t pixels; /* Always 0. */
};

/*
 * V2+: File header + DIB headers.
 */

/* File header. */
struct SailBmpDibFileHeader
{
    uint16_t type; /* "BM" */
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

/* DIB headers. */
struct SailBmpDibHeaderV2
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bit_count;
};

struct SailBmpDibHeaderV3
{
    uint32_t compression;
    uint32_t bitmap_size;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};

SAIL_HIDDEN sail_status_t bmp_private_read_dib_file_header(struct sail_io *io, struct SailBmpDibFileHeader *fh) {

    SAIL_TRY(io->strict_read(io->stream, &fh->type,      sizeof(fh->type)));
    SAIL_TRY(io->strict_read(io->stream, &fh->size,      sizeof(fh->size)));
    SAIL_TRY(io->strict_read(io->stream, &fh->reserved1, sizeof(fh->reserved1)));
    SAIL_TRY(io->strict_read(io->stream, &fh->reserved2, sizeof(fh->reserved2)));
    SAIL_TRY(io->strict_read(io->stream, &fh->offset,    sizeof(fh->offset)));

    return SAIL_OK;
}

SAIL_HIDDEN sail_status_t bmp_private_read_v2(struct sail_io *io, struct SailBmpDibHeaderV2 *v2) {

    SAIL_TRY(io->strict_read(io->stream, &v2->size,      sizeof(v2->size)));
    SAIL_TRY(io->strict_read(io->stream, &v2->width,     sizeof(v2->width)));
    SAIL_TRY(io->strict_read(io->stream, &v2->height,    sizeof(v2->height)));
    SAIL_TRY(io->strict_read(io->stream, &v2->planes,    sizeof(v2->planes)));
    SAIL_TRY(io->strict_read(io->stream, &v2->bit_count, sizeof(v2->bit_count)));

    return SAIL_OK;
}

SAIL_HIDDEN sail_status_t bmp_private_read_v3(struct sail_io *io, struct SailBmpDibHeaderV3 *v3) {

    SAIL_TRY(io->strict_read(io->stream, &v3->compression,        sizeof(v3->compression)));
    SAIL_TRY(io->strict_read(io->stream, &v3->bitmap_size,        sizeof(v3->bitmap_size)));
    SAIL_TRY(io->strict_read(io->stream, &v3->x_pixels_per_meter, sizeof(v3->x_pixels_per_meter)));
    SAIL_TRY(io->strict_read(io->stream, &v3->y_pixels_per_meter, sizeof(v3->y_pixels_per_meter)));
    SAIL_TRY(io->strict_read(io->stream, &v3->colors_used,        sizeof(v3->colors_used)));
    SAIL_TRY(io->strict_read(io->stream, &v3->colors_important,   sizeof(v3->colors_important)));

    return SAIL_OK;
}

struct SailBmpDibHeaderV4
{
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;
	uint32_t color_space_type;
	int32_t  red_x;
	int32_t  red_y;
	int32_t  red_z;
	int32_t  green_x;
	int32_t  green_y;
	int32_t  green_z;
	int32_t  blue_x;
	int32_t  blue_y;
	int32_t  blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
};

struct SailBmpDibHeaderV5
{
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t reserved;
};

struct SailBmpRgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum SailBmpVersion
{
    SAIL_BMP_V1,
    SAIL_BMP_V2,
    SAIL_BMP_V3,
    SAIL_BMP_V4,
    SAIL_BMP_V5,
};

/*
 * Codec-specific state.
 */
struct bmp_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    enum SailBmpVersion version;

    struct SailBmpDdbFileHeader ddb_file_header;
    struct SailBmpDdbBitmap v1;

    struct SailBmpDibFileHeader dib_file_header;
    struct SailBmpDibHeaderV2 v2;
    struct SailBmpDibHeaderV3 v3;
    struct SailBmpDibHeaderV4 v4;
    struct SailBmpDibHeaderV5 v5;

    struct SailBmpRgb *palette;
    int pal_entr;
    int filler;

    bool frame_read;
    bool frame_written;
};

static sail_status_t alloc_bmp_state(struct bmp_state **bmp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct bmp_state), &ptr));
    *bmp_state = ptr;

    if (*bmp_state == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    (*bmp_state)->read_options  = NULL;
    (*bmp_state)->write_options = NULL;

#if 0
    (*bmp_state)->ddb_file_header = NULL;
    (*bmp_state)->v1              = NULL;
    (*bmp_state)->dib_file_header = NULL;
    (*bmp_state)->v2              = NULL;
    (*bmp_state)->v3              = NULL;
    (*bmp_state)->v4              = NULL;
    (*bmp_state)->v5              = NULL;
#endif

    (*bmp_state)->palette         = NULL;
    (*bmp_state)->pal_entr        = 0;
    (*bmp_state)->filler          = 0;

    (*bmp_state)->frame_read    = false;
    (*bmp_state)->frame_written = false;

    return SAIL_OK;
}

static void destroy_bmp_state(struct bmp_state *bmp_state) {

    if (bmp_state == NULL) {
        return;
    }

    sail_destroy_read_options(bmp_state->read_options);
    sail_destroy_write_options(bmp_state->write_options);

#if 0
    sail_free(bmp_state->ddb_file_header);
    sail_free(bmp_state->v1);
    sail_free(bmp_state->dib_file_header);
    sail_free(bmp_state->v2);
    sail_free(bmp_state->v3);
    sail_free(bmp_state->v4);
    sail_free(bmp_state->v5);
#endif

    sail_free(bmp_state->palette);

    sail_free(bmp_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v4_bmp(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    SAIL_TRY(bmp_private_supported_read_output_pixel_format(read_options->output_pixel_format));

    /* Allocate a new state. */
    struct bmp_state *bmp_state;
    SAIL_TRY(alloc_bmp_state(&bmp_state));
    *state = bmp_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &bmp_state->read_options));

    /* "BM" or 0x02. */
    uint16_t magic;
    SAIL_TRY(io->strict_read(io->stream, &magic, sizeof(magic)));
    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    if (magic == SAIL_DDB_IDENTIFIER) {
        bmp_state->version = SAIL_BMP_V1;

        SAIL_TRY(io->strict_read(io->stream, &bmp_state->ddb_file_header, sizeof(bmp_state->ddb_file_header)));
        SAIL_TRY(io->strict_read(io->stream, &bmp_state->v1, sizeof(bmp_state->v1)));

        // FIXME
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    } else if (magic == SAIL_DIB_IDENTIFIER) {
        SAIL_TRY(bmp_private_read_dib_file_header(io, &bmp_state->dib_file_header));
        SAIL_TRY(bmp_private_read_v2(io, &bmp_state->v2));

        switch (bmp_state->v2.size) {
            case SAIL_BITMAP_DIB_HEADER_V2_SIZE: {
                break;
            }
            case SAIL_BITMAP_DIB_HEADER_V3_SIZE: {
                SAIL_TRY(bmp_private_read_v3(io, &bmp_state->v3));
                break;
            }
            default: {
                SAIL_LOG_ERROR("BMP: Unsupported file header size %u", bmp_state->dib_file_header.size);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
            }
        }
    } else {
        SAIL_LOG_ERROR("0x%x is not a valid BMP magic number", magic);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }

SAIL_LOG_DEBUG("BMP: %ux%u", bmp_state->v2.width, bmp_state->v2.height);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v4_bmp(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE_PTR(image);

#if 0
    struct bmp_state *bmp_state = (struct bmp_state *)state;

    if (bmp_state->frame_read) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    bmp_state->frame_read = true;

    SAIL_TRY(sail_alloc_image(image));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&(*image)->source_image),
                        /* cleanup */ sail_destroy_image(*image));

    (*image)->source_image->compression = SAIL_COMPRESSION_NONE;
    (*image)->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    bmp_state->current_image++;

    bmp_state->prev_disposal      = bmp_state->disposal;
    bmp_state->disposal           = DISPOSAL_UNSPECIFIED;
    bmp_state->transparency_index = -1;

    bmp_state->prev_row    = bmp_state->row;
    bmp_state->prev_column = bmp_state->column;
    bmp_state->prev_width  = bmp_state->width;
    bmp_state->prev_height = bmp_state->height;

    /* Loop through records. */
    while (true) {
        GifRecordType record;

        if (DGifGetRecordType(bmp_state->bmp, &record) == GIF_ERROR) {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(bmp_state->bmp->Error));
            sail_destroy_image(*image);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        switch (record) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(bmp_state->bmp) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(bmp_state->bmp->Error));
                    sail_destroy_image(*image);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                (*image)->width = bmp_state->bmp->SWidth;
                (*image)->height = bmp_state->bmp->SHeight;

                bmp_state->row    = bmp_state->bmp->Image.Top;
                bmp_state->column = bmp_state->bmp->Image.Left;
                bmp_state->width  = bmp_state->bmp->Image.Width;
                bmp_state->height = bmp_state->bmp->Image.Height;

                if (bmp_state->column + bmp_state->width > (unsigned)bmp_state->bmp->SWidth ||
                        bmp_state->row + bmp_state->height > (unsigned)bmp_state->bmp->SHeight) {
                    sail_destroy_image(*image);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS);
                }
                break;
            }

            case EXTENSION_RECORD_TYPE: {
                int ext_code;
                GifByteType *extension;

                if (DGifGetExtension(bmp_state->bmp, &ext_code, &extension) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(bmp_state->bmp->Error));
                    sail_destroy_image(*image);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                if (extension == NULL) {
                    break;
                }

                switch (ext_code) {
                    case GRAPHICS_EXT_FUNC_CODE: {
                        /* Disposal method. */
                        bmp_state->disposal = (extension[1] >> 2) & 7;

                        /* Delay in 1/100 of seconds. */
                        unsigned delay = *(uint16_t *)(extension + 2);
                        /*
                         * 0 means as fast as possible. However, this makes the frame
                         * almost invisible on modern CPUs. Let's make a small delay of 100 ms
                         * in this case.
                         */
                        (*image)->delay = (delay == 0) ? 100 : delay * 10;

                        /* Transparent index. */
                        if (extension[1] & 1) {
                            bmp_state->transparency_index = extension[4];
                        }
                        break;
                    }

                    case COMMENT_EXT_FUNC_CODE: {
                        if (bmp_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(bmp_private_fetch_comment(extension, &(*image)->meta_data_node),
                                                /* cleanup*/ sail_destroy_image(*image));
                        }
                        break;
                    }

                    case APPLICATION_EXT_FUNC_CODE: {
                        if (bmp_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(bmp_private_fetch_application(extension, &(*image)->meta_data_node),
                                                /* cleanup */ sail_destroy_image(*image));
                        }
                        break;
                    }
                }

                /* We don't support other extension types, so just skip them. */
                while (extension != NULL) {
                    if (DGifGetExtensionNext(bmp_state->bmp, &extension) == GIF_ERROR) {
                        SAIL_LOG_ERROR("GIF: %s", GifErrorString(bmp_state->bmp->Error));
                        sail_destroy_image(*image);
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                    }
                }

                break;
            }

            case TERMINATE_RECORD_TYPE: {
                sail_destroy_image(*image);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
            }

            default: {
                break;
            }
        }

        if (record == IMAGE_DESC_RECORD_TYPE) {
            if (bmp_state->current_image > 0) {
                (*image)->animated = true;
            }

            bmp_state->map = (bmp_state->bmp->Image.ColorMap != NULL) ? bmp_state->bmp->Image.ColorMap : bmp_state->bmp->SColorMap;

            if (bmp_state->map == NULL) {
                sail_destroy_image(*image);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
            }

            if (bmp_state->bmp->Image.Interlace) {
                (*image)->source_image->properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
                (*image)->interlaced_passes = 4;
            }

            if (bmp_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
                (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
            } else if (bmp_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
                (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_BGRA;
            }
            SAIL_TRY_OR_CLEANUP(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line),
                                /* cleanup */ sail_destroy_image(*image));

            bmp_state->layer = -1;
            bmp_state->current_pass = -1;

            break;
        }
    }

    if (bmp_state->current_image == 0) {
        const char *pixel_format_str = NULL;
        SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string((*image)->source_image->pixel_format, &pixel_format_str));
        SAIL_LOG_DEBUG("GIF: Input pixel format is %s", pixel_format_str);
        SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(bmp_state->read_options->output_pixel_format, &pixel_format_str));
        SAIL_LOG_DEBUG("GIF: Output pixel format is %s", pixel_format_str);
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v4_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

#if 0
    struct bmp_state *bmp_state = (struct bmp_state *)state;

    bmp_state->layer++;
    bmp_state->current_pass++;
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v4_bmp(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

#if 0
    struct bmp_state *bmp_state = (struct bmp_state *)state;

    /* Apply disposal method on the previous frame. */
    if (bmp_state->current_image > 0 && bmp_state->current_pass == 0) {
       for (unsigned cc = bmp_state->prev_row; cc < bmp_state->prev_row+bmp_state->prev_height; cc++) {
            unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

            if (bmp_state->prev_disposal == DISPOSE_BACKGROUND) {
                /*
                 * Spec:
                 *     2 - Restore to background color. The area used by the
                 *         graphic must be restored to the background color.
                 *
                 * The meaning of the background color is not quite clear here. My idea was that
                 * it's the color specified by the background color index in the global color map.
                 * However, other decoders like XnView treat "background" as a transparent color here.
                 * Let's do the same.
                 */
                memset(bmp_state->first_frame[cc] + bmp_state->prev_column*4, 0, bmp_state->prev_width*4); /* 4 = RGBA */
            }

            memcpy(scan, bmp_state->first_frame[cc], image->width * 4);
        }
    }

    /* Read lines. */
    for (unsigned cc = 0; cc < image->height; cc++) {
        unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

        if (cc < bmp_state->row || cc >= bmp_state->row + bmp_state->height) {
            if (bmp_state->current_pass == 0) {
                memcpy(scan, bmp_state->first_frame[cc], image->width * 4);
            }

            continue;
        }

        /* In interlaced mode we skip some lines. */
        bool do_read = false;

        if (bmp_state->bmp->Image.Interlace) {
            if (cc == bmp_state->row) {
                bmp_state->next_interlaced_row = InterlacedOffset[bmp_state->layer] + bmp_state->row;
            }

            if (cc == bmp_state->next_interlaced_row) {
                do_read = true;
                bmp_state->next_interlaced_row += InterlacedJumps[bmp_state->layer];
            }
        }
        else { // !s32erlaced
            do_read = true;
        }

        if (do_read) {
            if (DGifGetLine(bmp_state->bmp, bmp_state->buf, bmp_state->width) == GIF_ERROR) {
                SAIL_LOG_ERROR("GIF: %s", GifErrorString(bmp_state->bmp->Error));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            memcpy(scan, bmp_state->first_frame[cc], image->width * 4);

            for (unsigned i = 0; i < bmp_state->width; i++) {
                if (bmp_state->buf[i] == bmp_state->transparency_index) {
                    continue;
                }

                unsigned char *pixel = scan + (bmp_state->column + i)*4;

                if (bmp_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
                    pixel[0] = bmp_state->map->Colors[bmp_state->buf[i]].Red;
                    pixel[1] = bmp_state->map->Colors[bmp_state->buf[i]].Green;
                    pixel[2] = bmp_state->map->Colors[bmp_state->buf[i]].Blue;
                } else if (bmp_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
                    pixel[0] = bmp_state->map->Colors[bmp_state->buf[i]].Blue;
                    pixel[1] = bmp_state->map->Colors[bmp_state->buf[i]].Green;
                    pixel[2] = bmp_state->map->Colors[bmp_state->buf[i]].Red;
                }

                pixel[3] = 255;
            } // for
        }

        if (bmp_state->current_pass == image->interlaced_passes-1) {
            memcpy(bmp_state->first_frame[cc], scan, image->width * 4);
        }
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v4_bmp(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct bmp_state *bmp_state = (struct bmp_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_bmp_state(bmp_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v4_bmp(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v4_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v4_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v4_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v4_bmp(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
