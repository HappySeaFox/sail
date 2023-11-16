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

#ifndef SAIL_PNM_HELPERS_H
#define SAIL_PNM_HELPERS_H

#include <stddef.h> /* size_t */

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_image;
struct sail_io;

enum SailPnmVersion {
    SAIL_PNM_VERSION_P1,
    SAIL_PNM_VERSION_P2,
    SAIL_PNM_VERSION_P3,
    SAIL_PNM_VERSION_P4,
    SAIL_PNM_VERSION_P5,
    SAIL_PNM_VERSION_P6,
};

static const char SAIL_PNM_INVALID_STARTING_CHAR = '\0';

SAIL_HIDDEN sail_status_t pnm_private_skip_to_letters_numbers_force_read(struct sail_io *io, char *first_char);

SAIL_HIDDEN sail_status_t pnm_private_skip_to_letters_numbers(struct sail_io *io, char starting_char, char *first_char);

SAIL_HIDDEN sail_status_t pnm_private_read_word(struct sail_io *io, char *str, size_t str_size);

SAIL_HIDDEN sail_status_t pnm_private_read_pixels(struct sail_io *io, struct sail_image *image, unsigned channels, unsigned bpc, double multiplier_to_full_range);

SAIL_HIDDEN enum SailPixelFormat pnm_private_rgb_sail_pixel_format(enum SailPnmVersion pnm_version, unsigned bpc);

SAIL_HIDDEN sail_status_t pnm_private_store_ascii(enum SailPnmVersion pnm_version, struct sail_hash_map *special_properties);

#endif
