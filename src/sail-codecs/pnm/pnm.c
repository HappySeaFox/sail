/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct pnm_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    bool frame_loaded;
    enum SailPnmVersion version;
    double multiplier_to_full_range;
};

static sail_status_t alloc_pnm_state(struct sail_io *io,
                                        const struct sail_load_options *load_options,
                                        const struct sail_save_options *save_options,
                                        struct pnm_state **pnm_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct pnm_state), &ptr));
    *pnm_state = ptr;

    **pnm_state = (struct pnm_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,
        .multiplier_to_full_range = 0,
    };

    return SAIL_OK;
}

static void destroy_pnm_state(struct pnm_state *pnm_state) {

    if (pnm_state == NULL) {
        return;
    }

    sail_free(pnm_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_pnm(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct pnm_state *pnm_state;
    SAIL_TRY(alloc_pnm_state(io, load_options, NULL, &pnm_state));
    *state = pnm_state;

    /* Init decoder. */
    char str[8];
    SAIL_TRY(sail_read_string_from_io(pnm_state->io, str, sizeof(str)));

    const char pnm = str[1];

    SAIL_LOG_TRACE("PNM: Version '%c'", pnm);

    switch (pnm) {
        case '1': pnm_state->version = SAIL_PNM_VERSION_P1; break;
        case '2': pnm_state->version = SAIL_PNM_VERSION_P2; break;
        case '3': pnm_state->version = SAIL_PNM_VERSION_P3; break;
        case '4': pnm_state->version = SAIL_PNM_VERSION_P4; break;
        case '5': pnm_state->version = SAIL_PNM_VERSION_P5; break;
        case '6': pnm_state->version = SAIL_PNM_VERSION_P6; break;

        default: {
            SAIL_LOG_ERROR("PNM: Unsupported version '%c'", pnm);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_pnm(void *state, struct sail_image **image) {

    struct pnm_state *pnm_state = state;

    if (pnm_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    pnm_state->frame_loaded = true;

    char first_char;
    SAIL_TRY(pnm_private_skip_to_data(pnm_state->io, &first_char));

    char str[32] = { first_char };
    SAIL_TRY(sail_read_string_from_io(pnm_state->io, str + 1, sizeof(str) - 1));

    /* Dimensions. */
    unsigned w, h;
#ifdef _MSC_VER
    if (sscanf_s(str, "%u%u", &w, &h) != 2) {
#else
    if (sscanf(str, "%u%u", &w, &h) != 2) {
#endif
        SAIL_LOG_ERROR("PNM: Failed to read image dimensions");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    unsigned bpp;

    /* Maximum color. */
    if (pnm_state->version == SAIL_PNM_VERSION_P2     ||
            pnm_state->version == SAIL_PNM_VERSION_P3 ||
            pnm_state->version == SAIL_PNM_VERSION_P5 ||
            pnm_state->version == SAIL_PNM_VERSION_P6) {
        SAIL_TRY(pnm_private_skip_to_data(pnm_state->io, &first_char));

        str[0] = first_char;
        SAIL_TRY(sail_read_string_from_io(pnm_state->io, str + 1, sizeof(str) - 1));

        unsigned max_color;
#ifdef _MSC_VER
        if (sscanf_s(str, "%u", &max_color) != 1) {
#else
        if (sscanf(str, "%u", &max_color) != 1) {
#endif
            SAIL_LOG_ERROR("PNM: Failed to read maximum color value");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (max_color <= 255) {
            bpp = 8;
            pnm_state->multiplier_to_full_range = 255.0 / max_color;
        } else if (max_color <= 65535) {
            bpp = 16;
            pnm_state->multiplier_to_full_range = 65535.0 / max_color;
        } else  {
            SAIL_LOG_ERROR("PNM: BPP more than 16 is not supported");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
        }

        SAIL_LOG_TRACE("PNM: Max color(%u), scale(%.1f)", max_color, pnm_state->multiplier_to_full_range);
    } else {
        pnm_state->multiplier_to_full_range = 1;
        bpp = 1;
    }

    enum SailPixelFormat pixel_format = pnm_private_rgb_sail_pixel_format(pnm_state->version, bpp);

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("PNM: Unsupported pixel format");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (pnm_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

    image_local->width          = w;
    image_local->height         = h;
    image_local->pixel_format   = pixel_format;
    image_local->delay          = -1;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    if (pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));

        unsigned char *palette_data = image_local->palette->data;

        *palette_data++ = 255;
        *palette_data++ = 255;
        *palette_data++ = 255;

        *palette_data++ = 0;
        *palette_data++ = 0;
        *palette_data++ = 0;
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_pnm(void *state, struct sail_image *image) {

    const struct pnm_state *pnm_state = state;

    switch (pnm_state->version) {
        case SAIL_PNM_VERSION_P1: {
            break;
        }
        case SAIL_PNM_VERSION_P2: {
            break;
        }
        case SAIL_PNM_VERSION_P3: {
            break;
        }
        case SAIL_PNM_VERSION_P4:
        case SAIL_PNM_VERSION_P5:
        case SAIL_PNM_VERSION_P6: {
            for (unsigned row = 0; row < image->height; row++) {
                SAIL_TRY(pnm_state->io->strict_read(pnm_state->io->stream, sail_scan_line(image, row), image->bytes_per_line));
            }
            break;
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_pnm(void **state) {

    struct pnm_state *pnm_state = *state;

    *state = NULL;

    destroy_pnm_state(pnm_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_pnm(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_pnm(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_pnm(void *state, const struct sail_image *image) {

    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_pnm(void **state) {

    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
