/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <avif/avif.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_iccp;
struct sail_meta_data_node;
struct sail_variant;

SAIL_HIDDEN enum SailPixelFormat avif_private_sail_pixel_format(enum avifPixelFormat avif_pixel_format, uint32_t depth, bool has_alpha);

SAIL_HIDDEN enum SailChromaSubsampling avif_private_sail_chroma_subsampling(enum avifPixelFormat avif_pixel_format);

SAIL_HIDDEN enum SailPixelFormat avif_private_rgb_sail_pixel_format(enum avifRGBFormat rgb_pixel_format, uint32_t depth);

SAIL_HIDDEN uint32_t avif_private_round_depth(uint32_t depth);

SAIL_HIDDEN sail_status_t avif_private_fetch_iccp(const struct avifRWData *avif_iccp, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t avif_private_fetch_meta_data(enum SailMetaData key, const struct avifRWData *avif_rw_data, struct sail_meta_data_node **meta_data_node);

SAIL_HIDDEN bool avif_private_sail_pixel_format_to_avif_rgb_format(enum SailPixelFormat pixel_format, enum avifRGBFormat *rgb_format, uint32_t *depth);

SAIL_HIDDEN sail_status_t avif_private_write_iccp(struct avifImage *avif_image, const struct sail_iccp *iccp);

SAIL_HIDDEN sail_status_t avif_private_write_meta_data(struct avifEncoder *encoder, struct avifImage *avif_image, const struct sail_meta_data_node *meta_data_node);

SAIL_HIDDEN bool avif_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);

SAIL_HIDDEN bool avif_private_load_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);
