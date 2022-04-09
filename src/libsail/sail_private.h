/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#ifndef SAIL_SAIL_PRIVATE_H
#define SAIL_SAIL_PRIVATE_H

#include <stdbool.h>
#include <stddef.h> /* size_t */

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_codec_info;
struct sail_codec;
struct sail_save_features;

struct hidden_state {

    struct sail_io *io;
    bool own_io;

    /*
     * Save operations use save options to check if the interlaced mode was requested on later stages.
     * It's also used to check if the supplied pixel format is supported.
     */
    struct sail_save_options *save_options;

    /* Local state passed to codec loading and saving functions. */
    void *state;

    /* Pointers to internal data structures so no need to free these. */
    const struct sail_codec_info *codec_info;
    const struct sail_codec *codec;
};

SAIL_HIDDEN sail_status_t load_codec_by_codec_info(const struct sail_codec_info *codec_info,
                                                    const struct sail_codec **codec);

SAIL_HIDDEN void destroy_hidden_state(struct hidden_state *state);

SAIL_HIDDEN sail_status_t stop_saving(void *state, size_t *written);

SAIL_HIDDEN sail_status_t allowed_write_output_pixel_format(const struct sail_save_features *save_features, enum SailPixelFormat pixel_format);

#endif
