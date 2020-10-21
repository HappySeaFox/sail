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

#include <gif_lib.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

static const int InterlacedOffset[] = { 0, 4, 2, 1 };
static const int InterlacedJumps[]  = { 8, 8, 4, 2 };

/*
 * Codec-specific state.
 */
struct gif_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    GifFileType *gif;
    GifByteType *Extension;
    unsigned char *buf;
    int layer;
    unsigned j;
    int transIndex, Lines_h, linesz, disposal, lastDisposal, currentImage, currentPass;
    unsigned Row, Col, Width, Height, lastRow, lastCol, lastWidth, lastHeight;
    unsigned char **Lines, **Last;
    unsigned char back[4]; /* RGBA */
    const ColorMapObject *map;
};

static sail_status_t alloc_gif_state(struct gif_state **gif_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct gif_state), &ptr));
    *gif_state = ptr;

    if (*gif_state == NULL) {
        return SAIL_ERROR_MEMORY_ALLOCATION;
    }

    (*gif_state)->read_options  = NULL;
    (*gif_state)->write_options = NULL;

    (*gif_state)->gif           = NULL;
    (*gif_state)->Extension     = NULL;
    (*gif_state)->buf           = NULL;
    (*gif_state)->transIndex    = -1;
    (*gif_state)->layer         = -1;
    (*gif_state)->disposal      = DISPOSAL_UNSPECIFIED;
    (*gif_state)->lastDisposal  = DISPOSAL_UNSPECIFIED;
    (*gif_state)->currentImage  = -1;
    (*gif_state)->currentPass   = -1;
    (*gif_state)->Row           = 0;
    (*gif_state)->Col           = 0;
    (*gif_state)->Width         = 0;
    (*gif_state)->Height        = 0;
    (*gif_state)->lastRow       = 0;
    (*gif_state)->lastCol       = 0;
    (*gif_state)->lastWidth     = 0;
    (*gif_state)->lastHeight    = 0;
    (*gif_state)->Last          = NULL;
    (*gif_state)->map           = NULL;

    return SAIL_OK;
}

static void destroy_gif_state(struct gif_state *gif_state) {

    if (gif_state == NULL) {
        return;
    }

    sail_destroy_read_options(gif_state->read_options);
    sail_destroy_write_options(gif_state->write_options);

    sail_free(gif_state->buf);

    if (gif_state->Last != NULL) {
        for(int i = 0; i < gif_state->Lines_h; i++) {
            sail_free(gif_state->Last[i]);
        }

        sail_free(gif_state->Last);
    }

    sail_free(gif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v3(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    SAIL_TRY(supported_read_output_pixel_format(read_options->output_pixel_format));

    /* Allocate a new state. */
    struct gif_state *gif_state;
    SAIL_TRY(alloc_gif_state(&gif_state));
    *state = gif_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &gif_state->read_options));

    /* Initialize GIF. */
    int error_code;
    gif_state->gif = DGifOpen(io, my_read_proc, &error_code);

    if (gif_state->gif == NULL) {
        SAIL_LOG_ERROR("GIF: Failed to initialize. GIFLIB error code: %d", error_code);
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    /* Initialize internal structs. */
    gif_state->linesz = gif_state->gif->SWidth * sizeof(GifPixelType);

    if (gif_state->gif->SColorMap != NULL) {
        if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
            gif_state->back[0] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Red;
            gif_state->back[1] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Green;
            gif_state->back[2] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Blue;
        } else if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
            gif_state->back[0] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Blue;
            gif_state->back[1] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Green;
            gif_state->back[2] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Red;
        }

        gif_state->back[3] = 255;
    } else {
        memset(&gif_state->back, 0, sizeof(gif_state->back));
    }

    void *ptr;

    SAIL_TRY(sail_malloc(gif_state->linesz, &ptr));
    gif_state->buf = ptr;

    gif_state->Lines_h = gif_state->gif->SHeight;

    SAIL_TRY(sail_malloc(gif_state->Lines_h * sizeof(unsigned char *), &ptr));
    gif_state->Last = ptr;

    for (int i = 0; i < gif_state->Lines_h; i++) {
        SAIL_TRY(sail_calloc(gif_state->gif->SWidth, 4, &ptr)); /* 4 = RGBA */
        gif_state->Last[i] = ptr;
#if 0
        for (int k = 0; k < gif_state->gif->SWidth; k++) {
            memcpy(gif_state->Last[i] + k*4, &gif_state->back, 4); /* 4 = RGBA */
        }
#endif
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v3(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE_PTR(image);

    struct gif_state *gif_state = (struct gif_state *)state;

    SAIL_TRY(sail_alloc_image(image));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&(*image)->source_image),
                        /* cleanup */ sail_destroy_image(*image));

    (*image)->source_image->compression = SAIL_COMPRESSION_LZW;
    (*image)->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    gif_state->currentImage++;

    gif_state->lastDisposal = gif_state->disposal;
    gif_state->disposal     = DISPOSAL_UNSPECIFIED;
    gif_state->transIndex   = -1;

    gif_state->lastRow    = gif_state->Row;
    gif_state->lastCol    = gif_state->Col;
    gif_state->lastWidth  = gif_state->Width;
    gif_state->lastHeight = gif_state->Height;

    /* Loop through records. */
    while (true) {
        GifRecordType record;

        if (DGifGetRecordType(gif_state->gif, &record) == GIF_ERROR) {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            sail_destroy_image(*image);
            return SAIL_ERROR_UNDERLYING_CODEC;
        }

        switch (record) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(gif_state->gif) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    sail_destroy_image(*image);
                    return SAIL_ERROR_UNDERLYING_CODEC;
                }

                (*image)->width = gif_state->gif->SWidth;
                (*image)->height = gif_state->gif->SHeight;

                gif_state->Row    = gif_state->gif->Image.Top;
                gif_state->Col    = gif_state->gif->Image.Left;
                gif_state->Width  = gif_state->gif->Image.Width;
                gif_state->Height = gif_state->gif->Image.Height;

                if (gif_state->gif->Image.Left + gif_state->gif->Image.Width > gif_state->gif->SWidth ||
                        gif_state->gif->Image.Top + gif_state->gif->Image.Height > gif_state->gif->SHeight) {
                    sail_destroy_image(*image);
                    return SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS;
                }
                break;
            }

            case EXTENSION_RECORD_TYPE: {
                int ExtCode;

                if (DGifGetExtension(gif_state->gif, &ExtCode, &gif_state->Extension) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    sail_destroy_image(*image);
                    return SAIL_ERROR_UNDERLYING_CODEC;
                }

                switch (ExtCode) {
                    case GRAPHICS_EXT_FUNC_CODE: {
                        /* Disposal method. */
                        gif_state->disposal = (gif_state->Extension[1] >> 2) & 7;

                        /* Delay in 1/100 of seconds. */
                        unsigned delay = *(uint16_t *)(gif_state->Extension + 2);
                        /*
                         * 0 means as fast as possible. However, this makes the frame
                         * almost invisible on modern CPUs. Let's make a small delay of 100 ms
                         * in this case.
                         */
                        (*image)->delay = (delay == 0) ? 100 : delay * 10;

                        /* Transparent index. */
                        if (gif_state->Extension[1] & 1) {
                            gif_state->transIndex = gif_state->Extension[4];
                        }
                        break;
                    }

                    case COMMENT_EXT_FUNC_CODE: {
                        if (gif_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(fetch_comment(gif_state->Extension, &(*image)->meta_data_node),
                                                /* cleanup*/ sail_destroy_image(*image));
                        }
                        break;
                    }

                    case APPLICATION_EXT_FUNC_CODE: {
                        if (gif_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(fetch_application(gif_state->Extension, &(*image)->meta_data_node),
                                                /* cleanup */ sail_destroy_image(*image));
                        }
                        break;
                    }
                }

                /* We don't support other extension types, so just skip them. */
                while (gif_state->Extension != NULL) {
                    if (DGifGetExtensionNext(gif_state->gif, &gif_state->Extension) == GIF_ERROR) {
                        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                        sail_destroy_image(*image);
                        return SAIL_ERROR_UNDERLYING_CODEC;
                    }
                }

                break;
            }

            case TERMINATE_RECORD_TYPE: {
                sail_destroy_image(*image);
                return SAIL_ERROR_NO_MORE_FRAMES;
            }

            default: {
                break;
            }
        }

        if (record == IMAGE_DESC_RECORD_TYPE) {
            if (gif_state->currentImage > 0) {
                (*image)->animated = true;
            }

            gif_state->map = (gif_state->gif->Image.ColorMap != NULL) ? gif_state->gif->Image.ColorMap : gif_state->gif->SColorMap;

            if (gif_state->map == NULL) {
                sail_destroy_image(*image);
                return SAIL_ERROR_MISSING_PALETTE;
            }

            if (gif_state->gif->Image.Interlace) {
                (*image)->source_image->properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
                (*image)->interlaced_passes = 4;
            }

            if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
                (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
            } else if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
                (*image)->pixel_format = SAIL_PIXEL_FORMAT_BPP32_BGRA;
            }
            SAIL_TRY_OR_CLEANUP(sail_bytes_per_line((*image)->width, (*image)->pixel_format, &(*image)->bytes_per_line),
                                /* cleanup */ sail_destroy_image(*image));

            gif_state->layer = -1;
            gif_state->currentPass = -1;

            break;
        }
    }

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string((*image)->source_image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("GIF: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(gif_state->read_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("GIF: Output pixel format is %s", pixel_format_str);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct gif_state *gif_state = (struct gif_state *)state;

    gif_state->layer++;
    gif_state->currentPass++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v3(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct gif_state *gif_state = (struct gif_state *)state;

    /* Apply disposal method on the previous frame. */
    for (unsigned cc = gif_state->lastRow; cc < gif_state->lastRow+gif_state->lastHeight; cc++) {
        unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

        if(gif_state->currentImage > 0 && gif_state->currentPass == 0)
        {
            if(gif_state->lastDisposal == DISPOSE_BACKGROUND)
            {
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
                memset(scan + gif_state->lastCol*4, 0, gif_state->lastWidth*4); /* 4 = RGBA */
                memcpy(gif_state->Last[cc], scan, image->width * 4);
            }
            else
            {
                memcpy(scan, gif_state->Last[cc], image->width * 4);
            }
        }
    }

    /* Read lines. */
    for (unsigned cc = 0; cc < image->height; cc++) {
        unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

        if(cc < gif_state->Row || cc >= gif_state->Row + gif_state->Height)
        {
            if (gif_state->currentPass == 0) {
                memcpy(scan, gif_state->Last[cc], image->width * 4);
            }

            continue;
        }

        /* In interlaced mode we skip some lines. */
        bool do_read = false;

        if(gif_state->gif->Image.Interlace)
        {
            if(cc == gif_state->Row)
                gif_state->j = InterlacedOffset[gif_state->layer] + gif_state->Row;

            if(cc == gif_state->j)
            {
                do_read = true;
                gif_state->j += InterlacedJumps[gif_state->layer];
            }
        }
        else // !s32erlaced
        {
            do_read = true;
        }

        if (do_read) {
            if(DGifGetLine(gif_state->gif, gif_state->buf, gif_state->Width) == GIF_ERROR)
            {
                SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                return SAIL_ERROR_UNDERLYING_CODEC;
            }

            memcpy(scan, gif_state->Last[cc], image->width * 4);

            for(unsigned i = 0; i < gif_state->Width; i++)
            {
                if(gif_state->buf[i] == gif_state->transIndex)
                {
                    continue;
                }

                const int index = gif_state->Col + i;
                unsigned char *pixel = scan + index*4;

                if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA) {
                    pixel[0] = gif_state->map->Colors[gif_state->buf[i]].Red;
                    pixel[1] = gif_state->map->Colors[gif_state->buf[i]].Green;
                    pixel[2] = gif_state->map->Colors[gif_state->buf[i]].Blue;
                } else if (gif_state->read_options->output_pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA) {
                    pixel[0] = gif_state->map->Colors[gif_state->buf[i]].Blue;
                    pixel[1] = gif_state->map->Colors[gif_state->buf[i]].Green;
                    pixel[2] = gif_state->map->Colors[gif_state->buf[i]].Red;
                }

                pixel[3] = 255;
            } // for
        }

        if(gif_state->currentPass == image->interlaced_passes-1)
        {
            memcpy(gif_state->Last[cc], scan, image->width * 4);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct gif_state *gif_state = (struct gif_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (gif_state->gif != NULL) {
        DGifCloseFile(gif_state->gif, /* ErrorCode */ NULL);
    }

    destroy_gif_state(gif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v3(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_CHECK_IO(io);
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    struct gif_state *gif_state;
    SAIL_TRY(alloc_gif_state(&gif_state));

    *state = gif_state;

    /* Deep copy write options. */
    SAIL_TRY(sail_copy_write_options(write_options, &gif_state->write_options));

#if 0
    /* Sanity check. */
    SAIL_TRY(supported_write_output_pixel_format(gif_state->write_options->output_pixel_format));
    SAIL_TRY(sail_compression_to_tiff_compression(gif_state->write_options->compression, &gif_state->write_compression));

    /* Initialize GIF.
     */
    /* TODO */
#endif

    if (gif_state->gif == NULL) {
        return SAIL_ERROR_UNDERLYING_CODEC;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct gif_state *gif_state = (struct gif_state *)state;

#if 0
    /* Write meta data. */
    if (gif_state->write_options->io_options & SAIL_IO_OPTION_META_DATA && image->meta_data_node != NULL) {
        /* TODO */
        SAIL_LOG_DEBUG("GIF: Writing meta data");
    }
#endif

    const char *pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(image->pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("GIF: Input pixel format is %s", pixel_format_str);
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(gif_state->write_options->output_pixel_format, &pixel_format_str));
    SAIL_LOG_DEBUG("GIF: Output pixel format is %s", pixel_format_str);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v3(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);
    SAIL_CHECK_IMAGE(image);

    struct gif_state *gif_state = (struct gif_state *)state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v3(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_CHECK_IO(io);

    struct gif_state *gif_state = (struct gif_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

#if 0
    /* Destroy internal TIFF objects. */
    if (gif_state->tiff != NULL) {
        /* TODO */
    }
#endif

    destroy_gif_state(gif_state);

    return SAIL_OK;
}
