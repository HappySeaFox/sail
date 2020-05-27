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
    #include "log.h"
    #include "meta_entry_node.h"
    #include "pixel_formats_mapping_node.h"
    #include "read_features.h"
    #include "read_options.h"
    #include "utils.h"
    #include "write_features.h"
    #include "write_options.h"
#else
    #include <sail-common/config.h>

    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail-common/image.h>
    #include <sail-common/io_common.h>
    #include <sail-common/log.h>
    #include <sail-common/meta_entry_node.h>
    #include <sail-common/pixel_formats_mapping_node.h>
    #include <sail-common/read_features.h>
    #include <sail-common/read_options.h>
    #include <sail-common/utils.h>
    #include <sail-common/write_features.h>
    #include <sail-common/write_options.h>
#endif

#endif
