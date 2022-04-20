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
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    GifFileType *gif;
    const ColorMapObject *map;
    unsigned char *buf;
    int transparency_index;
    int first_frame_height;
    int disposal;
    int prev_disposal;
    int current_image;
    unsigned row;
    unsigned column;
    unsigned width;
    unsigned height;
    unsigned prev_row;
    unsigned prev_column;
    unsigned prev_width;
    unsigned prev_height;
    unsigned char **first_frame;
    unsigned char background[4]; /* RGBA */
};

static sail_status_t alloc_gif_state(struct gif_state **gif_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct gif_state), &ptr));
    *gif_state = ptr;

    (*gif_state)->load_options = NULL;
    (*gif_state)->save_options = NULL;

    (*gif_state)->gif                = NULL;
    (*gif_state)->map                = NULL;
    (*gif_state)->buf                = NULL;
    (*gif_state)->transparency_index = -1;
    (*gif_state)->disposal           = DISPOSAL_UNSPECIFIED;
    (*gif_state)->prev_disposal      = DISPOSAL_UNSPECIFIED;
    (*gif_state)->current_image      = -1;
    (*gif_state)->row                = 0;
    (*gif_state)->column             = 0;
    (*gif_state)->width              = 0;
    (*gif_state)->height             = 0;
    (*gif_state)->prev_row           = 0;
    (*gif_state)->prev_column        = 0;
    (*gif_state)->prev_width         = 0;
    (*gif_state)->prev_height        = 0;
    (*gif_state)->first_frame        = NULL;

    return SAIL_OK;
}

static void destroy_gif_state(struct gif_state *gif_state) {

    if (gif_state == NULL) {
        return;
    }

    sail_destroy_load_options(gif_state->load_options);
    sail_destroy_save_options(gif_state->save_options);

    sail_free(gif_state->buf);

    if (gif_state->first_frame != NULL) {
        for(int i = 0; i < gif_state->first_frame_height; i++) {
            sail_free(gif_state->first_frame[i]);
        }

        sail_free(gif_state->first_frame);
    }

    sail_free(gif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_gif(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct gif_state *gif_state;
    SAIL_TRY(alloc_gif_state(&gif_state));
    *state = gif_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &gif_state->load_options));

    /* Initialize GIF. */
    int error_code;
    gif_state->gif = DGifOpen(io, my_read_proc, &error_code);

    if (gif_state->gif == NULL) {
        SAIL_LOG_ERROR("GIF: Failed to initialize. GIFLIB error code: %d", error_code);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Initialize internal structs. */
    if (gif_state->gif->SColorMap != NULL) {
        gif_state->background[0] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Red;
        gif_state->background[1] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Green;
        gif_state->background[2] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Blue;
        gif_state->background[3] = 255;
    } else {
        memset(&gif_state->background, 0, sizeof(gif_state->background));
    }

    void *ptr;

    SAIL_TRY(sail_malloc(gif_state->gif->SWidth * sizeof(GifPixelType), &ptr));
    gif_state->buf = ptr;

    gif_state->first_frame_height = gif_state->gif->SHeight;

    SAIL_TRY(sail_malloc(gif_state->first_frame_height * sizeof(unsigned char *), &ptr));
    gif_state->first_frame = ptr;

    for (int i = 0; i < gif_state->first_frame_height; i++) {
        SAIL_TRY(sail_calloc(gif_state->gif->SWidth, 4, &ptr)); /* 4 = RGBA */
        gif_state->first_frame[i] = ptr;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_gif(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct gif_state *gif_state = (struct gif_state *)state;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    image_local->source_image->compression = SAIL_COMPRESSION_LZW;

    gif_state->current_image++;

    gif_state->prev_disposal      = gif_state->disposal;
    gif_state->disposal           = DISPOSAL_UNSPECIFIED;
    gif_state->transparency_index = -1;

    gif_state->prev_row    = gif_state->row;
    gif_state->prev_column = gif_state->column;
    gif_state->prev_width  = gif_state->width;
    gif_state->prev_height = gif_state->height;

    struct sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

    /* Loop through records. */
    while (true) {
        GifRecordType record;

        if (DGifGetRecordType(gif_state->gif, &record) == GIF_ERROR) {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            sail_destroy_image(image_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        switch (record) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(gif_state->gif) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                image_local->width = gif_state->gif->SWidth;
                image_local->height = gif_state->gif->SHeight;

                gif_state->row    = gif_state->gif->Image.Top;
                gif_state->column = gif_state->gif->Image.Left;
                gif_state->width  = gif_state->gif->Image.Width;
                gif_state->height = gif_state->gif->Image.Height;

                if (gif_state->column + gif_state->width > (unsigned)gif_state->gif->SWidth ||
                        gif_state->row + gif_state->height > (unsigned)gif_state->gif->SHeight) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS);
                }
                break;
            }

            case EXTENSION_RECORD_TYPE: {
                int ext_code;
                GifByteType *extension;

                if (DGifGetExtension(gif_state->gif, &ext_code, &extension) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                if (extension == NULL) {
                    break;
                }

                switch (ext_code) {
                    case GRAPHICS_EXT_FUNC_CODE: {
                        /* Disposal method. */
                        gif_state->disposal = (extension[1] >> 2) & 7;

                        /* Delay in 1/100 of seconds. */
                        unsigned delay = *(uint16_t *)(extension + 2);
                        /*
                         * 0 means as fast as possible. However, this makes the frame
                         * almost invisible on modern CPUs. Let's make a small delay of 100 ms
                         * in this case.
                         */
                        image_local->delay = (delay == 0) ? 100 : delay * 10;

                        /* Transparent index. */
                        if (extension[1] & 1) {
                            gif_state->transparency_index = extension[4];
                        }
                        break;
                    }

                    case COMMENT_EXT_FUNC_CODE: {
                        if (gif_state->load_options->options & SAIL_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(gif_private_fetch_comment(extension, last_meta_data_node),
                                                /* cleanup*/ sail_destroy_image(image_local));
                            last_meta_data_node = &(*last_meta_data_node)->next;
                        }
                        break;
                    }

                    case APPLICATION_EXT_FUNC_CODE: {
                        if (gif_state->load_options->options & SAIL_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(gif_private_fetch_application(extension, last_meta_data_node),
                                                /* cleanup */ sail_destroy_image(image_local));
                            last_meta_data_node = &(*last_meta_data_node)->next;
                        }
                        break;
                    }
                }

                /* We don't support other extension types, so just skip them. */
                while (extension != NULL) {
                    if (DGifGetExtensionNext(gif_state->gif, &extension) == GIF_ERROR) {
                        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                        sail_destroy_image(image_local);
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                    }
                }

                break;
            }

            case TERMINATE_RECORD_TYPE: {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
            }

            default: {
                break;
            }
        }

        if (record == IMAGE_DESC_RECORD_TYPE) {
            gif_state->map = (gif_state->gif->Image.ColorMap != NULL) ? gif_state->gif->Image.ColorMap : gif_state->gif->SColorMap;

            if (gif_state->map == NULL) {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
            }

            if (gif_state->gif->Image.Interlace) {
                image_local->source_image->interlaced = true;
            }

            image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
            SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                                /* cleanup */ sail_destroy_image(image_local));

            break;
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_gif(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct gif_state *gif_state = (struct gif_state *)state;

    const int passes = image->source_image->interlaced ? 4 : 1;
    const int last_pass = passes - 1;
    unsigned next_interlaced_row = 0;

    for (int current_pass = 0; current_pass < passes; current_pass++) {
        /* Apply disposal method on the previous frame. */
        if (gif_state->current_image > 0 && current_pass == 0) {
           for (unsigned cc = gif_state->prev_row; cc < gif_state->prev_row+gif_state->prev_height; cc++) {
                unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

                if (gif_state->prev_disposal == DISPOSE_BACKGROUND) {
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
                    memset(gif_state->first_frame[cc] + gif_state->prev_column*4, 0, gif_state->prev_width*4); /* 4 = RGBA */
                }

                memcpy(scan, gif_state->first_frame[cc], image->width * 4);
            }
        }

        /* Read lines. */
        for (unsigned cc = 0; cc < image->height; cc++) {
            unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

            if (cc < gif_state->row || cc >= gif_state->row + gif_state->height) {
                if (current_pass == 0) {
                    memcpy(scan, gif_state->first_frame[cc], image->width * 4);
                }

                continue;
            }

            /* In interlaced mode we skip some lines. */
            bool do_read = false;

            if (gif_state->gif->Image.Interlace) {
                if (cc == gif_state->row) {
                    next_interlaced_row = InterlacedOffset[current_pass] + gif_state->row;
                }

                if (cc == next_interlaced_row) {
                    do_read = true;
                    next_interlaced_row += InterlacedJumps[current_pass];
                }
            } else {
                do_read = true;
            }

            if (do_read) {
                if (DGifGetLine(gif_state->gif, gif_state->buf, gif_state->width) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                memcpy(scan, gif_state->first_frame[cc], image->width * 4);

                for (unsigned i = 0; i < gif_state->width; i++) {
                    if (gif_state->buf[i] == gif_state->transparency_index) {
                        continue;
                    }

                    unsigned char *pixel = scan + (gif_state->column + i)*4;

                    *(pixel+0) = gif_state->map->Colors[gif_state->buf[i]].Red;
                    *(pixel+1) = gif_state->map->Colors[gif_state->buf[i]].Green;
                    *(pixel+2) = gif_state->map->Colors[gif_state->buf[i]].Blue;
                    *(pixel+3) = 255;
                } // for
            }

            if (current_pass == last_pass) {
                memcpy(gif_state->first_frame[cc], scan, image->width * 4);
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_gif(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct gif_state *gif_state = (struct gif_state *)(*state);

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

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_gif(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_gif(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
