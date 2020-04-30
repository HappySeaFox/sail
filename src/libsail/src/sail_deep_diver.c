/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

sail_error_t sail_start_reading_file_with_options(const char *path, struct sail_context *context,
                                                  const struct sail_plugin_info *plugin_info,
                                                  const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(alloc_io_read_file(path, &io));

    SAIL_TRY_OR_CLEANUP(start_reading_io_with_options(io, true, context, plugin_info_local, read_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_reading_mem_with_options(const void *buffer, unsigned long buffer_length, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info,
                                                 const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct sail_io *io;
    SAIL_TRY(alloc_io_read_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(start_reading_io_with_options(io, true, context, plugin_info, read_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_writing_file_with_options(const char *path, struct sail_context *context,
                                                  const struct sail_plugin_info *plugin_info,
                                                  const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_local;

    if (plugin_info == NULL) {
        SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info_local));
    } else {
        plugin_info_local = plugin_info;
    }

    struct sail_io *io;
    SAIL_TRY(alloc_io_write_file(path, &io));

    SAIL_TRY_OR_CLEANUP(start_writing_io_with_options(io, true, context, plugin_info_local, write_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_start_writing_mem_with_options(void *buffer, unsigned long buffer_length, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info,
                                                 const struct sail_write_options *write_options, void **state) {
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);

    struct sail_io *io;
    SAIL_TRY(alloc_io_write_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(start_writing_io_with_options(io, true, context, plugin_info, write_options, state),
                        /* cleanup */ sail_destroy_io(io));

    return 0;
}

sail_error_t sail_stop_writing_with_written(void *state, unsigned long *written) {

    SAIL_TRY(stop_writing(state, written));

    return 0;
}
