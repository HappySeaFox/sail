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
