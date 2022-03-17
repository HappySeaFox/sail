/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#ifndef SAIL_CODEC_FEATURES_H
#define SAIL_CODEC_FEATURES_H

#include <stdbool.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Can read or write static images. */
static const char * const SAIL_CODEC_FEATURE_STATIC      = "STATIC";

/* Can read or write animated images. */
static const char * const SAIL_CODEC_FEATURE_ANIMATED    = "ANIMATED";

/* Can read or write multi-paged (but not animated) images. */
static const char * const SAIL_CODEC_FEATURE_MULTI_PAGED = "MULTI-PAGED";

/* Can read or write image meta data like JPEG comments or EXIF. */
static const char * const SAIL_CODEC_FEATURE_META_DATA   = "META-DATA";

/* Can read or write interlaced images. */
static const char * const SAIL_CODEC_FEATURE_INTERLACED  = "INTERLACED";

/* Can read or write embedded ICC profiles. */
static const char * const SAIL_CODEC_FEATURE_ICCP        = "ICCP";

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
