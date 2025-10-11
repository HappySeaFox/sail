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

#include <stdio.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static const unsigned char reverse_lookup_4bits[] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
};

unsigned char xbm_private_reverse_byte(unsigned char byte)
{
    return (reverse_lookup_4bits[byte & 0xF] << 4) | reverse_lookup_4bits[byte >> 4];
}

sail_status_t xbm_private_write_header(struct sail_io* io, unsigned width, unsigned height, const char* name)
{
    const char* var_name = (name != NULL && name[0] != '\0') ? name : "image";

    char header[512];
    int written = snprintf(header, sizeof(header),
                           "#define %s_width %u\n"
                           "#define %s_height %u\n"
                           "static unsigned char %s_bits[] = {\n",
                           var_name, width, var_name, height, var_name);

    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("XBM: Failed to format header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    SAIL_TRY(io->strict_write(io->stream, header, written));

    return SAIL_OK;
}

sail_status_t xbm_private_write_pixels(
    struct sail_io* io, const unsigned char* pixels, unsigned width, unsigned height, enum SailXbmVersion version)
{
    const unsigned bytes_per_line = (width + 7) / 8;
    unsigned total_units;

    if (version == SAIL_XBM_VERSION_10)
    {
        /* X10: uses shorts (2 bytes per unit), padded to even number of bytes per line. */
        total_units = (((bytes_per_line + 1) / 2) * height);
    }
    else
    {
        /* X11: uses chars (1 byte per unit). */
        total_units = bytes_per_line * height;
    }

    for (unsigned i = 0; i < total_units; i++)
    {
        char hex_str[16];
        int written;
        unsigned short value;

        if (version == SAIL_XBM_VERSION_10)
        {
            /* X10: Write as shorts (2 bytes). */
            const unsigned byte_idx = i * 2;
            const unsigned char byte1 =
                (byte_idx < bytes_per_line * height) ? xbm_private_reverse_byte(pixels[byte_idx]) : 0;
            const unsigned char byte2 =
                (byte_idx + 1 < bytes_per_line * height) ? xbm_private_reverse_byte(pixels[byte_idx + 1]) : 0;
            value = byte1 | (byte2 << 8);

            if (i == total_units - 1)
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%04x\n", value);
            }
            else if ((i + 1) % 8 == 0)
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%04x,\n", value);
            }
            else
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%04x, ", value);
            }
        }
        else
        {
            /* X11: Write as chars (1 byte). */
            const unsigned char byte = xbm_private_reverse_byte(pixels[i]);

            if (i == total_units - 1)
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%02x\n", byte);
            }
            else if ((i + 1) % 12 == 0)
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%02x,\n", byte);
            }
            else
            {
                written = snprintf(hex_str, sizeof(hex_str), "0x%02x, ", byte);
            }
        }

        if (written < 0 || (size_t)written >= sizeof(hex_str))
        {
            SAIL_LOG_ERROR("XBM: Failed to format pixel data");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
        }

        SAIL_TRY(io->strict_write(io->stream, hex_str, written));
    }

    /* Write closing brace. */
    SAIL_TRY(io->strict_write(io->stream, "};\n", 3));

    return SAIL_OK;
}

bool xbm_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct xbm_state* xbm_state = user_data;

    if (strcmp(key, "xbm-version") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);

            if (strcmp(str_value, "X10") == 0 || strcmp(str_value, "x10") == 0)
            {
                xbm_state->version = SAIL_XBM_VERSION_10;
                SAIL_LOG_TRACE("XBM: Writing in X10 format (short)");
            }
            else if (strcmp(str_value, "X11") == 0 || strcmp(str_value, "x11") == 0)
            {
                xbm_state->version = SAIL_XBM_VERSION_11;
                SAIL_LOG_TRACE("XBM: Writing in X11 format (char)");
            }
            else
            {
                SAIL_LOG_WARNING("XBM: Unknown version '%s', using X11 (default)", str_value);
            }
        }
        else
        {
            SAIL_LOG_ERROR("XBM: 'xbm-version' must be a string");
        }
    }
    else if (strcmp(key, "xbm-name") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);

            if (str_value != NULL)
            {
                strncpy(xbm_state->var_name, str_value, sizeof(xbm_state->var_name) - 1);
                xbm_state->var_name[sizeof(xbm_state->var_name) - 1] = '\0';
                SAIL_LOG_TRACE("XBM: Using variable name '%s'", xbm_state->var_name);
            }
        }
        else
        {
            SAIL_LOG_ERROR("XBM: 'xbm-name' must be a string");
        }
    }

    return true;
}
