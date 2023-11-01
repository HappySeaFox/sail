/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#ifndef SAIL_JPEGXL_HELPERS_H
#define SAIL_JPEGXL_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include <jxl/decode.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_hash_map;
struct sail_iccp;
struct sail_io;
struct sail_meta_data_node;

SAIL_HIDDEN bool jpegxl_private_is_cmyk(JxlDecoder *decoder, uint32_t num_extra_channels);

SAIL_HIDDEN enum SailPixelFormat jpegxl_private_source_pixel_format_cmyk(uint32_t bits_per_sample, uint32_t alpha_bits);

SAIL_HIDDEN enum SailPixelFormat jpegxl_private_source_pixel_format(uint32_t bits_per_sample, uint32_t num_color_channels, uint32_t alpha_bits);

SAIL_HIDDEN enum SailPixelFormat jpegxl_private_source_pixel_format_to_output(enum SailPixelFormat pixel_format);

SAIL_HIDDEN unsigned jpegxl_private_pixel_format_to_num_channels(enum SailPixelFormat pixel_format);

SAIL_HIDDEN JxlDataType jpegxl_private_pixel_format_to_jxl_data_type(enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_status_t jpegxl_private_fetch_iccp(JxlDecoder *decoder, struct sail_iccp **iccp);

SAIL_HIDDEN sail_status_t jpegxl_private_read_more_data(struct sail_io *io, JxlDecoder *decoder, unsigned char *buffer, size_t buffer_size);

SAIL_HIDDEN sail_status_t jpegxl_private_fetch_special_properties(const JxlBasicInfo *basic_info, struct sail_hash_map *special_properties);

SAIL_HIDDEN sail_status_t jpegxl_private_fetch_name(JxlDecoder *decoder, uint32_t name_length, struct sail_meta_data_node **meta_data_node);

SAIL_HIDDEN sail_status_t jpegxl_private_fetch_metadata(JxlDecoder *decoder, struct sail_meta_data_node **meta_data_node);

#endif
