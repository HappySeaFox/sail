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

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <Imath/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
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
    case HALF:
        switch (channel_count)
        {
        case 1: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF;
        case 2: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA_HALF;
        case 3: return SAIL_PIXEL_FORMAT_BPP48_RGB_HALF;
        case 4: return SAIL_PIXEL_FORMAT_BPP64_RGBA_HALF;
        }
        break;

    case FLOAT:
        switch (channel_count)
        {
        case 1: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT;
        case 2: return SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_FLOAT;
        case 3: return SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT;
        case 4: return SAIL_PIXEL_FORMAT_BPP128_RGBA_FLOAT;
        }
        break;

    case UINT:
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
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_HALF: return {HALF, 1};
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA_HALF: return {HALF, 2};
    case SAIL_PIXEL_FORMAT_BPP48_RGB_HALF: return {HALF, 3};
    case SAIL_PIXEL_FORMAT_BPP64_RGBA_HALF: return {HALF, 4};

    // FLOAT formats
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT: return {FLOAT, 1};
    case SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_FLOAT: return {FLOAT, 2};
    case SAIL_PIXEL_FORMAT_BPP96_RGB_FLOAT: return {FLOAT, 3};
    case SAIL_PIXEL_FORMAT_BPP128_RGBA_FLOAT: return {FLOAT, 4};

    // UINT formats
    case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_UINT: return {UINT, 1};
    case SAIL_PIXEL_FORMAT_BPP64_GRAYSCALE_ALPHA_UINT: return {UINT, 2};
    case SAIL_PIXEL_FORMAT_BPP96_RGB_UINT: return {UINT, 3};
    case SAIL_PIXEL_FORMAT_BPP128_RGBA_UINT: return {UINT, 4};

    default: throw std::runtime_error("Unsupported pixel format");
    }
}

SailCompression compression_to_sail(int compression)
{
    switch (compression)
    {
    case NO_COMPRESSION: return SAIL_COMPRESSION_NONE;
    case RLE_COMPRESSION: return SAIL_COMPRESSION_RLE;
    case ZIPS_COMPRESSION: return SAIL_COMPRESSION_ZIPS;
    case ZIP_COMPRESSION: return SAIL_COMPRESSION_ZIP;
    case PIZ_COMPRESSION: return SAIL_COMPRESSION_PIZ;
    case PXR24_COMPRESSION: return SAIL_COMPRESSION_PXR24;
    case B44_COMPRESSION: return SAIL_COMPRESSION_B44;
    case B44A_COMPRESSION: return SAIL_COMPRESSION_B44A;
    case DWAA_COMPRESSION: return SAIL_COMPRESSION_DWAA;
    case DWAB_COMPRESSION: return SAIL_COMPRESSION_DWAB;
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

std::tuple<std::string, int> create_temp_file(const std::string& prefix)
{
    std::array<char, 512> path_template{};

#ifdef _WIN32
    std::array<char, MAX_PATH> tmpdir{};
    const auto len = GetTempPathA(static_cast<DWORD>(tmpdir.size()), tmpdir.data());
    if (len == 0 || len > tmpdir.size())
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to get temp directory on Windows");
        throw std::runtime_error("Failed to get temp directory");
    }

    snprintf(path_template.data(), path_template.size(), "%s%s_XXXXXXXX", tmpdir.data(), prefix.c_str());

    if (_mktemp_s(path_template.data(), path_template.size()) != 0)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to create temporary filename");
        throw std::runtime_error("Failed to create temporary filename");
    }

    int fd;
    if (_sopen_s(&fd, path_template.data(), _O_RDWR | _O_CREAT | _O_EXCL | _O_TEMPORARY | _O_BINARY, _SH_DENYRW,
                 _S_IREAD | _S_IWRITE)
        != 0)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to create temporary file");
        throw std::runtime_error("Failed to create temporary file");
    }

    return {std::string(path_template.data()), fd};
#else
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir == nullptr || tmpdir[0] == '\0')
    {
#ifdef P_tmpdir
        tmpdir = P_tmpdir;
#else
        tmpdir = "/tmp";
#endif
    }

    snprintf(path_template.data(), path_template.size(), "%s/%s_XXXXXXXX", tmpdir, prefix.c_str());
    const int fd = mkstemp(path_template.data());

    if (fd < 0)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to create temporary file");
        throw std::runtime_error("Failed to create temporary file");
    }

    return {std::string(path_template.data()), fd};
#endif
}

std::string create_temp_file_from_io(sail_io* io)
{
    auto [path, fd] = create_temp_file("sail_exr");

    // Copy data from SAIL I/O to the temp file
    if (io->seek(io->stream, 0, SEEK_SET) != SAIL_OK)
    {
#ifdef _WIN32
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
    const Channel* y_channel = channels.findChannel("Y");
    const Channel* r_channel = channels.findChannel("R");
    const Channel* g_channel = channels.findChannel("G");
    const Channel* b_channel = channels.findChannel("B");
    const Channel* a_channel = channels.findChannel("A");

    info.has_y = (y_channel != nullptr);
    info.has_r = (r_channel != nullptr);
    info.has_g = (g_channel != nullptr);
    info.has_b = (b_channel != nullptr);
    info.has_a = (a_channel != nullptr);

    // Determine pixel type (use first available channel)
    const Channel* first_channel = nullptr;
    if (info.has_y)
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
    case HALF: bytes_per_channel = 2; break;
    case FLOAT: bytes_per_channel = 4; break;
    case UINT: bytes_per_channel = 4; break;
    default: bytes_per_channel = 2; break;
    }

    return bytes_per_channel * info.num_channels;
}

void setup_framebuffer_read(
    FrameBuffer& fb, const ChannelInfo& info, void* pixels, int width, int height, const Imath::Box2i& data_window)
{
    (void)height; // May be unused

    char* base              = static_cast<char*>(pixels);
    const size_t pixel_size = bytes_per_pixel(info);
    const size_t x_stride   = pixel_size;
    const size_t y_stride   = width * pixel_size;

    // Adjust base pointer for data window
    base = base - data_window.min.x * x_stride - data_window.min.y * y_stride;

    if (info.has_y)
    {
        // Grayscale
        fb.insert("Y", Slice(info.type, base, x_stride, y_stride));

        if (info.has_a)
        {
            // Grayscale + Alpha
            const size_t bytes_per_channel = (info.type == HALF) ? 2 : 4;
            fb.insert("A", Slice(info.type, base + bytes_per_channel, x_stride, y_stride));
        }
    }
    else if (info.has_r && info.has_g && info.has_b)
    {
        // RGB
        const size_t bytes_per_channel = (info.type == HALF) ? 2 : 4;
        fb.insert("R", Slice(info.type, base + 0 * bytes_per_channel, x_stride, y_stride));
        fb.insert("G", Slice(info.type, base + 1 * bytes_per_channel, x_stride, y_stride));
        fb.insert("B", Slice(info.type, base + 2 * bytes_per_channel, x_stride, y_stride));

        if (info.has_a)
        {
            // RGBA
            fb.insert("A", Slice(info.type, base + 3 * bytes_per_channel, x_stride, y_stride));
        }
    }
    else
    {
        throw std::runtime_error("Unsupported channel configuration in OpenEXR file");
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
