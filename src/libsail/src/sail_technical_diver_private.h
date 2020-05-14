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

#ifndef SAIL_SAIL_TECHNICAL_DIVER_PRIVATE_H
#define SAIL_SAIL_TECHNICAL_DIVER_PRIVATE_H

#include <stdbool.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_context;
struct sail_io;
struct sail_plugin_info;
struct sail_read_options;
struct sail_write_options;

SAIL_HIDDEN sail_error_t start_reading_io_with_options(struct sail_io *io, bool own_io, struct sail_context *context,
                                                       const struct sail_plugin_info *plugin_info,
                                                       const struct sail_read_options *read_options, void **state);

SAIL_HIDDEN sail_error_t start_writing_io_with_options(struct sail_io *io, bool own_io, struct sail_context *context,
                                                       const struct sail_plugin_info *plugin_info,
                                                       const struct sail_write_options *write_options, void **state);

#endif
