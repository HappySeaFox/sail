/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <string.h>

#include <gif_lib.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io.h"

static const int InterlacedOffset[] = {0, 4, 2, 1};
static const int InterlacedJumps[]  = {8, 8, 4, 2};

/*
 * Codec-specific state.
 */
struct gif_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    GifFileType* gif;
    const ColorMapObject* map;
    unsigned char* buf;
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
    unsigned char** first_frame;
    unsigned char background[4]; /* RGBA */

    /* For saving. */
    int frames_written;
    ColorMapObject* color_map;
    bool is_first_frame;
    int transparency_index_save;
    int loop_count;
    int background_color_index;
};

static sail_status_t alloc_gif_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct gif_state** gif_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct gif_state), &ptr));
    *gif_state = ptr;

    **gif_state = (struct gif_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .gif                = NULL,
        .map                = NULL,
        .buf                = NULL,
        .transparency_index = -1,
        .disposal           = DISPOSAL_UNSPECIFIED,
        .prev_disposal      = DISPOSAL_UNSPECIFIED,
        .current_image      = -1,
        .row                = 0,
        .column             = 0,
        .width              = 0,
        .height             = 0,
        .prev_row           = 0,
        .prev_column        = 0,
        .prev_width         = 0,
        .prev_height        = 0,
        .first_frame        = NULL,

        .frames_written          = 0,
        .color_map               = NULL,
        .is_first_frame          = true,
        .transparency_index_save = -1,
        .loop_count              = 0,
        .background_color_index  = 0,
    };

    return SAIL_OK;
}

static void destroy_gif_state(struct gif_state* gif_state)
{
    if (gif_state == NULL)
    {
        return;
    }

    sail_free(gif_state->buf);

    if (gif_state->first_frame != NULL)
    {
        for (int i = 0; i < gif_state->first_frame_height; i++)
        {
            sail_free(gif_state->first_frame[i]);
        }

        sail_free(gif_state->first_frame);
    }

    if (gif_state->color_map != NULL)
    {
        GifFreeMapObject(gif_state->color_map);
    }

    sail_free(gif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_gif(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct gif_state* gif_state;
    SAIL_TRY(alloc_gif_state(io, load_options, NULL, &gif_state));
    *state = gif_state;

    /* Initialize GIF. */
    int error_code;
    gif_state->gif = DGifOpen(gif_state->io, my_read_proc, &error_code);

    if (gif_state->gif == NULL)
    {
        SAIL_LOG_ERROR("GIF: Failed to initialize. GIFLIB error code: %d", error_code);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Initialize internal structs. */
    if (gif_state->gif->SColorMap != NULL)
    {
        gif_state->background[0] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Red;
        gif_state->background[1] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Green;
        gif_state->background[2] = gif_state->gif->SColorMap->Colors[gif_state->gif->SBackGroundColor].Blue;
        gif_state->background[3] = 255;
    }
    else
    {
        memset(&gif_state->background, 0, sizeof(gif_state->background));
    }

    void* ptr;

    SAIL_TRY(sail_malloc(gif_state->gif->SWidth * sizeof(GifPixelType), &ptr));
    gif_state->buf = ptr;

    gif_state->first_frame_height = gif_state->gif->SHeight;

    SAIL_TRY(sail_malloc(gif_state->first_frame_height * sizeof(unsigned char*), &ptr));
    gif_state->first_frame = ptr;

    for (int i = 0; i < gif_state->first_frame_height; i++)
    {
        SAIL_TRY(sail_calloc(gif_state->gif->SWidth, 4, &ptr)); /* 4 = RGBA */
        gif_state->first_frame[i] = ptr;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_gif(void* state, struct sail_image** image)
{
    struct gif_state* gif_state = state;

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (gif_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        image_local->source_image->compression  = SAIL_COMPRESSION_LZW;
    }

    gif_state->current_image++;

    gif_state->prev_disposal      = gif_state->disposal;
    gif_state->disposal           = DISPOSAL_UNSPECIFIED;
    gif_state->transparency_index = -1;

    gif_state->prev_row    = gif_state->row;
    gif_state->prev_column = gif_state->column;
    gif_state->prev_width  = gif_state->width;
    gif_state->prev_height = gif_state->height;

    struct sail_meta_data_node** last_meta_data_node = &image_local->meta_data_node;

    /* Loop through records. */
    while (true)
    {
        GifRecordType record;

        if (DGifGetRecordType(gif_state->gif, &record) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            sail_destroy_image(image_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        switch (record)
        {
        case IMAGE_DESC_RECORD_TYPE:
        {
            if (DGifGetImageDesc(gif_state->gif) == GIF_ERROR)
            {
                SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            image_local->width  = gif_state->gif->SWidth;
            image_local->height = gif_state->gif->SHeight;

            gif_state->row    = gif_state->gif->Image.Top;
            gif_state->column = gif_state->gif->Image.Left;
            gif_state->width  = gif_state->gif->Image.Width;
            gif_state->height = gif_state->gif->Image.Height;

            if (gif_state->column + gif_state->width > (unsigned)gif_state->gif->SWidth
                || gif_state->row + gif_state->height > (unsigned)gif_state->gif->SHeight)
            {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
            }
            break;
        }

        case EXTENSION_RECORD_TYPE:
        {
            int ext_code;
            GifByteType* extension;

            if (DGifGetExtension(gif_state->gif, &ext_code, &extension) == GIF_ERROR)
            {
                SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            if (extension == NULL)
            {
                break;
            }

            switch (ext_code)
            {
            case GRAPHICS_EXT_FUNC_CODE:
            {
                /* Disposal method. */
                gif_state->disposal = (extension[1] >> 2) & 7;

                /* Delay in 1/100 of seconds. */
                unsigned delay = *(uint16_t*)(extension + 2);
                /*
                 * 0 means as fast as possible. However, this makes the frame
                 * almost invisible on modern CPUs. Let's make a small delay of 100 ms
                 * in this case.
                 */
                image_local->delay = (delay == 0) ? 100 : delay * 10;

                /* Transparent index. */
                if (extension[1] & 1)
                {
                    gif_state->transparency_index = extension[4];
                }
                break;
            }

            case COMMENT_EXT_FUNC_CODE:
            {
                if (gif_state->load_options->options & SAIL_OPTION_META_DATA)
                {
                    SAIL_TRY_OR_CLEANUP(gif_private_fetch_comment(extension, last_meta_data_node),
                                        /* cleanup*/ sail_destroy_image(image_local));
                    last_meta_data_node = &(*last_meta_data_node)->next;
                }
                break;
            }

            case APPLICATION_EXT_FUNC_CODE:
            {
                if (gif_state->load_options->options & SAIL_OPTION_META_DATA)
                {
                    SAIL_TRY_OR_CLEANUP(gif_private_fetch_application(extension, last_meta_data_node),
                                        /* cleanup */ sail_destroy_image(image_local));
                    last_meta_data_node = &(*last_meta_data_node)->next;
                }
                break;
            }
            }

            /* We don't support other extension types, so just skip them. */
            while (extension != NULL)
            {
                if (DGifGetExtensionNext(gif_state->gif, &extension) == GIF_ERROR)
                {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    sail_destroy_image(image_local);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }
            }

            break;
        }

        case TERMINATE_RECORD_TYPE:
        {
            sail_destroy_image(image_local);
            return SAIL_ERROR_NO_MORE_FRAMES;
        }

        default:
        {
            break;
        }
        }

        if (record == IMAGE_DESC_RECORD_TYPE)
        {
            gif_state->map =
                (gif_state->gif->Image.ColorMap != NULL) ? gif_state->gif->Image.ColorMap : gif_state->gif->SColorMap;

            if (gif_state->map == NULL)
            {
                sail_destroy_image(image_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
            }

            if (gif_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
            {
                if (gif_state->gif->Image.Interlace)
                {
                    image_local->source_image->interlaced = true;
                }
            }

            image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP32_RGBA;
            image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

            break;
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_gif(void* state, struct sail_image* image)
{
    struct gif_state* gif_state = state;

    const int passes             = gif_state->gif->Image.Interlace ? 4 : 1;
    const int last_pass          = passes - 1;
    unsigned next_interlaced_row = 0;

    for (int current_pass = 0; current_pass < passes; current_pass++)
    {
        /* Apply disposal method on the previous frame. */
        if (gif_state->current_image > 0 && current_pass == 0)
        {
            for (unsigned cc = gif_state->prev_row; cc < gif_state->prev_row + gif_state->prev_height; cc++)
            {
                unsigned char* scan = (unsigned char*)image->pixels + image->width * 4 * cc;

                if (gif_state->prev_disposal == DISPOSE_BACKGROUND)
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
                    memset(gif_state->first_frame[cc] + gif_state->prev_column * 4, 0,
                           gif_state->prev_width * 4); /* 4 = RGBA */
                }

                memcpy(scan, gif_state->first_frame[cc], image->width * 4);
            }
        }

        /* Read lines. */
        for (unsigned cc = 0; cc < image->height; cc++)
        {
            unsigned char* scan = (unsigned char*)image->pixels + image->width * 4 * cc;

            if (cc < gif_state->row || cc >= gif_state->row + gif_state->height)
            {
                if (current_pass == 0)
                {
                    memcpy(scan, gif_state->first_frame[cc], image->width * 4);
                }

                continue;
            }

            /* In interlaced mode we skip some lines. */
            bool do_read = false;

            if (gif_state->gif->Image.Interlace)
            {
                if (cc == gif_state->row)
                {
                    next_interlaced_row = InterlacedOffset[current_pass] + gif_state->row;
                }

                if (cc == next_interlaced_row)
                {
                    do_read              = true;
                    next_interlaced_row += InterlacedJumps[current_pass];
                }
            }
            else
            {
                do_read = true;
            }

            if (do_read)
            {
                if (DGifGetLine(gif_state->gif, gif_state->buf, gif_state->width) == GIF_ERROR)
                {
                    SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                }

                memcpy(scan, gif_state->first_frame[cc], image->width * 4);

                for (unsigned i = 0; i < gif_state->width; i++)
                {
                    if (gif_state->buf[i] == gif_state->transparency_index)
                    {
                        continue;
                    }

                    unsigned char* pixel = scan + (gif_state->column + i) * 4;

                    *(pixel + 0) = gif_state->map->Colors[gif_state->buf[i]].Red;
                    *(pixel + 1) = gif_state->map->Colors[gif_state->buf[i]].Green;
                    *(pixel + 2) = gif_state->map->Colors[gif_state->buf[i]].Blue;
                    *(pixel + 3) = 255;
                } // for
            }

            if (current_pass == last_pass)
            {
                memcpy(gif_state->first_frame[cc], scan, image->width * 4);
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_gif(void** state)
{
    struct gif_state* gif_state = *state;

    *state = NULL;

    if (gif_state->gif != NULL)
    {
        DGifCloseFile(gif_state->gif, /* ErrorCode */ NULL);
    }

    destroy_gif_state(gif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_gif(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct gif_state* gif_state;
    SAIL_TRY(alloc_gif_state(io, NULL, save_options, &gif_state));
    *state = gif_state;

    /* Check compression. GIF always uses LZW. */
    if (gif_state->save_options->compression != SAIL_COMPRESSION_LZW
        && gif_state->save_options->compression != SAIL_COMPRESSION_NONE)
    {
        SAIL_LOG_ERROR("GIF: Only LZW and NONE compressions are supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Handle tuning options. */
    if (gif_state->save_options->tuning != NULL)
    {
        struct gif_tuning_state tuning_state = {.transparency_index_save = &gif_state->transparency_index_save,
                                                .loop_count              = &gif_state->loop_count,
                                                .background_color_index  = &gif_state->background_color_index};
        sail_traverse_hash_map_with_user_data(gif_state->save_options->tuning, gif_private_tuning_key_value_callback,
                                              &tuning_state);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_gif(void* state, const struct sail_image* image)
{
    struct gif_state* gif_state = state;

    int bpp;
    SAIL_TRY(gif_private_pixel_format_to_bpp(image->pixel_format, &bpp));

    if (image->palette == NULL)
    {
        SAIL_LOG_ERROR("GIF: Indexed image must have a palette");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
    }

    /* First frame: initialize GIF and write header. */
    if (gif_state->is_first_frame)
    {
        gif_state->is_first_frame = false;

        /* Build color map from first frame. */
        SAIL_TRY(gif_private_build_color_map(image->palette, &gif_state->color_map));

        /* Initialize GIF for writing. */
        int error_code;
        gif_state->gif = EGifOpen(gif_state->io, my_write_proc, &error_code);

        if (gif_state->gif == NULL)
        {
            SAIL_LOG_ERROR("GIF: Failed to initialize for writing. GIFLIB error code: %d", error_code);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Set GIF version to GIF89a (needed for animation). */
        EGifSetGifVersion(gif_state->gif, true);

        /* Write screen descriptor. */
        gif_state->gif->SWidth           = image->width;
        gif_state->gif->SHeight          = image->height;
        gif_state->gif->SColorResolution = bpp;
        gif_state->gif->SBackGroundColor = gif_state->background_color_index;
        gif_state->gif->SColorMap        = gif_state->color_map;

        if (EGifPutScreenDesc(gif_state->gif, image->width, image->height, bpp, gif_state->background_color_index,
                              gif_state->color_map)
            == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Write NETSCAPE2.0 application extension for looping animation. */
        unsigned char netscape_ext[12] = "NETSCAPE2.0";
        unsigned char netscape_params[3];
        netscape_params[0] = 1;
        netscape_params[1] = (unsigned char)(gif_state->loop_count & 0xFF);
        netscape_params[2] = (unsigned char)((gif_state->loop_count >> 8) & 0xFF);

        if (EGifPutExtensionLeader(gif_state->gif, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        if (EGifPutExtensionBlock(gif_state->gif, 11, netscape_ext) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        if (EGifPutExtensionBlock(gif_state->gif, 3, netscape_params) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        if (EGifPutExtensionTrailer(gif_state->gif) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    /* Write meta data (comments) if requested and available. */
    if (gif_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL)
    {
        for (const struct sail_meta_data_node* node = image->meta_data_node; node != NULL; node = node->next)
        {
            if (node->meta_data->key == SAIL_META_DATA_COMMENT)
            {
                const char* comment = sail_variant_to_string(node->meta_data->value);
                if (comment != NULL)
                {
                    size_t comment_len = strlen(comment);
                    if (comment_len > 0 && comment_len <= 255)
                    {
                        if (EGifPutExtensionLeader(gif_state->gif, COMMENT_EXT_FUNC_CODE) == GIF_ERROR
                            || EGifPutExtensionBlock(gif_state->gif, comment_len, (const unsigned char*)comment)
                                   == GIF_ERROR
                            || EGifPutExtensionTrailer(gif_state->gif) == GIF_ERROR)
                        {
                            SAIL_LOG_ERROR("GIF: Failed to write comment extension");
                            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
                        }
                    }
                }
            }
        }
    }

    /* Write Graphics Control Extension for frame delay and transparency. */
    unsigned char gce[4];
    /* Disposal method in bits 2-4, transparency flag in bit 0. */
    gce[0] = (gif_state->transparency_index_save >= 0) ? 1 : 0; /* Transparency flag. */
    gce[1] = (unsigned char)(image->delay / 10);                /* Delay in 1/100 seconds. */
    gce[2] = (unsigned char)((image->delay / 10) >> 8);
    gce[3] = (gif_state->transparency_index_save >= 0) ? (unsigned char)gif_state->transparency_index_save : 0;

    if (EGifPutExtensionLeader(gif_state->gif, GRAPHICS_EXT_FUNC_CODE) == GIF_ERROR)
    {
        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (EGifPutExtensionBlock(gif_state->gif, 4, gce) == GIF_ERROR)
    {
        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (EGifPutExtensionTrailer(gif_state->gif) == GIF_ERROR)
    {
        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Write image descriptor. */
    if (EGifPutImageDesc(gif_state->gif, 0,                  /* left */
                         0,                                  /* top */
                         image->width, image->height, false, /* interlace */
                         NULL)
        == GIF_ERROR)
    {
        SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    gif_state->frames_written++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_gif(void* state, const struct sail_image* image)
{
    struct gif_state* gif_state = state;

    /* Write pixel data line by line. */
    for (unsigned row = 0; row < image->height; row++)
    {
        const unsigned char* scanline = (const unsigned char*)image->pixels + row * image->bytes_per_line;

        if (EGifPutLine(gif_state->gif, (GifPixelType*)scanline, image->width) == GIF_ERROR)
        {
            SAIL_LOG_ERROR("GIF: %s", GifErrorString(gif_state->gif->Error));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_gif(void** state)
{
    struct gif_state* gif_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Close the GIF file. */
    if (gif_state->gif != NULL)
    {
        if (EGifCloseFile(gif_state->gif, /* ErrorCode */ NULL) == GIF_ERROR)
        {
            destroy_gif_state(gif_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    destroy_gif_state(gif_state);

    return SAIL_OK;
}
