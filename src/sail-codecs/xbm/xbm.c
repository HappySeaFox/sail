/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-common.h"

#include "helpers.h"

enum SailXbmVersion {
    SAIL_XBM_VERSION_10 = 10,
    SAIL_XBM_VERSION_11 = 11,
};

/*
 * Codec-specific state.
 */
struct xbm_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;

    enum SailXbmVersion version;
};

static sail_status_t alloc_xbm_state(struct xbm_state **xbm_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct xbm_state), &ptr));
    *xbm_state = ptr;

    (*xbm_state)->load_options = NULL;
    (*xbm_state)->save_options = NULL;

    (*xbm_state)->frame_loaded = false;

    return SAIL_OK;
}

static void destroy_xbm_state(struct xbm_state *xbm_state) {

    if (xbm_state == NULL) {
        return;
    }

    sail_destroy_load_options(xbm_state->load_options);
    sail_destroy_save_options(xbm_state->save_options);

    sail_free(xbm_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_xbm(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct xbm_state *xbm_state;
    SAIL_TRY(alloc_xbm_state(&xbm_state));
    *state = xbm_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &xbm_state->load_options));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_xbm(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct xbm_state *xbm_state = (struct xbm_state *)state;

    if (xbm_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    xbm_state->frame_loaded = true;

    char buf[512 + 1];

    /* Read width. */
    SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));

    if (strncmp(buf, "#define ", 8) != 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    char *ptr;
    if ((ptr = strstr(buf, "_width ")) == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    unsigned width = atoi(ptr + 6);

    /* Read height. */
    SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));

    if (strncmp(buf, "#define ", 8) != 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if ((ptr = strstr(buf, "_height ")) == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    unsigned height = atoi(ptr + 7);

    /* Skip other defines. */
    do {
        SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));
    } while(strstr(buf, "#define ") != NULL);

    if ((ptr = strchr(buf, '[')) == NULL || strchr(ptr, '{') == NULL) {
        SAIL_LOG_ERROR("XBM: C array declaration is not found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }
    
    if(strstr(buf, "short") != NULL) {
        xbm_state->version = SAIL_XBM_VERSION_10;
        SAIL_LOG_TRACE("XBM: Version 10");
    } else if (strstr(buf, "char") != NULL) {
        xbm_state->version = SAIL_XBM_VERSION_11;
        SAIL_LOG_TRACE("XBM: Version 11");
    } else {
        SAIL_LOG_ERROR("XBM: Data format must be [unsigned] char or [unsigned] short");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
    image_local->source_image->compression = SAIL_COMPRESSION_NONE;

    image_local->width = width;
    image_local->height = height;
    image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Black and white palette. */
    SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, &image_local->palette),
                        /* cleanup */ sail_destroy_image(image_local));

    unsigned char *palette = image_local->palette->data;

    *palette++ = 255;
    *palette++ = 255;
    *palette++ = 255;

    *palette++ = 0;
    *palette++ = 0;
    *palette++ = 0;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_xbm(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    const struct xbm_state *xbm_state = (struct xbm_state *)state;

    unsigned literals_to_read;

    if (SAIL_LIKELY(xbm_state->version == SAIL_XBM_VERSION_11)) {
        literals_to_read = ((image->width + 7) / 8) * image->height;
    } else {
        literals_to_read = (((image->width + 7) / 8 + 1) / 2) * image->height;
    }

    char buf[512 + 1];
    unsigned char *pixels = image->pixels;

    for (unsigned literals_read = 0; literals_read < literals_to_read; ) {
        SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));

        unsigned buf_offset = 0;
        unsigned holder;
        char comma;
        int bytes_consumed;

    #ifdef _MSC_VER
        while (sscanf_s(buf + buf_offset, "%x %c %n", &holder, &comma, 1, &bytes_consumed) == 2) {
    #else
        while (sscanf(buf + buf_offset, "%x %c %n", &holder, &comma, &bytes_consumed) == 2) {
    #endif
            *pixels++ = xbm_private_reverse_byte((unsigned char)holder);
            literals_read++;
            buf_offset += bytes_consumed;
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_xbm(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct xbm_state *xbm_state = (struct xbm_state *)(*state);

    *state = NULL;

    destroy_xbm_state(xbm_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_xbm(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_xbm(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_xbm(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_xbm(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
