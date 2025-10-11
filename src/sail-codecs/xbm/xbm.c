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
#include <stdlib.h> /* atoi() */
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static const unsigned char SAIL_XBM_MONO_PALETTE[] = {255, 255, 255, 0, 0, 0};

/*
 * Codec-specific state.
 */
struct xbm_codec_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    bool frame_loaded;

    enum SailXbmVersion version;
    struct xbm_state tuning_state;
};

static sail_status_t alloc_xbm_codec_state(struct sail_io* io,
                                           const struct sail_load_options* load_options,
                                           const struct sail_save_options* save_options,
                                           struct xbm_codec_state** xbm_codec_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct xbm_codec_state), &ptr));
    *xbm_codec_state = ptr;

    **xbm_codec_state = (struct xbm_codec_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,
        .version      = SAIL_XBM_VERSION_11,
    };

    (*xbm_codec_state)->tuning_state.version     = SAIL_XBM_VERSION_11;
    (*xbm_codec_state)->tuning_state.var_name[0] = '\0';

    return SAIL_OK;
}

static void destroy_xbm_codec_state(struct xbm_codec_state* xbm_codec_state)
{
    if (xbm_codec_state == NULL)
    {
        return;
    }

    sail_free(xbm_codec_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_xbm(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct xbm_codec_state* xbm_codec_state;
    SAIL_TRY(alloc_xbm_codec_state(io, load_options, NULL, &xbm_codec_state));
    *state = xbm_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_xbm(void* state, struct sail_image** image)
{
    struct xbm_codec_state* xbm_state = state;

    if (xbm_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    xbm_state->frame_loaded = true;

    char buf[512 + 1];

    /* Read width. */
    SAIL_TRY(sail_read_string_from_io(xbm_state->io, buf, sizeof(buf)));

    if (strncmp(buf, "#define ", 8) != 0)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    char* ptr;
    if ((ptr = strstr(buf, "_width ")) == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    unsigned width = atoi(ptr + 6);

    /* Read height. */
    SAIL_TRY(sail_read_string_from_io(xbm_state->io, buf, sizeof(buf)));

    if (strncmp(buf, "#define ", 8) != 0)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    if ((ptr = strstr(buf, "_height ")) == NULL)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    unsigned height = atoi(ptr + 7);

    /* Skip other defines. */
    do
    {
        SAIL_TRY(sail_read_string_from_io(xbm_state->io, buf, sizeof(buf)));
    } while (strstr(buf, "#define ") != NULL);

    if ((ptr = strchr(buf, '[')) == NULL || strchr(ptr, '{') == NULL)
    {
        SAIL_LOG_ERROR("XBM: C array declaration is not found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    if (strstr(buf, "short") != NULL)
    {
        xbm_state->version = SAIL_XBM_VERSION_10;
        SAIL_LOG_TRACE("XBM: Version 10");
    }
    else if (strstr(buf, "char") != NULL)
    {
        xbm_state->version = SAIL_XBM_VERSION_11;
        SAIL_LOG_TRACE("XBM: Version 11");
    }
    else
    {
        SAIL_LOG_ERROR("XBM: Data format must be [unsigned] char or [unsigned] short");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Construct image. */
    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (xbm_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

    image_local->width          = width;
    image_local->height         = height;
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Black and white palette. */
    SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, &image_local->palette),
                        /* cleanup */ sail_destroy_image(image_local));

    memcpy(image_local->palette->data, SAIL_XBM_MONO_PALETTE, 6);

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_xbm(void* state, struct sail_image* image)
{
    const struct xbm_codec_state* xbm_codec_state = state;

    unsigned literals_to_read;

    if (SAIL_LIKELY(xbm_codec_state->version == SAIL_XBM_VERSION_11))
    {
        literals_to_read = ((image->width + 7) / 8) * image->height;
    }
    else
    {
        literals_to_read = (((image->width + 7) / 8 + 1) / 2) * image->height;
    }

    SAIL_LOG_TRACE("XBM: Literals to read(%u)", literals_to_read);

    char buf[512 + 1];
    unsigned char* pixels = image->pixels;

    for (unsigned literals_read = 0; literals_read < literals_to_read;)
    {
        SAIL_TRY(sail_read_string_from_io(xbm_codec_state->io, buf, sizeof(buf)));

        unsigned buf_offset = 0;
        unsigned holder;
        char comma;
        int bytes_consumed;

#ifdef _MSC_VER
        while (sscanf_s(buf + buf_offset, "%x %c %n", &holder, &comma, 1, &bytes_consumed) == 2)
        {
#else
        while (sscanf(buf + buf_offset, "%x %c %n", &holder, &comma, &bytes_consumed) == 2)
        {
#endif

            if (SAIL_LIKELY(xbm_codec_state->version == SAIL_XBM_VERSION_11))
            {
                *pixels++ = xbm_private_reverse_byte((unsigned char)holder);
            }
            else
            {
                *pixels++ = xbm_private_reverse_byte((unsigned char)(holder & 0xff));
                *pixels++ = xbm_private_reverse_byte((unsigned char)(holder >> 8));
            }

            literals_read++;
            buf_offset += bytes_consumed;
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_xbm(void** state)
{
    struct xbm_codec_state* xbm_codec_state = *state;

    *state = NULL;

    destroy_xbm_codec_state(xbm_codec_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_xbm(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    *state = NULL;

    /* Allocate a new state. */
    struct xbm_codec_state* xbm_codec_state;
    SAIL_TRY(alloc_xbm_codec_state(io, NULL, save_options, &xbm_codec_state));
    *state = xbm_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_xbm(void* state, const struct sail_image* image)
{
    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xbm_codec_state* xbm_codec_state = state;

    if (xbm_codec_state->frame_loaded)
    {
        SAIL_LOG_ERROR("XBM: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* XBM only supports 1-bit indexed format. */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP1_INDEXED)
    {
        SAIL_LOG_ERROR("XBM: Only BPP1-INDEXED pixel format is supported for saving, got %s",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Process tuning options. */
    if (xbm_codec_state->save_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(xbm_codec_state->save_options->tuning,
                                              xbm_private_tuning_key_value_callback, &xbm_codec_state->tuning_state);
    }

    /* Copy tuning results to codec state. */
    xbm_codec_state->version = xbm_codec_state->tuning_state.version;

    /* Write XBM header. */
    const char* name =
        (xbm_codec_state->tuning_state.var_name[0] != '\0') ? xbm_codec_state->tuning_state.var_name : NULL;
    SAIL_TRY(xbm_private_write_header(xbm_codec_state->io, image->width, image->height, name));

    xbm_codec_state->frame_loaded = true;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_xbm(void* state, const struct sail_image* image)
{
    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct xbm_codec_state* xbm_codec_state = state;

    /* Write pixel data. */
    SAIL_TRY(xbm_private_write_pixels(xbm_codec_state->io, image->pixels, image->width, image->height,
                                      xbm_codec_state->version));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_xbm(void** state)
{
    SAIL_CHECK_PTR(state);

    struct xbm_codec_state* xbm_codec_state = *state;

    *state = NULL;

    destroy_xbm_codec_state(xbm_codec_state);

    return SAIL_OK;
}
