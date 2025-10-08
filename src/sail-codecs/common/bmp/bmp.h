/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#ifndef SAIL_BMP_COMMON_H
#define SAIL_BMP_COMMON_H

#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_image;
struct sail_io;
struct sail_load_options;
struct sail_save_options;

enum SailBmpReadOptions {

    /*
     * No specific BMP flags. This will:
     *   1. Skip the BMP file header
     */
    SAIL_NO_BMP_FLAGS = 0,

    /*
     * Read BMP file header. BMP files have file headers, while
     * ICO files have no BMP file headers.
     */
    SAIL_READ_BMP_FILE_HEADER = 1 << 0,
};

enum SailBmpWriteOptions {

    /*
     * No specific BMP write flags.
     */
    SAIL_NO_BMP_WRITE_FLAGS = 0,

    /*
     * Write BMP file header. BMP files have file headers, while
     * ICO files have no BMP file headers.
     */
    SAIL_WRITE_BMP_FILE_HEADER = 1 << 0,
};

SAIL_HIDDEN sail_status_t bmp_private_read_init(struct sail_io *io, const struct sail_load_options *load_options, void **state, int bmp_load_options);

SAIL_HIDDEN sail_status_t bmp_private_read_seek_next_frame(void *state, struct sail_io *io, struct sail_image **image);

SAIL_HIDDEN sail_status_t bmp_private_read_frame(void *state, struct sail_io *io, struct sail_image *image);

SAIL_HIDDEN sail_status_t bmp_private_read_finish(void **state, struct sail_io *io);

SAIL_HIDDEN sail_status_t bmp_private_write_init(struct sail_io *io, const struct sail_save_options *save_options, void **state, int bmp_write_options);

SAIL_HIDDEN sail_status_t bmp_private_write_seek_next_frame(void *state, struct sail_io *io, const struct sail_image *image);

SAIL_HIDDEN sail_status_t bmp_private_write_frame(void *state, struct sail_io *io, const struct sail_image *image);

SAIL_HIDDEN sail_status_t bmp_private_write_finish(void **state, struct sail_io *io);

#endif
