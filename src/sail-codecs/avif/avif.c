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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avif/avif.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct avif_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    struct avifIO *avif_io;
    struct avifDecoder *avif_decoder;
};

static sail_status_t alloc_avif_state(struct avif_state **avif_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct avif_state), &ptr));
    *avif_state = ptr;

    if (*avif_state == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    (*avif_state)->read_options  = NULL;
    (*avif_state)->write_options = NULL;
    (*avif_state)->avif_io       = NULL;
    (*avif_state)->avif_decoder  = NULL;

    SAIL_TRY(sail_malloc(sizeof(struct avifIO), &ptr));
    (*avif_state)->avif_io = ptr;

    return SAIL_OK;
}

static void destroy_avif_state(struct avif_state *avif_state) {

    if (avif_state == NULL) {
        return;
    }

    sail_destroy_read_options(avif_state->read_options);
    sail_destroy_write_options(avif_state->write_options);

    sail_free(avif_state->avif_io);

    avifDecoderDestroy(avif_state->avif_decoder);

    sail_free(avif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v5_gif(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_READ_OPTIONS_PTR(read_options);

    /* Allocate a new state. */
    struct avif_state *avif_state;
    SAIL_TRY(alloc_avif_state(&avif_state));
    *state = avif_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &avif_state->read_options));

    /* Initialize GIF. */
    int error_code;
    avif_state->gif = DGifOpen(io, my_read_proc, &error_code);

    if (avif_state->gif == NULL) {
        SAIL_LOG_ERROR("GIF: Failed to initialize. GIFLIB error code: %d", error_code);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Initialize internal structs. */
    if (avif_state->gif->SColorMap != NULL) {
        avif_state->background[0] = avif_state->gif->SColorMap->Colors[avif_state->gif->SBackGroundColor].Red;
        avif_state->background[1] = avif_state->gif->SColorMap->Colors[avif_state->gif->SBackGroundColor].Green;
        avif_state->background[2] = avif_state->gif->SColorMap->Colors[avif_state->gif->SBackGroundColor].Blue;
        avif_state->background[3] = 255;
    } else {
        memset(&avif_state->background, 0, sizeof(avif_state->background));
    }

    void *ptr;

    SAIL_TRY(sail_malloc(avif_state->gif->SWidth * sizeof(GifPixelType), &ptr));
    avif_state->buf = ptr;

    avif_state->first_frame_height = avif_state->gif->SHeight;

    SAIL_TRY(sail_malloc(avif_state->first_frame_height * sizeof(unsigned char *), &ptr));
    avif_state->first_frame = ptr;

    for (int i = 0; i < avif_state->first_frame_height; i++) {
        SAIL_TRY(sail_calloc(avif_state->gif->SWidth, 4, &ptr)); /* 4 = RGBA */
        avif_state->first_frame[i] = ptr;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v5_gif(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_IMAGE_PTR(image);

    struct avif_state *avif_state = (struct avif_state *)state;

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->compression = SAIL_COMPRESSION_LZW;
    image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    avif_state->current_image++;

    avif_state->prev_disposal      = avif_state->disposal;
    avif_state->disposal           = DISPOSAL_UNSPECIFIED;
    avif_state->transparency_index = -1;

    avif_state->prev_row    = avif_state->row;
    avif_state->prev_column = avif_state->column;
    avif_state->prev_width  = avif_state->width;
    avif_state->prev_height = avif_state->height;

    /* Loop through records. */
    while (true) {
        GifRecordType record;

        if (DGifGetRecordType(avif_state->gif, &record) == GIF_ERROR) {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(avif_state->gif->Error));
            sail_destroy_image(image_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        switch (record) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(avif_state->gif) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(avif_state->gif->Error));
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                image_local->width = avif_state->gif->SWidth;
                image_local->height = avif_state->gif->SHeight;

                avif_state->row    = avif_state->gif->Image.Top;
                avif_state->column = avif_state->gif->Image.Left;
                avif_state->width  = avif_state->gif->Image.Width;
                avif_state->height = avif_state->gif->Image.Height;

                if (avif_state->column + avif_state->width > (unsigned)avif_state->gif->SWidth ||
                        avif_state->row + avif_state->height > (unsigned)avif_state->gif->SHeight) {
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INCORRECT_IMAGE_DIMENSIONS);
                }
                break;
            }

            case EXTENSION_RECORD_TYPE: {
                int ext_code;
                GifByteType *extension;

                if (DGifGetExtension(avif_state->gif, &ext_code, &extension) == GIF_ERROR) {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(avif_state->gif->Error));
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                if (extension == NULL) {
                    break;
                }

                switch (ext_code) {
                    case GRAPHICS_EXT_FUNC_CODE: {
                        /* Disposal method. */
                        avif_state->disposal = (extension[1] >> 2) & 7;

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
                            avif_state->transparency_index = extension[4];
                        }
                        break;
                    }

                    case COMMENT_EXT_FUNC_CODE: {
                        if (avif_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(gif_private_fetch_comment(extension, &image_local->meta_data_node),
                                                /* cleanup*/ sail_destroy_image(image_local));
                        }
                        break;
                    }

                    case APPLICATION_EXT_FUNC_CODE: {
                        if (avif_state->read_options->io_options & SAIL_IO_OPTION_META_DATA) {
                            SAIL_TRY_OR_CLEANUP(gif_private_fetch_application(extension, &image_local->meta_data_node),
                                                /* cleanup */ sail_destroy_image(image_local));
                        }
                        break;
                    }
                }

                /* We don't support other extension types, so just skip them. */
                while (extension != NULL) {
                    if (DGifGetExtensionNext(avif_state->gif, &extension) == GIF_ERROR) {
                        SAIL_LOG_ERROR("GIF: %s", GifErrorString(avif_state->gif->Error));
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
            if (avif_state->current_image > 0) {
                image_local->animated = true;
            }

            avif_state->map = (avif_state->gif->Image.ColorMap != NULL) ? avif_state->gif->Image.ColorMap : avif_state->gif->SColorMap;

            if (avif_state->map == NULL) {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
            }

            if (avif_state->gif->Image.Interlace) {
                image_local->source_image->properties |= SAIL_IMAGE_PROPERTY_INTERLACED;
                image_local->interlaced_passes = 4;
            }

            image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;
            SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                                /* cleanup */ sail_destroy_image(image_local));

            avif_state->layer = -1;
            avif_state->current_pass = -1;

            break;
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_pass_v5_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct avif_state *avif_state = (struct avif_state *)state;

    avif_state->layer++;
    avif_state->current_pass++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v5_gif(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct avif_state *avif_state = (struct avif_state *)state;

    /* Apply disposal method on the previous frame. */
    if (avif_state->current_image > 0 && avif_state->current_pass == 0) {
       for (unsigned cc = avif_state->prev_row; cc < avif_state->prev_row+avif_state->prev_height; cc++) {
            unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

            if (avif_state->prev_disposal == DISPOSE_BACKGROUND) {
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
                memset(avif_state->first_frame[cc] + avif_state->prev_column*4, 0, avif_state->prev_width*4); /* 4 = RGBA */
            }

            memcpy(scan, avif_state->first_frame[cc], image->width * 4);
        }
    }

    /* Read lines. */
    for (unsigned cc = 0; cc < image->height; cc++) {
        unsigned char *scan = (unsigned char *)image->pixels + image->width*4*cc;

        if (cc < avif_state->row || cc >= avif_state->row + avif_state->height) {
            if (avif_state->current_pass == 0) {
                memcpy(scan, avif_state->first_frame[cc], image->width * 4);
            }

            continue;
        }

        /* In interlaced mode we skip some lines. */
        bool do_read = false;

        if (avif_state->gif->Image.Interlace) {
            if (cc == avif_state->row) {
                avif_state->next_interlaced_row = InterlacedOffset[avif_state->layer] + avif_state->row;
            }

            if (cc == avif_state->next_interlaced_row) {
                do_read = true;
                avif_state->next_interlaced_row += InterlacedJumps[avif_state->layer];
            }
        }
        else { // !s32erlaced
            do_read = true;
        }

        if (do_read) {
            if (DGifGetLine(avif_state->gif, avif_state->buf, avif_state->width) == GIF_ERROR) {
                SAIL_LOG_ERROR("GIF: %s", GifErrorString(avif_state->gif->Error));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            memcpy(scan, avif_state->first_frame[cc], image->width * 4);

            for (unsigned i = 0; i < avif_state->width; i++) {
                if (avif_state->buf[i] == avif_state->transparency_index) {
                    continue;
                }

                unsigned char *pixel = scan + (avif_state->column + i)*4;

                *(pixel+0) = avif_state->map->Colors[avif_state->buf[i]].Red;
                *(pixel+1) = avif_state->map->Colors[avif_state->buf[i]].Green;
                *(pixel+2) = avif_state->map->Colors[avif_state->buf[i]].Blue;
                *(pixel+3) = 255;
            } // for
        }

        if (avif_state->current_pass == image->interlaced_passes-1) {
            memcpy(avif_state->first_frame[cc], scan, image->width * 4);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v5_gif(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct avif_state *avif_state = (struct avif_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    if (avif_state->gif != NULL) {
        DGifCloseFile(avif_state->gif, /* ErrorCode */ NULL);
    }

    destroy_avif_state(avif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v5_gif(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_WRITE_OPTIONS_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v5_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_pass_v5_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v5_gif(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v5_gif(void **state, struct sail_io *io) {

    SAIL_CHECK_STATE_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
