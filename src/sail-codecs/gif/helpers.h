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

#pragma once

#include <gif_lib.h>

#include <stdbool.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_meta_data_node;
struct sail_palette;
struct sail_variant;

struct gif_tuning_state
{
    int* transparency_index_save;
    int* loop_count;
    int* background_color_index;
};

SAIL_HIDDEN sail_status_t gif_private_fetch_comment(const GifByteType* extension,
                                                    struct sail_meta_data_node** meta_data_node);

SAIL_HIDDEN sail_status_t gif_private_fetch_application(const GifByteType* extension,
                                                        struct sail_meta_data_node** meta_data_node);

SAIL_HIDDEN sail_status_t gif_private_pixel_format_to_bpp(enum SailPixelFormat pixel_format, int* bpp);

SAIL_HIDDEN sail_status_t gif_private_build_color_map(const struct sail_palette* palette,
                                                      ColorMapObject** color_map,
                                                      int* auto_transparency_index);

SAIL_HIDDEN sail_status_t gif_private_convert_rgba_palette_to_rgb(const struct sail_palette* source_palette,
                                                                  struct sail_palette** target_palette,
                                                                  int* transparency_index);

SAIL_HIDDEN bool gif_private_tuning_key_value_callback(const char* key,
                                                       const struct sail_variant* value,
                                                       void* user_data);
