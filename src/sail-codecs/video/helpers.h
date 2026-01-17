/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_hash_map;
struct sail_meta_data_node;
struct sail_variant;

SAIL_HIDDEN enum SailPixelFormat video_private_av_pixel_format_to_sail(enum AVPixelFormat av_pix_fmt);

SAIL_HIDDEN enum AVPixelFormat video_private_sail_pixel_format_to_av(enum SailPixelFormat sail_pix_fmt);

SAIL_HIDDEN enum AVPixelFormat video_private_find_best_sail_pixel_format(enum AVPixelFormat source_pix_fmt);

SAIL_HIDDEN sail_status_t video_private_fetch_meta_data(struct AVFormatContext* format_ctx,
                                                        struct AVStream* video_stream,
                                                        struct sail_meta_data_node*** last_meta_data_node);

SAIL_HIDDEN sail_status_t video_private_fetch_special_properties(struct AVFormatContext* format_ctx,
                                                                 struct AVStream* video_stream,
                                                                 struct sail_hash_map* special_properties);

SAIL_HIDDEN bool video_private_load_tuning_key_value_callback(const char* key,
                                                               const struct sail_variant* value,
                                                               void* user_data);
