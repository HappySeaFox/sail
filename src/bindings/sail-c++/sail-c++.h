/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#ifndef SAIL_SAIL_CPP_H
#define SAIL_SAIL_CPP_H

// Universal include
//
#include <sail-common/sail-common.h>

#include <sail-manip/manip_common.h>

#include <sail-c++/suppress_begin.h>
#include <sail-c++/suppress_c4251.h>

#ifdef SAIL_BUILD
    #include <sail-c++/abstract_io_adapter.h>
    #include <sail-c++/io_base_private.h>
    #include <sail-c++/utils_private.h>
#endif

#include <sail-c++/abstract_io.h>
#include <sail-c++/arbitrary_data.h>
#include <sail-c++/at_scope_exit.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/compression_level.h>
#include <sail-c++/context.h>
#include <sail-c++/conversion_options.h>
#include <sail-c++/iccp.h>
#include <sail-c++/image.h>
#include <sail-c++/image_input.h>
#include <sail-c++/image_output.h>
#include <sail-c++/io_base.h>
#include <sail-c++/io_file.h>
#include <sail-c++/io_memory.h>
#include <sail-c++/load_features.h>
#include <sail-c++/load_options.h>
#include <sail-c++/log.h>
#include <sail-c++/meta_data.h>
#include <sail-c++/ostream.h>
#include <sail-c++/palette.h>
#include <sail-c++/resolution.h>
#include <sail-c++/save_features.h>
#include <sail-c++/save_options.h>
#include <sail-c++/source_image.h>
#include <sail-c++/special_properties.h>
#include <sail-c++/tuning.h>
#include <sail-c++/utils.h>
#include <sail-c++/variant.h>

#include <sail-c++/suppress_end.h>

#endif
