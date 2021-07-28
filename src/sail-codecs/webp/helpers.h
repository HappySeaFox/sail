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

#ifndef SAIL_WEBP_HELPERS_H
#define SAIL_WEBP_HELPERS_H

#include <stdint.h>

#include <webp/demux.h>

#include "common.h"
#include "error.h"
#include "export.h"

SAIL_HIDDEN void webp_private_fill_color(uint8_t *pixels, unsigned bytes_per_line, unsigned bytes_per_pixel,
                                            uint32_t color, unsigned x, unsigned y, unsigned width, unsigned height);

SAIL_HIDDEN sail_status_t webp_private_blend_over(void *dst_raw, unsigned dst_offset, const void *src_raw,
                                                    unsigned width, unsigned bytes_per_pixel);

SAIL_HIDDEN sail_status_t webp_private_fetch_iccp(WebPDemuxer *webp_demux, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t webp_private_fetch_meta_data(WebPDemuxer *webp_demux, struct sail_meta_data_node **last_meta_data_node);

#endif
