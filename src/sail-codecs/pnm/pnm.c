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
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static const uint8_t SAIL_PNM_MONO_PALETTE[] = {255, 255, 255, 0, 0, 0};

/*
 * Codec-specific state.
 */
struct pnm_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    bool frame_loaded;
    bool frame_saved;
    enum SailPnmVersion version;
    double multiplier_to_full_range;
    unsigned bpc;

    /* PAM-specific. */
    unsigned pam_depth;
    enum SailPamTuplType pam_tupltype;
};

static sail_status_t alloc_pnm_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct pnm_state** pnm_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct pnm_state), &ptr));
    *pnm_state = ptr;

    **pnm_state = (struct pnm_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,
        .frame_saved  = false,

        .multiplier_to_full_range = 0,
        .bpc                      = 0,

        .pam_depth    = 0,
        .pam_tupltype = SAIL_PAM_TUPLTYPE_UNKNOWN,
    };

    return SAIL_OK;
}

static void destroy_pnm_state(struct pnm_state* pnm_state)
{
    if (pnm_state == NULL)
    {
        return;
    }

    sail_free(pnm_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_pnm(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct pnm_state* pnm_state;
    SAIL_TRY(alloc_pnm_state(io, load_options, NULL, &pnm_state));
    *state = pnm_state;

    /* Init decoder. */
    char str[8];
    SAIL_TRY(pnm_private_read_word(pnm_state->io, str, sizeof(str)));

    const char pnm = str[1];

    SAIL_LOG_TRACE("PNM: Version '%c'", pnm);

    switch (pnm)
    {
    case '1': pnm_state->version = SAIL_PNM_VERSION_P1; break;
    case '2': pnm_state->version = SAIL_PNM_VERSION_P2; break;
    case '3': pnm_state->version = SAIL_PNM_VERSION_P3; break;
    case '4': pnm_state->version = SAIL_PNM_VERSION_P4; break;
    case '5': pnm_state->version = SAIL_PNM_VERSION_P5; break;
    case '6': pnm_state->version = SAIL_PNM_VERSION_P6; break;
    case '7': pnm_state->version = SAIL_PNM_VERSION_P7; break;

    default:
    {
        SAIL_LOG_ERROR("PNM: Unsupported version '%c'", pnm);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_pnm(void* state, struct sail_image** image)
{
    struct pnm_state* pnm_state = state;

    if (pnm_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    pnm_state->frame_loaded = true;

    unsigned w, h;
    enum SailPixelFormat pixel_format;

    /* P7 (PAM) has different header format. */
    if (pnm_state->version == SAIL_PNM_VERSION_P7)
    {
        unsigned maxval;
        SAIL_TRY(pnm_private_read_pam_header(pnm_state->io, &w, &h, &pnm_state->pam_depth, &maxval,
                                             &pnm_state->pam_tupltype));

        if (maxval <= 255)
        {
            pnm_state->bpc                      = 8;
            pnm_state->multiplier_to_full_range = 255.0 / maxval;
        }
        else if (maxval <= 65535)
        {
            pnm_state->bpc                      = 16;
            pnm_state->multiplier_to_full_range = 65535.0 / maxval;
        }
        else
        {
            SAIL_LOG_ERROR("PAM: MAXVAL more than 65535 is not supported");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
        }

        pixel_format = pnm_private_pam_sail_pixel_format(pnm_state->pam_tupltype, pnm_state->pam_depth, pnm_state->bpc);

        SAIL_LOG_TRACE("PAM: W=%u, H=%u, DEPTH=%u, MAXVAL=%u, BPC=%u, TUPLTYPE=%d", w, h, pnm_state->pam_depth, maxval,
                       pnm_state->bpc, pnm_state->pam_tupltype);
    }
    else
    {
        /* P1-P6: Standard PNM header. */
        char buffer[32];

        /* Dimensions. */
        SAIL_TRY(pnm_private_read_word(pnm_state->io, buffer, sizeof(buffer)));

#ifdef _MSC_VER
        if (sscanf_s(buffer, "%u", &w) != 1)
        {
#else
        if (sscanf(buffer, "%u", &w) != 1)
        {
#endif
            SAIL_LOG_ERROR("PNM: Failed to read image dimensions");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
        }

        SAIL_TRY(pnm_private_read_word(pnm_state->io, buffer, sizeof(buffer)));

#ifdef _MSC_VER
        if (sscanf_s(buffer, "%u", &h) != 1)
        {
#else
        if (sscanf(buffer, "%u", &h) != 1)
        {
#endif
            SAIL_LOG_ERROR("PNM: Failed to read image dimensions");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
        }

        /* Maximum color. */
        if (pnm_state->version == SAIL_PNM_VERSION_P2 || pnm_state->version == SAIL_PNM_VERSION_P3
            || pnm_state->version == SAIL_PNM_VERSION_P5 || pnm_state->version == SAIL_PNM_VERSION_P6)
        {
            SAIL_TRY(pnm_private_read_word(pnm_state->io, buffer, sizeof(buffer)));

            unsigned max_color;
#ifdef _MSC_VER
            if (sscanf_s(buffer, "%u", &max_color) != 1)
            {
#else
            if (sscanf(buffer, "%u", &max_color) != 1)
            {
#endif
                SAIL_LOG_ERROR("PNM: Failed to read maximum color value");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
            }

            if (max_color <= 255)
            {
                pnm_state->bpc                      = 8;
                pnm_state->multiplier_to_full_range = 255.0 / max_color;
            }
            else if (max_color <= 65535)
            {
                pnm_state->bpc                      = 16;
                pnm_state->multiplier_to_full_range = 65535.0 / max_color;
            }
            else
            {
                SAIL_LOG_ERROR("PNM: BPP more than 16 is not supported");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
            }

            SAIL_LOG_TRACE("PNM: Max color(%u), scale(%.1f)", max_color, pnm_state->multiplier_to_full_range);
        }
        else
        {
            pnm_state->multiplier_to_full_range = 1;
            pnm_state->bpc                      = 1;
        }

        pixel_format = pnm_private_rgb_sail_pixel_format(pnm_state->version, pnm_state->bpc);
    }

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_ERROR("PNM: Unsupported pixel format");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (pnm_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

    if (pnm_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        if (image_local->source_image == NULL)
        {
            SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                                /* cleanup */ sail_destroy_image(image_local));
        }

        SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->source_image->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(pnm_private_store_ascii(pnm_state->version, image_local->source_image->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    image_local->width          = w;
    image_local->height         = h;
    image_local->pixel_format   = pixel_format;
    image_local->delay          = -1;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    if (pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));

        memcpy(image_local->palette->data, SAIL_PNM_MONO_PALETTE, 6);
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_pnm(void* state, struct sail_image* image)
{
    const struct pnm_state* pnm_state = state;

    switch (pnm_state->version)
    {
    case SAIL_PNM_VERSION_P1:
    {
        for (unsigned row = 0; row < image->height; row++)
        {
            uint8_t* scan  = sail_scan_line(image, row);
            unsigned shift = 8;

            for (unsigned column = 0; column < image->width; column++)
            {
                char first_char;
                SAIL_TRY(pnm_private_skip_to_letters_numbers_force_read(pnm_state->io, &first_char));

                const unsigned value = first_char - '0';

                if (value != 0 && value != 1)
                {
                    SAIL_LOG_ERROR("PNM: Unexpected character '%c'", first_char);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
                }

                if (shift == 8)
                {
                    *scan = 0;
                }

                *scan |= (value << --shift);

                if (shift == 0)
                {
                    scan++;
                    shift = 8;
                }
            }
        }
        break;
    }
    case SAIL_PNM_VERSION_P2:
    {
        SAIL_TRY(pnm_private_read_pixels(pnm_state->io, image, 1, pnm_state->bpc, pnm_state->multiplier_to_full_range));
        break;
    }
    case SAIL_PNM_VERSION_P3:
    {
        SAIL_TRY(pnm_private_read_pixels(pnm_state->io, image, 3, pnm_state->bpc, pnm_state->multiplier_to_full_range));
        break;
    }
    case SAIL_PNM_VERSION_P4:
    case SAIL_PNM_VERSION_P5:
    case SAIL_PNM_VERSION_P6:
    {
        for (unsigned row = 0; row < image->height; row++)
        {
            SAIL_TRY(
                pnm_state->io->strict_read(pnm_state->io->stream, sail_scan_line(image, row), image->bytes_per_line));
        }

        /* For 16-bit formats, swap from big-endian to little-endian (SAIL internal). */
        if (pnm_state->bpc == 16)
        {
            for (unsigned row = 0; row < image->height; row++)
            {
                uint16_t* pixels = (uint16_t*)sail_scan_line(image, row);
                for (unsigned i = 0; i < image->bytes_per_line / 2; i++)
                {
                    pixels[i] = sail_reverse_uint16(pixels[i]);
                }
            }
        }
        break;
    }
    case SAIL_PNM_VERSION_P7:
    {
        /* PAM: Binary format, read raw pixel data. */
        for (unsigned row = 0; row < image->height; row++)
        {
            SAIL_TRY(
                pnm_state->io->strict_read(pnm_state->io->stream, sail_scan_line(image, row), image->bytes_per_line));
        }

        /* For 16-bit formats, swap from big-endian to little-endian (SAIL internal). */
        if (pnm_state->bpc == 16)
        {
            for (unsigned row = 0; row < image->height; row++)
            {
                uint16_t* pixels = (uint16_t*)sail_scan_line(image, row);
                for (unsigned i = 0; i < image->bytes_per_line / 2; i++)
                {
                    pixels[i] = sail_reverse_uint16(pixels[i]);
                }
            }
        }
        break;
    }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_pnm(void** state)
{
    struct pnm_state* pnm_state = *state;

    *state = NULL;

    destroy_pnm_state(pnm_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_pnm(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    SAIL_LOG_TRACE("PNM: Starting save");

    struct pnm_state* pnm_state;
    SAIL_TRY(alloc_pnm_state(io, NULL, save_options, &pnm_state));

    *state = pnm_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_pnm(void* state, const struct sail_image* image)
{
    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct pnm_state* pnm_state = state;

    if (pnm_state->frame_saved)
    {
        SAIL_LOG_ERROR("PNM: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Determine PNM format from pixel format. */
    unsigned bpc, depth;
    enum SailPamTuplType tupltype;
    SAIL_TRY(pnm_private_pixel_format_to_pnm_params(image->pixel_format, &pnm_state->version, &bpc, &depth, &tupltype));

    pnm_state->bpc          = bpc;
    pnm_state->pam_depth    = depth;
    pnm_state->pam_tupltype = tupltype;

    /* Write header. */
    unsigned maxval = (bpc == 1) ? 1 : ((1u << bpc) - 1);

    if (pnm_state->version == SAIL_PNM_VERSION_P7)
    {
        SAIL_TRY(pnm_private_write_pam_header(pnm_state->io, image->width, image->height, depth, maxval, tupltype));
    }
    else
    {
        SAIL_TRY(pnm_private_write_pnm_header(pnm_state->io, pnm_state->version, image->width, image->height, maxval));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_pnm(void* state, const struct sail_image* image)
{
    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct pnm_state* pnm_state = state;

    /* For 16-bit formats, need to swap byte order to big-endian. */
    if (pnm_state->bpc == 16)
    {
        for (unsigned row = 0; row < image->height; row++)
        {
            const uint8_t* scan = sail_scan_line(image, row);
            void* buffer;
            SAIL_TRY(sail_malloc(image->bytes_per_line, &buffer));

            memcpy(buffer, scan, image->bytes_per_line);

            /* Swap to big-endian. */
            uint16_t* pixels = (uint16_t*)buffer;
            for (unsigned i = 0; i < image->bytes_per_line / 2; i++)
            {
                pixels[i] = sail_reverse_uint16(pixels[i]);
            }

            SAIL_TRY_OR_CLEANUP(pnm_state->io->strict_write(pnm_state->io->stream, buffer, image->bytes_per_line),
                                /* cleanup */ sail_free(buffer));
            sail_free(buffer);
        }
    }
    else
    {
        /* For 8-bit and 1-bit formats, write as-is. */
        for (unsigned row = 0; row < image->height; row++)
        {
            SAIL_TRY(
                pnm_state->io->strict_write(pnm_state->io->stream, sail_scan_line(image, row), image->bytes_per_line));
        }
    }

    pnm_state->frame_saved = true;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_pnm(void** state)
{
    SAIL_CHECK_PTR(state);

    struct pnm_state* pnm_state = *state;

    *state = NULL;

    destroy_pnm_state(pnm_state);

    return SAIL_OK;
}
