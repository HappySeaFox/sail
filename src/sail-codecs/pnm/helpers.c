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

#include <sail-common/sail-common.h>

#include "helpers.h"

sail_status_t pnm_private_skip_to_letters_numbers_force_read(struct sail_io *io, char *first_char) {

    char c;

    do {
        SAIL_TRY(io->strict_read(io->stream, &c, 1));

        if (c == '#') {
            do {
                SAIL_TRY(io->strict_read(io->stream, &c, 1));
            } while(c != '\n');
        }
    } while (!isalnum(c));

    *first_char = c;

    return SAIL_OK;
}

sail_status_t pnm_private_skip_to_letters_numbers(struct sail_io *io, char starting_char, char *first_char) {

    if (isalnum(starting_char)) {
        *first_char = starting_char;
        return SAIL_OK;
    }

    SAIL_TRY(pnm_private_skip_to_letters_numbers_force_read(io, first_char));

    return SAIL_OK;
}

sail_status_t pnm_private_read_word(struct sail_io *io, char *str, size_t str_size) {

    if (str_size < 2) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    char first_char;
    SAIL_TRY(pnm_private_skip_to_letters_numbers(io, SAIL_PNM_INVALID_STARTING_CHAR, &first_char));

    unsigned i = 0;
    char c = first_char;

    bool eof;
    SAIL_TRY(io->eof(io->stream, &eof));

    sail_status_t saved_status = SAIL_OK;

    while (isalnum(c) && i < str_size - 1 && !eof) {
        *(str + i++) = c;
        SAIL_TRY_OR_EXECUTE(io->strict_read(io->stream, &c, 1),
                            /* on error */ saved_status = __sail_status);
        SAIL_TRY(io->eof(io->stream, &eof));

        if (saved_status != SAIL_OK && !eof) {
            return saved_status;
        }
    }

    /* The buffer is full but no word delimiter found. */
    if (i == str_size - 1 && !eof) {
        SAIL_LOG_ERROR("PNM: No word delimiter found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    *(str + i) = '\0';

    return SAIL_OK;
}

sail_status_t pnm_private_read_pixels(struct sail_io *io, struct sail_image *image, unsigned channels, unsigned bpc, double multiplier_to_full_range) {

    for (unsigned row = 0; row < image->height; row++) {
        uint8_t *scan8 = sail_scan_line(image, row);
        uint16_t *scan16 = sail_scan_line(image, row);

        for (unsigned column = 0; column < image->width; column++) {
            for(unsigned channel = 0; channel < channels; channel++) {
                char buffer[8];
                SAIL_TRY(pnm_private_read_word(io, buffer, sizeof(buffer)));

                unsigned value;
            #ifdef _MSC_VER
                if (sscanf_s(buffer, "%u", &value) != 1) {
            #else
                if (sscanf(buffer, "%u", &value) != 1) {
            #endif
                    SAIL_LOG_ERROR("PNM: Failed to read color value");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
                }

                if (SAIL_LIKELY(bpc == 8)) {
                    *scan8++ = (uint8_t)(value * multiplier_to_full_range);
                } else {
                    *scan16++ = (uint16_t)(value * multiplier_to_full_range);
                }
            }
        }
    }

    return SAIL_OK;
}

enum SailPixelFormat pnm_private_rgb_sail_pixel_format(enum SailPnmVersion pnm_version, unsigned bpc) {

    switch (pnm_version) {
        case SAIL_PNM_VERSION_P1:
        case SAIL_PNM_VERSION_P4: return SAIL_PIXEL_FORMAT_BPP1_INDEXED;

        case SAIL_PNM_VERSION_P2:
        case SAIL_PNM_VERSION_P5: {
            switch (bpc) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        case SAIL_PNM_VERSION_P3:
        case SAIL_PNM_VERSION_P6: {
            switch (bpc) {
                case 8:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

