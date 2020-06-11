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

sail_error_t sail_start_reading_io(struct sail_io *io, struct sail_context *context,
                                   const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_reading_io_with_options(struct sail_io *io, struct sail_context *context,
                                                const struct sail_plugin_info *plugin_info,
                                                const struct sail_read_options *read_options, void **state) {

    SAIL_TRY(start_reading_io_with_options(io, false, context, plugin_info, read_options, state));

    return 0;
}

sail_error_t sail_start_writing_io(struct sail_io *io, struct sail_context *context,
                                   const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_writing_io_with_options(io, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_writing_io_with_options(struct sail_io *io, struct sail_context *context,
                                                const struct sail_plugin_info *plugin_info,
                                                const struct sail_write_options *write_options, void **state) {

    SAIL_TRY(start_writing_io_with_options(io, false, context, plugin_info, write_options, state));

    return 0;
}
