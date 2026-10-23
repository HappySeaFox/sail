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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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

sail_status_t xbm_private_parse_dimension(const char* str, unsigned* dimension)
{
    errno = 0;

    char* end;
    const long value = strtol(str, &end, 10);

    if (end == str || errno == ERANGE || value <= 0 || value > SAIL_XBM_MAX_DIMENSION)
    {
        SAIL_LOG_ERROR("XBM: Image dimension must be a positive number not greater than %d", SAIL_XBM_MAX_DIMENSION);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    *dimension = (unsigned)value;

    return SAIL_OK;
}

sail_status_t xbm_private_read_hex_literal(struct sail_io* io, unsigned* holder)
{
    char ch;

    do
    {
        SAIL_TRY(io->strict_read(io->stream, &ch, 1));
        /* Bytes above 127 are negative in a signed char, which is undefined behavior for isspace(). */
    } while (isspace((unsigned char)ch) || ch == ',');

    if (ch != '0')
    {
        SAIL_LOG_ERROR("XBM: Expected '0' from '0x', got '%c'", ch);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    SAIL_TRY(io->strict_read(io->stream, &ch, 1));

    if (ch != 'x' && ch != 'X')
    {
        SAIL_LOG_ERROR("XBM: Expected 'x' in hex literal, got '%c'", ch);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    *holder        = 0;
    int hex_digits = 0;

    while (hex_digits < 4)
    {
        SAIL_TRY(io->strict_read(io->stream, &ch, 1));

        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
        {
            unsigned digit;
            if (ch >= '0' && ch <= '9')
            {
                digit = ch - '0';
            }
            else if (ch >= 'a' && ch <= 'f')
            {
                digit = ch - 'a' + 10;
            }
            else
            {
                digit = ch - 'A' + 10;
            }

            *holder = (*holder << 4) | digit;
            hex_digits++;
        }
        else
        {
            SAIL_TRY(io->seek(io->stream, -1, SEEK_CUR));
            break;
        }
    }

    if (hex_digits == 0)
    {
        SAIL_LOG_ERROR("XBM: No hex digits found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    return SAIL_OK;
}

sail_status_t xbm_private_write_header(
    struct sail_io* io, unsigned width, unsigned height, const char* name, enum SailXbmVersion version)
{
    const char* var_name  = (name != NULL && name[0] != '\0') ? name : "image";
    const char* type_name = (version == SAIL_XBM_VERSION_10) ? "unsigned short" : "unsigned char";

    char header[512];
    int written = snprintf(header, sizeof(header),
                           "#define %s_width %u\n"
                           "#define %s_height %u\n"
                           "static %s %s_bits[] = {\n",
                           var_name, width, var_name, height, type_name, var_name);

    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("XBM: Failed to format header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    SAIL_TRY(io->strict_write(io->stream, header, written));

    return SAIL_OK;
}

sail_status_t xbm_private_write_pixels(struct sail_io* io,
                                       const unsigned char* pixels,
                                       unsigned width,
                                       unsigned height,
                                       unsigned bytes_per_line,
                                       enum SailXbmVersion version)
{
    /* Scan lines are addressed with bytes_per_line as it may contain padding bytes. */
    const unsigned bytes_per_row = (width + 7) / 8;
    const unsigned shorts_per_row = (bytes_per_row + 1) / 2;

    size_t total_units;

    if (version == SAIL_XBM_VERSION_10)
    {
        /* X10: uses shorts (2 bytes per unit), padded to even number of bytes per line. */
        SAIL_TRY(sail_size_mul(shorts_per_row, height, &total_units));
    }
    else
    {
        /* X11: uses chars (1 byte per unit). */
        SAIL_TRY(sail_size_mul(bytes_per_row, height, &total_units));
    }

    for (size_t i = 0; i < total_units; i++)
    {
        char hex_str[16];
        int written;

        if (version == SAIL_XBM_VERSION_10)
        {
            /* X10: Write as shorts (2 bytes), padded to an even number of bytes per row. */
            const unsigned row          = (unsigned)(i / shorts_per_row);
            const unsigned short_in_row = (unsigned)(i % shorts_per_row);
            const unsigned column       = short_in_row * 2;
            const size_t row_start      = (size_t)row * bytes_per_line;

            const unsigned char byte1 = xbm_private_reverse_byte(pixels[row_start + column]);
            const unsigned char byte2 =
                (column + 1 < bytes_per_row) ? xbm_private_reverse_byte(pixels[row_start + column + 1]) : 0;
            const unsigned short value = (unsigned short)(byte1 | (byte2 << 8));

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
            const unsigned row    = (unsigned)(i / bytes_per_row);
            const unsigned column = (unsigned)(i % bytes_per_row);

            const unsigned char byte = xbm_private_reverse_byte(pixels[(size_t)row * bytes_per_line + column]);

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
#ifdef _MSC_VER
                strncpy_s(xbm_state->var_name, sizeof(xbm_state->var_name), str_value, sizeof(xbm_state->var_name) - 1);
#else
                strncpy(xbm_state->var_name, str_value, sizeof(xbm_state->var_name) - 1);
#endif
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
