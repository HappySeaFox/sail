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

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <Imath/ImathBox.h>
#include <Imath/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfPixelType.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

using namespace OPENEXR_IMF_INTERNAL_NAMESPACE;

namespace sail::openexr
{

SailPixelFormat pixel_type_to_sail(int pixel_type, int channel_count)
{
    switch (pixel_type)
    {
    case OPENEXR_IMF_INTERNAL_NAMESPACE::HALF:
        switch (channel_count)
        {
        case 1: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF;
        case 2: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA_HALF;
        case 3: return SAIL_PIXEL_FORMAT_BPP48_RGB_HALF;
        case 4: return SAIL_PIXEL_FORMAT_BPP64_RGBA_HALF;
        }
        break;

    case OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT:
        switch (channel_count)
        {
        case 1: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT;
        case 2: return SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_FLOAT;
        case 3: return SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT;
        case 4: return SAIL_PIXEL_FORMAT_BPP128_RGBA_FLOAT;
        }
        break;

    case OPENEXR_IMF_INTERNAL_NAMESPACE::UINT:
        switch (channel_count)
        {
        case 1: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_UINT;
        case 2: return SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_UINT;
        case 3: return SAIL_PIXEL_FORMAT_BPP96_RGB_UINT;
        case 4: return SAIL_PIXEL_FORMAT_BPP128_RGBA_UINT;
        }
        break;
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

std::tuple<int, int> sail_to_pixel_type(SailPixelFormat pixel_format)
{
    switch (pixel_format)
    {
    // HALF formats
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF:       return {OPENEXR_IMF_INTERNAL_NAMESPACE::HALF, 1};
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA_HALF: return {OPENEXR_IMF_INTERNAL_NAMESPACE::HALF, 2};
    case SAIL_PIXEL_FORMAT_BPP48_RGB_HALF:             return {OPENEXR_IMF_INTERNAL_NAMESPACE::HALF, 3};
    case SAIL_PIXEL_FORMAT_BPP64_RGBA_HALF:            return {OPENEXR_IMF_INTERNAL_NAMESPACE::HALF, 4};

    // FLOAT formats
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT:       return {OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT, 1};
    case SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_FLOAT: return {OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT, 2};
    case SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT:             return {OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT, 3};
    case SAIL_PIXEL_FORMAT_BPP128_RGBA_FLOAT:           return {OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT, 4};

    // UINT formats
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_UINT:       return {OPENEXR_IMF_INTERNAL_NAMESPACE::UINT, 1};
    case SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_UINT: return {OPENEXR_IMF_INTERNAL_NAMESPACE::UINT, 2};
    case SAIL_PIXEL_FORMAT_BPP96_RGB_UINT:             return {OPENEXR_IMF_INTERNAL_NAMESPACE::UINT, 3};
    case SAIL_PIXEL_FORMAT_BPP128_RGBA_UINT:           return {OPENEXR_IMF_INTERNAL_NAMESPACE::UINT, 4};

    default: throw std::runtime_error("Unsupported pixel format");
    }
}

SailCompression compression_to_sail(int compression)
{
    switch (compression)
    {
    case NO_COMPRESSION:    return SAIL_COMPRESSION_NONE;
    case RLE_COMPRESSION:   return SAIL_COMPRESSION_RLE;
    case ZIPS_COMPRESSION:  return SAIL_COMPRESSION_ZIPS;
    case ZIP_COMPRESSION:   return SAIL_COMPRESSION_ZIP;
    case PIZ_COMPRESSION:   return SAIL_COMPRESSION_PIZ;
    case PXR24_COMPRESSION: return SAIL_COMPRESSION_PXR24;
    case B44_COMPRESSION:   return SAIL_COMPRESSION_B44;
    case B44A_COMPRESSION:  return SAIL_COMPRESSION_B44A;
    case DWAA_COMPRESSION:  return SAIL_COMPRESSION_DWAA;
    case DWAB_COMPRESSION:  return SAIL_COMPRESSION_DWAB;
    default: return SAIL_COMPRESSION_UNKNOWN;
    }
}

int sail_compression_to_exr(SailCompression compression)
{
    switch (compression)
    {
    case SAIL_COMPRESSION_NONE: return NO_COMPRESSION;
    case SAIL_COMPRESSION_RLE: return RLE_COMPRESSION;
    case SAIL_COMPRESSION_ZIPS: return ZIPS_COMPRESSION;
    case SAIL_COMPRESSION_ZIP: return ZIP_COMPRESSION;
    case SAIL_COMPRESSION_PIZ: return PIZ_COMPRESSION;
    case SAIL_COMPRESSION_PXR24: return PXR24_COMPRESSION;
    case SAIL_COMPRESSION_B44: return B44_COMPRESSION;
    case SAIL_COMPRESSION_B44A: return B44A_COMPRESSION;
    case SAIL_COMPRESSION_DWAA: return DWAA_COMPRESSION;
    case SAIL_COMPRESSION_DWAB: return DWAB_COMPRESSION;
    default: return ZIP_COMPRESSION;
    }
}

const char* compression_to_string(int compression)
{
    switch (compression)
    {
    case NO_COMPRESSION: return "NONE";
    case RLE_COMPRESSION: return "RLE";
    case ZIPS_COMPRESSION: return "ZIPS";
    case ZIP_COMPRESSION: return "ZIP";
    case PIZ_COMPRESSION: return "PIZ";
    case PXR24_COMPRESSION: return "PXR24";
    case B44_COMPRESSION: return "B44";
    case B44A_COMPRESSION: return "B44A";
    case DWAA_COMPRESSION: return "DWAA";
    case DWAB_COMPRESSION: return "DWAB";
    default: return "UNKNOWN";
    }
}

std::string create_temp_file_from_io(sail_io* io)
{
    auto path = []() {
        char* path_c = nullptr;

        SAIL_TRY_OR_EXECUTE(sail_temp_file_path("sail_exr", &path_c),
                            /* on error */ throw std::runtime_error("Failed to create temporary file"));

        std::string path{path_c};
        sail_free(path_c);
        return path;
    }();

    /* Open the file */
#ifdef _WIN32
    int fd;
    if (_sopen_s(&fd, path.c_str(), _O_RDWR | _O_BINARY, _SH_DENYRW, _S_IREAD | _S_IWRITE) != 0)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to open temporary file");
        throw std::runtime_error("Failed to open temporary file");
    }
#else
    const int fd = open(path.c_str(), O_RDWR);

    if (fd < 0)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to open temporary file");
        throw std::runtime_error("Failed to open temporary file");
    }
#endif

    // Copy data from SAIL I/O to the temp file
    if (io->seek(io->stream, 0, SEEK_SET) != SAIL_OK)
    {
#ifdef _MSC_VER
        _close(fd);
#else
        close(fd);
#endif
        remove(path.c_str());
        throw std::runtime_error("Failed to seek I/O stream");
    }

    std::array<unsigned char, 8192> buffer{};
    size_t bytes_read;
    sail_status_t err;

    while ((err = io->tolerant_read(io->stream, buffer.data(), buffer.size(), &bytes_read)) == SAIL_OK
           && bytes_read > 0)
    {
#ifdef _WIN32
        if (_write(fd, buffer.data(), static_cast<unsigned int>(bytes_read)) != static_cast<int>(bytes_read))
#else
        if (write(fd, buffer.data(), bytes_read) != static_cast<ssize_t>(bytes_read))
#endif
        {
#ifdef _WIN32
            _close(fd);
#else
            close(fd);
#endif
            remove(path.c_str());
            SAIL_LOG_ERROR("OpenEXR: Failed to write to temporary file");
            throw std::runtime_error("Failed to write to temporary file");
        }
    }

#ifdef _WIN32
    _close(fd);
#else
    close(fd);
#endif

    // EOF is expected when reaching the end of the stream
    if (err != SAIL_OK && err != SAIL_ERROR_EOF)
    {
        remove(path.c_str());
        throw std::runtime_error("Failed to read from I/O stream");
    }

    return path;
}

ChannelInfo analyze_channels(const ChannelList& channels)
{
    ChannelInfo info{};
    info.num_channels = 0;

    // Check for standard channels
    auto* y_channel  = channels.findChannel("Y");
    auto* r_channel  = channels.findChannel("R");
    auto* g_channel  = channels.findChannel("G");
    auto* b_channel  = channels.findChannel("B");
    auto* ry_channel = channels.findChannel("RY");
    auto* by_channel = channels.findChannel("BY");
    auto* a_channel  = channels.findChannel("A");

    info.has_y  = (y_channel != nullptr);
    info.has_r  = (r_channel != nullptr);
    info.has_g  = (g_channel != nullptr);
    info.has_b  = (b_channel != nullptr);
    info.has_ry = (ry_channel != nullptr);
    info.has_by = (by_channel != nullptr);
    info.has_a  = (a_channel != nullptr);

    // Store subsampling factors for chroma channels
    if (info.has_ry)
    {
        info.ry_xsampling = ry_channel->xSampling;
        info.ry_ysampling = ry_channel->ySampling;
    }
    else
    {
        info.ry_xsampling = 1;
        info.ry_ysampling = 1;
    }

    if (info.has_by)
    {
        info.by_xsampling = by_channel->xSampling;
        info.by_ysampling = by_channel->ySampling;
    }
    else
    {
        info.by_xsampling = 1;
        info.by_ysampling = 1;
    }

    // Determine pixel type (use first available channel)
    const Channel* first_channel = nullptr;

    if (info.has_y && info.has_ry && info.has_by)
    {
        first_channel     = y_channel;
        info.num_channels = 3; // Will be converted to RGB
    }
    else if (info.has_y && !info.has_ry && !info.has_by)
    {
        first_channel     = y_channel;
        info.num_channels = 1;
    }
    else if (info.has_r && info.has_g && info.has_b)
    {
        first_channel     = r_channel;
        info.num_channels = 3;
    }

    if (info.has_a)
    {
        info.num_channels++;
        if (!first_channel)
        {
            first_channel = a_channel;
        }
    }

    if (first_channel)
    {
        info.type = first_channel->type;
    }
    else
    {
        // Fallback: iterate through all channels
        auto it = channels.begin();
        if (it != channels.end())
        {
            info.type         = it.channel().type;
            info.num_channels = 1;
        }
        else
        {
            throw std::runtime_error("No channels found in OpenEXR file");
        }
    }

    return info;
}

SailPixelFormat determine_pixel_format(const ChannelInfo& info)
{
    return pixel_type_to_sail(static_cast<int>(info.type), info.num_channels);
}

size_t bytes_per_pixel(const ChannelInfo& info)
{
    size_t bytes_per_channel = 0;

    switch (info.type)
    {
    case OPENEXR_IMF_INTERNAL_NAMESPACE::HALF:  bytes_per_channel = 2; break;
    case OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT: bytes_per_channel = 4; break;
    case OPENEXR_IMF_INTERNAL_NAMESPACE::UINT:  bytes_per_channel = 4; break;
    default: bytes_per_channel = 2; break;
    }

    return bytes_per_channel * info.num_channels;
}

void setup_framebuffer_read(
    FrameBuffer& fb, const ChannelInfo& info, void* pixels, unsigned width, unsigned height, const Imath::Box2i& data_window)
{
    (void)height; // May be unused

    char* base              = static_cast<char*>(pixels);
    const size_t pixel_size = bytes_per_pixel(info);
    const size_t x_stride   = pixel_size;
    const size_t y_stride   = width * pixel_size;

    // Adjust base pointer for data window
    base = base - data_window.min.x * x_stride - data_window.min.y * y_stride;

    if (info.has_y && info.has_ry && info.has_by)
    {
        throw std::runtime_error("YCbCr should be handled via read_ycbcr_and_convert()");
    }
    else if (info.has_y && !info.has_ry && !info.has_by)
    {
        fb.insert("Y", Slice(info.type, base, x_stride, y_stride));

        if (info.has_a)
        {
            const size_t bytes_per_channel = (info.type == HALF) ? 2 : 4;
            fb.insert("A", Slice(info.type, base + bytes_per_channel, x_stride, y_stride));
        }
    }
    else if (info.has_r && info.has_g && info.has_b)
    {
        const size_t bytes_per_channel = (info.type == HALF) ? 2 : 4;
        fb.insert("R", Slice(info.type, base + 0 * bytes_per_channel, x_stride, y_stride));
        fb.insert("G", Slice(info.type, base + 1 * bytes_per_channel, x_stride, y_stride));
        fb.insert("B", Slice(info.type, base + 2 * bytes_per_channel, x_stride, y_stride));

        if (info.has_a)
        {
            fb.insert("A", Slice(info.type, base + 3 * bytes_per_channel, x_stride, y_stride));
        }
    }
    else
    {
        throw std::runtime_error("Unsupported channel configuration in OpenEXR file");
    }
}

void read_ycbcr_and_convert(InputFile& file,
                             const ChannelInfo& info,
                             void* pixels,
                             unsigned width,
                             unsigned height,
                             const Imath::Box2i& data_window)
{
    if (info.type != HALF)
    {
        throw std::runtime_error("Only HALF YCbCr is currently supported");
    }

    // Calculate subsampled dimensions
    const auto chroma_width = (width + info.ry_xsampling - 1) / info.ry_xsampling;
    const auto chroma_height = (height + info.ry_ysampling - 1) / info.ry_ysampling;

    // Allocate temporary buffers for Y, RY, BY channels
    std::vector<Imath::half> y_buffer(width * height);
    std::vector<Imath::half> ry_buffer(chroma_width * chroma_height);
    std::vector<Imath::half> by_buffer(chroma_width * chroma_height);

    auto* y_base = reinterpret_cast<char*>(y_buffer.data());
    auto* ry_base = reinterpret_cast<char*>(ry_buffer.data());
    auto* by_base = reinterpret_cast<char*>(by_buffer.data());

    y_base = y_base - data_window.min.x * sizeof(Imath::half) - data_window.min.y * width * sizeof(Imath::half);
    ry_base = ry_base - (data_window.min.x / info.ry_xsampling) * sizeof(Imath::half)
            - (data_window.min.y / info.ry_ysampling) * chroma_width * sizeof(Imath::half);
    by_base = by_base - (data_window.min.x / info.by_xsampling) * sizeof(Imath::half)
            - (data_window.min.y / info.by_ysampling) * chroma_width * sizeof(Imath::half);

    FrameBuffer fb;
    fb.insert("Y", Slice(HALF, y_base, sizeof(Imath::half), width * sizeof(Imath::half), 1, 1));
    fb.insert("RY", Slice(HALF, ry_base, sizeof(Imath::half), chroma_width * sizeof(Imath::half),
                         info.ry_xsampling, info.ry_ysampling));
    fb.insert("BY", Slice(HALF, by_base, sizeof(Imath::half), chroma_width * sizeof(Imath::half),
                         info.by_xsampling, info.by_ysampling));

    file.setFrameBuffer(fb);
    file.readPixels(data_window.min.y, data_window.max.y);

    // Luminance weights
    const auto yw_r = 0.2126f;
    const auto yw_g = 0.7152f;
    const auto yw_b = 0.0722f;

    // Convert YCA to RGB with bilinear upsampling for chroma
    Imath::half* dest = static_cast<Imath::half*>(pixels);

    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            // Get Y value (full resolution)
            auto Y = static_cast<float>(y_buffer[y * width + x]);

            // Get RY and BY with bilinear upsampling
            const auto cx = x / info.ry_xsampling;
            const auto cy = y / info.ry_ysampling;
            const auto cx_next = std::min(cx + 1u, chroma_width - 1);
            const auto cy_next = std::min(cy + 1u, chroma_height - 1);

            // Bilinear interpolation weights
            const auto fx = (x % info.ry_xsampling) / static_cast<float>(info.ry_xsampling);
            const auto fy = (y % info.ry_ysampling) / static_cast<float>(info.ry_ysampling);

            // Interpolate RY
            auto ry00 = static_cast<float>(ry_buffer[cy * chroma_width + cx]);
            auto ry10 = static_cast<float>(ry_buffer[cy * chroma_width + cx_next]);
            auto ry01 = static_cast<float>(ry_buffer[cy_next * chroma_width + cx]);
            auto ry11 = static_cast<float>(ry_buffer[cy_next * chroma_width + cx_next]);
            auto RY = ry00 * (1 - fx) * (1 - fy) + ry10 * fx * (1 - fy) +
                       ry01 * (1 - fx) * fy + ry11 * fx * fy;

            // Interpolate BY
            auto by00 = static_cast<float>(by_buffer[cy * chroma_width + cx]);
            auto by10 = static_cast<float>(by_buffer[cy * chroma_width + cx_next]);
            auto by01 = static_cast<float>(by_buffer[cy_next * chroma_width + cx]);
            auto by11 = static_cast<float>(by_buffer[cy_next * chroma_width + cx_next]);
            auto BY = by00 * (1 - fx) * (1 - fy) + by10 * fx * (1 - fy) +
                       by01 * (1 - fx) * fy + by11 * fx * fy;

            float r, g, b;
            if (RY == 0.0f && BY == 0.0f)
            {
                r = g = b = Y;
            }
            else
            {
                r = (RY + 1.0f) * Y;
                b = (BY + 1.0f) * Y;
                g = (Y - r * yw_r - b * yw_b) / yw_g;
            }

            *dest++ = Imath::half(r);
            *dest++ = Imath::half(g);
            *dest++ = Imath::half(b);

            if (info.has_a)
            {
                *dest++ = Imath::half(1.0f);
            }
        }
    }
}


void setup_header_write(
    Header& header, SailPixelFormat pixel_format, int width, int height, SailCompression compression)
{
    (void)width;
    (void)height;

    const auto [pixel_type, channel_count] = sail_to_pixel_type(pixel_format);
    const auto exr_pixel_type              = static_cast<PixelType>(pixel_type);

    // Add channels based on format
    if (pixel_format == SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF
        || pixel_format == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT
        || pixel_format == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_UINT
        || pixel_format == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA_HALF
        || pixel_format == SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_FLOAT
        || pixel_format == SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_UINT)
    {
        // Grayscale or Grayscale+Alpha
        header.channels().insert("Y", Channel(exr_pixel_type));

        if (channel_count == 2)
        {
            header.channels().insert("A", Channel(exr_pixel_type));
        }
    }
    else
    {
        // RGB or RGBA
        header.channels().insert("R", Channel(exr_pixel_type));
        header.channels().insert("G", Channel(exr_pixel_type));
        header.channels().insert("B", Channel(exr_pixel_type));

        if (channel_count == 4)
        {
            header.channels().insert("A", Channel(exr_pixel_type));
        }
    }

    // Set compression
    header.compression() = static_cast<Compression>(sail_compression_to_exr(compression));
}

} // namespace sail::openexr
