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

#include <string>
#include <tuple>

#include <sail-common/common.h>
#include <sail-common/status.h>

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfPixelType.h>

struct sail_io;

namespace sail::openexr
{

/*
 * Information about OpenEXR file channels.
 */
struct ChannelInfo
{
    bool has_y;                                     // Grayscale (Y channel)
    bool has_r, has_g, has_b;                       // RGB channels
    bool has_a;                                     // Alpha channel
    OPENEXR_IMF_INTERNAL_NAMESPACE::PixelType type; // HALF, FLOAT, or UINT
    int num_channels;                               // Total number of channels
};

SailPixelFormat pixel_type_to_sail(int pixel_type, int channel_count);

std::tuple<int, int> sail_to_pixel_type(SailPixelFormat pixel_format);

SailCompression compression_to_sail(int compression);

int sail_compression_to_exr(SailCompression compression);

const char* compression_to_string(int compression);

std::tuple<std::string, int> create_temp_file(const std::string& prefix);

std::string create_temp_file_from_io(sail_io* io);

ChannelInfo analyze_channels(const OPENEXR_IMF_INTERNAL_NAMESPACE::ChannelList& channels);

SailPixelFormat determine_pixel_format(const ChannelInfo& info);

void setup_framebuffer_read(OPENEXR_IMF_INTERNAL_NAMESPACE::FrameBuffer& fb,
                            const ChannelInfo& info,
                            void* pixels,
                            int width,
                            int height,
                            const Imath::Box2i& data_window);

void setup_header_write(OPENEXR_IMF_INTERNAL_NAMESPACE::Header& header,
                        SailPixelFormat pixel_format,
                        int width,
                        int height,
                        SailCompression compression);

size_t bytes_per_pixel(const ChannelInfo& info);

} // namespace sail::openexr
