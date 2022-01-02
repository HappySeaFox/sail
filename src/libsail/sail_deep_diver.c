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

sail_status_t sail_start_reading_file_with_options(const char *path, const struct sail_codec_info *codec_info,
                                                  const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PTR(path);

    const struct sail_codec_info *codec_info_local;

    if (codec_info == NULL) {
        SAIL_TRY(sail_codec_info_from_path(path, &codec_info_local));
    } else {
        codec_info_local = codec_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_file(path, &io));

    SAIL_TRY(start_reading_io_with_options(io, true, codec_info_local, read_options, state));

    return SAIL_OK;
}

sail_status_t sail_start_reading_memory_with_options(const void *buffer, size_t buffer_length,
                                                     const struct sail_codec_info *codec_info,
                                                     const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PTR(buffer);

    const struct sail_codec_info *codec_info_local;

    if (codec_info == NULL) {
        SAIL_TRY(sail_codec_info_by_magic_number_from_memory(buffer, buffer_length, &codec_info_local));
    } else {
        codec_info_local = codec_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_memory(buffer, buffer_length, &io));

    SAIL_TRY(start_reading_io_with_options(io, true, codec_info_local, read_options, state));

    return SAIL_OK;
}

sail_status_t sail_start_writing_file_with_options(const char *path, const struct sail_codec_info *codec_info,
                                                  const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PTR(path);

    const struct sail_codec_info *codec_info_local;

    if (codec_info == NULL) {
        SAIL_TRY(sail_codec_info_from_path(path, &codec_info_local));
    } else {
        codec_info_local = codec_info;
    }

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_write_file(path, &io));

    /* The I/O object will be destroyed in this function. */
    SAIL_TRY(start_writing_io_with_options(io, true, codec_info_local, write_options, state));

    return SAIL_OK;
}

sail_status_t sail_start_writing_memory_with_options(void *buffer, size_t buffer_length,
                                                     const struct sail_codec_info *codec_info,
                                                     const struct sail_write_options *write_options, void **state) {
    SAIL_CHECK_PTR(buffer);
    SAIL_CHECK_PTR(codec_info);

    struct sail_io *io;
    SAIL_TRY(sail_alloc_io_read_write_memory(buffer, buffer_length, &io));

    /* The I/O object will be destroyed in this function. */
    SAIL_TRY(start_writing_io_with_options(io, true, codec_info, write_options, state));

    return SAIL_OK;
}

sail_status_t sail_stop_writing_with_written(void *state, size_t *written) {

    SAIL_TRY(stop_writing(state, written));

    return SAIL_OK;
}
