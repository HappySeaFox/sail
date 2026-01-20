/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)
 *
 *  Copyright (c) 2026 Dmitry Baryshev
 *
 *  The MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-manip/scale.h>

#ifdef SAIL_MANIP_SWSCALE_ENABLED

#include <libavutil/pixfmt.h>

struct sail_image;

/*
 * Private functions for swscale-based scaling.
 * These are internal implementation details and not part of the public API.
 */

/*
 * Convert SailScaling enum to swscale flags.
 */
SAIL_HIDDEN int sail_scaling_to_swscale_flags(enum SailScaling algorithm);

/*
 * Convert SailPixelFormat to AVPixelFormat.
 */
SAIL_HIDDEN enum AVPixelFormat sail_to_av_pixel_format(enum SailPixelFormat sail_pix_fmt);

/*
 * Scale using libswscale.
 */
SAIL_HIDDEN sail_status_t scale_with_swscale(const struct sail_image* src_image,
                                             struct sail_image* dst_image,
                                             enum SailScaling algorithm);

#endif /* SAIL_MANIP_SWSCALE_ENABLED */
