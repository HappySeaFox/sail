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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

sail_status_t pnm_private_skip_to_letters_numbers_force_read(struct sail_io* io, char* first_char)
{

    char c;

    do
    {
        SAIL_TRY(io->strict_read(io->stream, &c, 1));

        if (c == '#')
        {
            do
            {
                SAIL_TRY(io->strict_read(io->stream, &c, 1));
            } while (c != '\n');
        }
    } while (!isalnum(c));

    *first_char = c;

    return SAIL_OK;
}

sail_status_t pnm_private_skip_to_letters_numbers(struct sail_io* io, char starting_char, char* first_char)
{

    if (isalnum(starting_char))
    {
        *first_char = starting_char;
        return SAIL_OK;
    }

    SAIL_TRY(pnm_private_skip_to_letters_numbers_force_read(io, first_char));

    return SAIL_OK;
}

sail_status_t pnm_private_read_word(struct sail_io* io, char* str, size_t str_size)
{

    if (str_size < 2)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    char first_char;
    SAIL_TRY(pnm_private_skip_to_letters_numbers(io, SAIL_PNM_INVALID_STARTING_CHAR, &first_char));

    unsigned i = 0;
    char c     = first_char;

    bool eof;
    SAIL_TRY(io->eof(io->stream, &eof));

    if (eof)
    {
        *(str + i++) = c;
    }
    else
    {
        while ((isalnum(c) || c == '_') && i < str_size - 1 && !eof)
        {
            *(str + i++) = c;

            SAIL_TRY(io->strict_read(io->stream, &c, 1));
            SAIL_TRY(io->eof(io->stream, &eof));
        }
    }

    /* The buffer is full but no word delimiter found. */
    if (i == str_size - 1 && !eof)
    {
        SAIL_LOG_ERROR("PNM: No word delimiter found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    *(str + i) = '\0';

    return SAIL_OK;
}

sail_status_t pnm_private_read_pixels(
    struct sail_io* io, struct sail_image* image, unsigned channels, unsigned bpc, double multiplier_to_full_range)
{

    for (unsigned row = 0; row < image->height; row++)
    {
        uint8_t* scan8   = sail_scan_line(image, row);
        uint16_t* scan16 = sail_scan_line(image, row);

        for (unsigned column = 0; column < image->width; column++)
        {
            for (unsigned channel = 0; channel < channels; channel++)
            {
                char buffer[8];
                SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));

                unsigned value;
#ifdef _MSC_VER
                if (sscanf_s(buffer, "%u", &value) != 1)
                {
#else
                if (sscanf(buffer, "%u", &value) != 1)
                {
#endif
                    SAIL_LOG_ERROR("PNM: Failed to read color value from '%s'", buffer);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
                }

                if (SAIL_LIKELY(bpc == 8))
                {
                    *scan8++ = (uint8_t)(value * multiplier_to_full_range);
                }
                else
                {
                    *scan16++ = (uint16_t)(value * multiplier_to_full_range);
                }
            }
        }
    }

    return SAIL_OK;
}

enum SailPixelFormat pnm_private_rgb_sail_pixel_format(enum SailPnmVersion pnm_version, unsigned bpc)
{

    switch (pnm_version)
    {
    case SAIL_PNM_VERSION_P1:
    case SAIL_PNM_VERSION_P4: return SAIL_PIXEL_FORMAT_BPP1_INDEXED;

    case SAIL_PNM_VERSION_P2:
    case SAIL_PNM_VERSION_P5:
    {
        switch (bpc)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    case SAIL_PNM_VERSION_P3:
    case SAIL_PNM_VERSION_P6:
    {
        switch (bpc)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;

        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    default:
    {
        return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
    }
}

sail_status_t pnm_private_store_ascii(enum SailPnmVersion pnm_version, struct sail_hash_map* special_properties)
{

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    switch (pnm_version)
    {
    case SAIL_PNM_VERSION_P1:
    case SAIL_PNM_VERSION_P2:
    case SAIL_PNM_VERSION_P3:
    {
        sail_set_variant_bool(variant, true);
        break;
    }

    default:
    {
        sail_set_variant_bool(variant, false);
    }
    }

    sail_put_hash_map(special_properties, "pnm-ascii", variant);

    sail_destroy_variant(variant);

    return SAIL_OK;
}

sail_status_t pnm_private_read_pam_header(struct sail_io* io,
                                          unsigned* width,
                                          unsigned* height,
                                          unsigned* depth,
                                          unsigned* maxval,
                                          enum SailPamTuplType* tupltype)
{

    *width    = 0;
    *height   = 0;
    *depth    = 0;
    *maxval   = 0;
    *tupltype = SAIL_PAM_TUPLTYPE_UNKNOWN;

    char buffer[256];

    while (true)
    {
        SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));

        if (strcmp(buffer, "ENDHDR") == 0)
        {
            break;
        }
        else if (strcmp(buffer, "WIDTH") == 0)
        {
            SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));
            if (sscanf(buffer, "%u", width) != 1)
            {
                SAIL_LOG_ERROR("PAM: Failed to read WIDTH");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }
        else if (strcmp(buffer, "HEIGHT") == 0)
        {
            SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));
            if (sscanf(buffer, "%u", height) != 1)
            {
                SAIL_LOG_ERROR("PAM: Failed to read HEIGHT");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }
        else if (strcmp(buffer, "DEPTH") == 0)
        {
            SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));
            if (sscanf(buffer, "%u", depth) != 1)
            {
                SAIL_LOG_ERROR("PAM: Failed to read DEPTH");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }
        else if (strcmp(buffer, "MAXVAL") == 0)
        {
            SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));
            if (sscanf(buffer, "%u", maxval) != 1)
            {
                SAIL_LOG_ERROR("PAM: Failed to read MAXVAL");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }
        else if (strcmp(buffer, "TUPLTYPE") == 0)
        {
            SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));

            if (strcmp(buffer, "BLACKANDWHITE") == 0)
            {
                *tupltype = SAIL_PAM_TUPLTYPE_BLACKANDWHITE;
            }
            else if (strcmp(buffer, "GRAYSCALE") == 0)
            {
                *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE;
            }
            else if (strcmp(buffer, "GRAYSCALE_ALPHA") == 0)
            {
                *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA;
            }
            else if (strcmp(buffer, "RGB") == 0)
            {
                *tupltype = SAIL_PAM_TUPLTYPE_RGB;
            }
            else if (strcmp(buffer, "RGB_ALPHA") == 0)
            {
                *tupltype = SAIL_PAM_TUPLTYPE_RGB_ALPHA;
            }
            else
            {
                SAIL_LOG_WARNING("PAM: Unknown TUPLTYPE '%s', will try to deduce from DEPTH", buffer);
            }
        }
    }

    /* Validate required fields. */
    if (*width == 0 || *height == 0 || *depth == 0 || *maxval == 0)
    {
        SAIL_LOG_ERROR("PAM: Missing required header fields (WIDTH=%u, HEIGHT=%u, DEPTH=%u, MAXVAL=%u)", *width,
                       *height, *depth, *maxval);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Deduce TUPLTYPE from DEPTH if not specified. */
    if (*tupltype == SAIL_PAM_TUPLTYPE_UNKNOWN)
    {
        switch (*depth)
        {
        case 1: *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE; break;
        case 2: *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA; break;
        case 3: *tupltype = SAIL_PAM_TUPLTYPE_RGB; break;
        case 4: *tupltype = SAIL_PAM_TUPLTYPE_RGB_ALPHA; break;
        default:
        {
            SAIL_LOG_ERROR("PAM: Cannot deduce TUPLTYPE from DEPTH=%u", *depth);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
        }
    }

    return SAIL_OK;
}

enum SailPixelFormat pnm_private_pam_sail_pixel_format(enum SailPamTuplType tupltype, unsigned depth, unsigned bpc)
{

    switch (tupltype)
    {
    case SAIL_PAM_TUPLTYPE_BLACKANDWHITE:
    {
        if (depth == 1 && bpc == 1)
        {
            return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
        }
        break;
    }
    case SAIL_PAM_TUPLTYPE_GRAYSCALE:
    {
        if (depth == 1)
        {
            switch (bpc)
            {
            case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
            case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
            }
        }
        break;
    }
    case SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA:
    {
        if (depth == 2)
        {
            switch (bpc)
            {
            case 8: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
            case 16: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;
            }
        }
        break;
    }
    case SAIL_PAM_TUPLTYPE_RGB:
    {
        if (depth == 3)
        {
            switch (bpc)
            {
            case 8: return SAIL_PIXEL_FORMAT_BPP24_RGB;
            case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;
            }
        }
        break;
    }
    case SAIL_PAM_TUPLTYPE_RGB_ALPHA:
    {
        if (depth == 4)
        {
            switch (bpc)
            {
            case 8: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
            case 16: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
            }
        }
        break;
    }
    default: break;
    }

    SAIL_LOG_ERROR("PAM: Unsupported combination of TUPLTYPE=%d, DEPTH=%u, BPC=%u", tupltype, depth, bpc);
    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

sail_status_t pnm_private_pixel_format_to_pnm_params(enum SailPixelFormat pixel_format,
                                                     enum SailPnmVersion* version,
                                                     unsigned* bpc,
                                                     unsigned* depth,
                                                     enum SailPamTuplType* tupltype)
{

    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
    {
        *version  = SAIL_PNM_VERSION_P4;
        *bpc      = 1;
        *depth    = 1;
        *tupltype = SAIL_PAM_TUPLTYPE_BLACKANDWHITE;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
    {
        *version  = SAIL_PNM_VERSION_P5;
        *bpc      = 8;
        *depth    = 1;
        *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:
    {
        *version  = SAIL_PNM_VERSION_P5;
        *bpc      = 16;
        *depth    = 1;
        *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA:
    {
        *version  = SAIL_PNM_VERSION_P7;
        *bpc      = 8;
        *depth    = 2;
        *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA:
    {
        *version  = SAIL_PNM_VERSION_P7;
        *bpc      = 16;
        *depth    = 2;
        *tupltype = SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        *version  = SAIL_PNM_VERSION_P6;
        *bpc      = 8;
        *depth    = 3;
        *tupltype = SAIL_PAM_TUPLTYPE_RGB;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP48_RGB:
    {
        *version  = SAIL_PNM_VERSION_P6;
        *bpc      = 16;
        *depth    = 3;
        *tupltype = SAIL_PAM_TUPLTYPE_RGB;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        *version  = SAIL_PNM_VERSION_P7;
        *bpc      = 8;
        *depth    = 4;
        *tupltype = SAIL_PAM_TUPLTYPE_RGB_ALPHA;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP64_RGBA:
    {
        *version  = SAIL_PNM_VERSION_P7;
        *bpc      = 16;
        *depth    = 4;
        *tupltype = SAIL_PAM_TUPLTYPE_RGB_ALPHA;
        return SAIL_OK;
    }
    default:
    {
        SAIL_LOG_ERROR("PNM: Unsupported pixel format for writing: %s", sail_pixel_format_to_string(pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }
}

sail_status_t pnm_private_write_pnm_header(
    struct sail_io* io, enum SailPnmVersion version, unsigned width, unsigned height, unsigned maxval)
{

    char header[256];
    int written = 0;

    switch (version)
    {
    case SAIL_PNM_VERSION_P4:
    {
        written = snprintf(header, sizeof(header), "P4\n%u %u\n", width, height);
        break;
    }
    case SAIL_PNM_VERSION_P5:
    {
        written = snprintf(header, sizeof(header), "P5\n%u %u\n%u\n", width, height, maxval);
        break;
    }
    case SAIL_PNM_VERSION_P6:
    {
        written = snprintf(header, sizeof(header), "P6\n%u %u\n%u\n", width, height, maxval);
        break;
    }
    default:
    {
        SAIL_LOG_ERROR("PNM: Invalid version for PNM header: %d", version);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }
    }

    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("PNM: Failed to format header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    SAIL_TRY(io->strict_write(io->stream, header, written));

    return SAIL_OK;
}

sail_status_t pnm_private_write_pam_header(
    struct sail_io* io, unsigned width, unsigned height, unsigned depth, unsigned maxval, enum SailPamTuplType tupltype)
{

    char header[512];
    const char* tupltype_str = NULL;

    switch (tupltype)
    {
    case SAIL_PAM_TUPLTYPE_BLACKANDWHITE: tupltype_str = "BLACKANDWHITE"; break;
    case SAIL_PAM_TUPLTYPE_GRAYSCALE: tupltype_str = "GRAYSCALE"; break;
    case SAIL_PAM_TUPLTYPE_GRAYSCALE_ALPHA: tupltype_str = "GRAYSCALE_ALPHA"; break;
    case SAIL_PAM_TUPLTYPE_RGB: tupltype_str = "RGB"; break;
    case SAIL_PAM_TUPLTYPE_RGB_ALPHA: tupltype_str = "RGB_ALPHA"; break;
    default:
    {
        SAIL_LOG_ERROR("PAM: Invalid tuple type: %d", tupltype);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }
    }

    int written = snprintf(header, sizeof(header),
                           "P7\n"
                           "WIDTH %u\n"
                           "HEIGHT %u\n"
                           "DEPTH %u\n"
                           "MAXVAL %u\n"
                           "TUPLTYPE %s\n"
                           "ENDHDR\n",
                           width, height, depth, maxval, tupltype_str);

    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("PAM: Failed to format header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    SAIL_TRY(io->strict_write(io->stream, header, written));

    return SAIL_OK;
}
