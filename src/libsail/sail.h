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

#ifndef SAIL_SAIL_H
#define SAIL_SAIL_H

/* Universal libsail include. */

#ifdef SAIL_BUILD
    #include "sail-common.h"

    #include "codec.h"
    #include "codec_bundle.h"
    #include "codec_bundle_node.h"
    #include "codec_bundle_node_private.h"
    #include "codec_bundle_private.h"
    #include "codec_info.h"
    #include "codec_info_private.h"
    #include "codec_layout.h"
    #include "context.h"
    #include "context_private.h"
    #include "ini.h"
    #include "io_file.h"
    #include "io_mem.h"
    #include "io_noop.h"
    #include "sail_advanced.h"
    #include "sail_deep_diver.h"
    #include "sail_junior.h"
    #include "sail_private.h"
    #include "sail_technical_diver.h"
    #include "sail_technical_diver_private.h"
    #include "string_node.h"
    #include "string_node_private.h"
    #include "threading.h"
#else
    #include <sail-common/sail-common.h>

    #include <sail/codec_bundle.h>
    #include <sail/codec_bundle_node.h>
    #include <sail/codec_info.h>
    #include <sail/context.h>
    #include <sail/sail_advanced.h>
    #include <sail/sail_deep_diver.h>
    #include <sail/sail_junior.h>
    #include <sail/sail_technical_diver.h>
    #include <sail/string_node.h>
#endif

#endif
