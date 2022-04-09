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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

sail_status_t sail_start_loading_io(struct sail_io *io, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_loading_io_with_options(io, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_loading_io_with_options(struct sail_io *io,
                                                const struct sail_codec_info *codec_info,
                                                const struct sail_load_options *load_options, void **state) {

    SAIL_TRY(start_loading_io_with_options(io, false, codec_info, load_options, state));

    return SAIL_OK;
}

sail_status_t sail_start_saving_io(struct sail_io *io, const struct sail_codec_info *codec_info, void **state) {

    SAIL_TRY(sail_start_saving_io_with_options(io, codec_info, NULL, state));

    return SAIL_OK;
}

sail_status_t sail_start_saving_io_with_options(struct sail_io *io,
                                                const struct sail_codec_info *codec_info,
                                                const struct sail_save_options *save_options, void **state) {

    SAIL_TRY(start_saving_io_with_options(io, false, codec_info, save_options, state));

    return SAIL_OK;
}
