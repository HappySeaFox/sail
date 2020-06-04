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

#ifndef SAIL_SAIL_CPP_H
#define SAIL_SAIL_CPP_H

// Universal include
//
#ifdef SAIL_BUILD
    #include "sail-common.h"

    #include "at_scope_exit-c++.h"
    #include "context-c++.h"
    #include "iccp-c++.h"
    #include "image-c++.h"
    #include "image_reader-c++.h"
    #include "image_writer-c++.h"
    #include "io_common-c++.h"
    #include "plugin_info-c++.h"
    #include "read_features-c++.h"
    #include "read_options-c++.h"
    #include "write_features-c++.h"
    #include "write_options-c++.h"
    #include "utils-c++.h"
#else
    #include <sail-common/sail-common.h>

    #include <sail-c++/at_scope_exit-c++.h>
    #include <sail-c++/context-c++.h>
    #include <sail-c++/iccp-c++.h>
    #include <sail-c++/image-c++.h>
    #include <sail-c++/image_reader-c++.h>
    #include <sail-c++/image_writer-c++.h>
    #include <sail-c++/io_common-c++.h>
    #include <sail-c++/plugin_info-c++.h>
    #include <sail-c++/read_features-c++.h>
    #include <sail-c++/read_options-c++.h>
    #include <sail-c++/write_features-c++.h>
    #include <sail-c++/write_options-c++.h>
    #include <sail-c++/utils-c++.h>
#endif

#endif
