/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_AVIF_HELPERS_H
#define SAIL_AVIF_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include <avif/avif.h>

#include "common.h"
#include "error.h"
#include "export.h"

SAIL_HIDDEN enum SailPixelFormat avif_private_sail_pixel_format(enum avifPixelFormat avif_pixel_format, uint32_t depth, bool has_alpha);

SAIL_HIDDEN enum SailChromaSubsampling avif_private_sail_chroma_subsampling(enum avifPixelFormat avif_pixel_format);

SAIL_HIDDEN enum SailPixelFormat avif_private_rgb_sail_pixel_format(enum avifRGBFormat rgb_pixel_format, uint32_t depth);

SAIL_HIDDEN uint32_t avif_private_round_depth(uint32_t depth);

SAIL_HIDDEN sail_status_t avif_private_fetch_iccp(const struct avifRWData *avif_iccp, struct sail_iccp **iccp);

#endif
