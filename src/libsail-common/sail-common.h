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

#ifndef SAIL_SAIL_COMMON_H
#define SAIL_SAIL_COMMON_H

/* Universal libsail-common include. */

#ifdef SAIL_BUILD
    #include "config.h"

    #include "common.h"
    #include "common_serialize.h"
    #include "compiler_specifics.h"
    #include "compression_level.h"
    #include "error.h"
    #include "export.h"
    #include "hash_map.h"
    #include "hash_map_p.h"
    #include "iccp.h"
    #include "image.h"
    #include "io_common.h"
    #include "linked_list_node.h"
    #include "load_features.h"
    #include "load_options.h"
    #include "log.h"
    #include "memory.h"
    #include "meta_data.h"
    #include "meta_data_node.h"
    #include "palette.h"
    #include "pixel.h"
    #include "resolution.h"
    #include "save_features.h"
    #include "save_options.h"
    #include "source_image.h"
    #include "string_node.h"
    #include "utils.h"
    #include "variant.h"
    #include "variant_node.h"
#else
    #include <sail-common/config.h>

    #include <sail-common/common.h>
    #include <sail-common/common_serialize.h>
    #include <sail-common/compiler_specifics.h>
    #include <sail-common/compression_level.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail-common/hash_map.h>
    #include <sail-common/iccp.h>
    #include <sail-common/image.h>
    #include <sail-common/io_common.h>
    #include <sail-common/load_features.h>
    #include <sail-common/load_options.h>
    #include <sail-common/log.h>
    #include <sail-common/memory.h>
    #include <sail-common/meta_data.h>
    #include <sail-common/meta_data_node.h>
    #include <sail-common/palette.h>
    #include <sail-common/pixel.h>
    #include <sail-common/resolution.h>
    #include <sail-common/save_features.h>
    #include <sail-common/save_options.h>
    #include <sail-common/source_image.h>
    #include <sail-common/string_node.h>
    #include <sail-common/utils.h>
    #include <sail-common/variant.h>
    #include <sail-common/variant_node.h>
#endif

#endif
