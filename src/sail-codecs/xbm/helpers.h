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

#ifndef SAIL_XBM_HELPERS_H
#define SAIL_XBM_HELPERS_H

#include <stdbool.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_io;
struct sail_variant;

enum SailXbmVersion {
    SAIL_XBM_VERSION_10 = 10,
    SAIL_XBM_VERSION_11 = 11,
};

/*
 * Tuning state for callback.
 */
struct xbm_state {
    enum SailXbmVersion version;
    char var_name[256];
};

SAIL_HIDDEN unsigned char xbm_private_reverse_byte(unsigned char byte);

SAIL_HIDDEN sail_status_t xbm_private_write_header(struct sail_io *io, unsigned width, unsigned height, const char *name);

SAIL_HIDDEN sail_status_t xbm_private_write_pixels(struct sail_io *io, const unsigned char *pixels, unsigned width, unsigned height, enum SailXbmVersion version);

SAIL_HIDDEN bool xbm_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);

#endif
