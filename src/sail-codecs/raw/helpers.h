/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <vector>

#include <libraw/libraw.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_meta_data_node;
struct sail_resolution;

SAIL_HIDDEN enum SailPixelFormat raw_private_libraw_to_pixel_format(unsigned colors, unsigned bits);

SAIL_HIDDEN sail_status_t raw_private_fetch_meta_data(libraw_data_t* raw_data,
                                                      struct sail_meta_data_node** meta_data_node,
                                                      const std::vector<unsigned char>& exif_data);

SAIL_HIDDEN sail_status_t raw_private_store_special_properties(libraw_data_t* raw_data,
                                                               struct sail_hash_map* special_properties);

SAIL_HIDDEN bool raw_private_tuning_key_value_callback(const char* key,
                                                       const struct sail_variant* value,
                                                       void* user_data);
