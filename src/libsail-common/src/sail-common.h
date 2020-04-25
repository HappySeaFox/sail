/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#ifndef SAIL_SAIL_COMMON_H
#define SAIL_SAIL_COMMON_H

/* Universal libsail-common include. */

#ifdef SAIL_BUILD
    #include "config.h"

    #include "common.h"
    #include "error.h"
    #include "export.h"
    #include "image.h"
    #include "io_common.h"
    #include "io_file.h"
    #include "io_private.h"
    #include "log.h"
    #include "meta_entry_node.h"
    #include "read_features.h"
    #include "read_options.h"
    #include "utils.h"
    #include "write_features.h"
    #include "write_options.h"
#else
    #include <sail/config.h>

    #include <sail/common.h>
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/image.h>
    #include <sail/io_common.h>
    #include <sail/log.h>
    #include <sail/meta_entry_node.h>
    #include <sail/read_features.h>
    #include <sail/read_options.h>
    #include <sail/utils.h>
    #include <sail/write_features.h>
    #include <sail/write_options.h>
#endif

#endif
