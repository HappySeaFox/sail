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

#include <cstring>
#include <memory>
#include <string>

#include <Imath/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

using namespace OPENEXR_IMF_INTERNAL_NAMESPACE;

/*
 * Codec-specific state.
 */
struct openexr_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;
    struct sail_io* io;

    std::unique_ptr<InputFile> input_file;
    std::unique_ptr<OutputFile> output_file;

    std::string temp_path_read;
    std::string temp_path_write;

    bool frame_processed;

    unsigned width;
    unsigned height;

    sail::openexr::ChannelInfo channel_info;
};

static sail_status_t alloc_openexr_state(struct sail_io* io,
                                         const struct sail_load_options* load_options,
                                         const struct sail_save_options* save_options,
                                         openexr_state** state)
{
    *state = new openexr_state{
        load_options,                  // load_options
        save_options,                  // save_options
        io,                            // io
        std::unique_ptr<InputFile>(),  // input_file
        std::unique_ptr<OutputFile>(), // output_file
        {},                            // temp_path_read
        {},                            // temp_path_write
        false,                         // frame_processed
        0,                             // width
        0,                             // height
        {}                             // channel_info
    };

    return SAIL_OK;
}

static void destroy_openexr_state(openexr_state* state)
{
    if (state == nullptr)
    {
        return;
    }

    if (!state->temp_path_read.empty())
    {
        remove(state->temp_path_read.c_str());
    }

    if (!state->temp_path_write.empty())
    {
        remove(state->temp_path_write.c_str());
    }

    delete state;
}

/*
 * Decoding functions.
 */

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_init_v8_openexr(struct sail_io* io,
                                                                     const struct sail_load_options* load_options,
                                                                     void** state)
{
    *state = NULL;

    struct openexr_state* openexr_state;
    SAIL_TRY(alloc_openexr_state(io, load_options, NULL, &openexr_state));
    *state = openexr_state;

    /* Create temporary file from I/O. */
    try
    {
        openexr_state->temp_path_read = sail::openexr::create_temp_file_from_io(io);
        openexr_state->input_file.reset(new InputFile(openexr_state->temp_path_read.c_str()));
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to open input file: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_openexr(void* state, struct sail_image** image)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(state);

    if (openexr_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Get image dimensions and channels from the file. */
    const Header& header            = openexr_state->input_file->header();
    const Imath::Box2i& data_window = header.dataWindow();
    openexr_state->width            = static_cast<unsigned>(data_window.max.x - data_window.min.x + 1);
    openexr_state->height           = static_cast<unsigned>(data_window.max.y - data_window.min.y + 1);

    /* Analyze channels. */
    try
    {
        openexr_state->channel_info = sail::openexr::analyze_channels(header.channels());
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to analyze channels: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }

    /* Determine pixel format. */
    const SailPixelFormat pixel_format = sail::openexr::determine_pixel_format(openexr_state->channel_info);
    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_ERROR("OpenEXR: Unsupported channel configuration");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Allocate image structure (SAIL will allocate pixels). */
    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width          = openexr_state->width;
    image_local->height         = openexr_state->height;
    image_local->pixel_format   = pixel_format;
    image_local->bytes_per_line = sail_bytes_per_line(openexr_state->width, pixel_format);

    /* Fill source image info if requested. */
    if (openexr_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression  = sail::openexr::compression_to_sail(header.compression());
    }

    *image = image_local;

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_openexr(void* state, struct sail_image* image)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(state);

    /* Setup FrameBuffer and read pixels. */
    try
    {
        const Header& header            = openexr_state->input_file->header();
        const Imath::Box2i& data_window = header.dataWindow();

        // Special handling for YCbCr
        if (openexr_state->channel_info.has_y && openexr_state->channel_info.has_ry
            && openexr_state->channel_info.has_by)
        {
            sail::openexr::read_ycbcr_and_convert(*openexr_state->input_file, openexr_state->channel_info,
                                                  image->pixels, openexr_state->width, openexr_state->height,
                                                  data_window);
        }
        else
        {
            FrameBuffer frameBuffer;
            sail::openexr::setup_framebuffer_read(frameBuffer, openexr_state->channel_info, image->pixels,
                                                  openexr_state->width, openexr_state->height, data_window);

            openexr_state->input_file->setFrameBuffer(frameBuffer);
            openexr_state->input_file->readPixels(data_window.min.y, data_window.max.y);
        }
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to read pixels: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    openexr_state->frame_processed = true;

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_openexr(void** state)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(*state);

    *state = nullptr;

    openexr_state->input_file.reset();

    destroy_openexr_state(openexr_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_init_v8_openexr(struct sail_io* io,
                                                                     const struct sail_save_options* save_options,
                                                                     void** state)
{
    *state = NULL;

    struct openexr_state* openexr_state;
    SAIL_TRY(alloc_openexr_state(io, NULL, save_options, &openexr_state));
    *state = openexr_state;

    /* Create temporary file for writing. */
    try
    {
        char* path_c = nullptr;
        const sail_status_t status = sail_temp_file_path("sail_exr_write", &path_c);

        if (status != SAIL_OK)
        {
            SAIL_LOG_ERROR("OpenEXR: Failed to create temporary file");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
        }

        openexr_state->temp_path_write = path_c;
        sail_free(path_c);
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to create temporary file: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_openexr(void* state,
                                                                                const struct sail_image* image)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(state);

    if (openexr_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Verify pixel format is supported. */
    const auto [pixel_type, channel_count] = sail::openexr::sail_to_pixel_type(image->pixel_format);
    (void)pixel_type;
    (void)channel_count;

    openexr_state->width  = image->width;
    openexr_state->height = image->height;

    /* Create OpenEXR file for writing using General API. */
    try
    {
        Header header(static_cast<int>(image->width), static_cast<int>(image->height));

        sail::openexr::setup_header_write(header, image->pixel_format, static_cast<int>(image->width),
                                          static_cast<int>(image->height), openexr_state->save_options->compression);

        openexr_state->output_file.reset(new OutputFile(openexr_state->temp_path_write.c_str(), header));

        /* Analyze the format we're writing. */
        openexr_state->channel_info = sail::openexr::analyze_channels(header.channels());
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to open output file: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_openexr(void* state, const struct sail_image* image)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(state);

    /* Setup FrameBuffer and write pixels. */
    try
    {
        FrameBuffer frameBuffer;
        const Imath::Box2i data_window(
            Imath::V2i(0, 0), Imath::V2i(static_cast<int>(image->width) - 1, static_cast<int>(image->height) - 1));

        sail::openexr::setup_framebuffer_read(frameBuffer, openexr_state->channel_info, image->pixels,
                                              image->width, image->height, data_window);

        openexr_state->output_file->setFrameBuffer(frameBuffer);
        openexr_state->output_file->writePixels(static_cast<int>(image->height));
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_ERROR("OpenEXR: Failed to write pixels: %s", e.what());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    openexr_state->frame_processed = true;

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_openexr(void** state)
{
    struct openexr_state* openexr_state = static_cast<struct openexr_state*>(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = nullptr;

    openexr_state->output_file.reset();

    /* Copy temporary file back to I/O stream. */
    if (!openexr_state->temp_path_write.empty())
    {
        FILE* temp_file = fopen(openexr_state->temp_path_write.c_str(), "rb");
        if (temp_file != nullptr)
        {
            SAIL_TRY(openexr_state->io->seek(openexr_state->io->stream, 0, SEEK_SET));

            unsigned char buffer[8192];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), temp_file)) > 0)
            {
                SAIL_TRY(openexr_state->io->strict_write(openexr_state->io->stream, buffer, bytes_read));
            }

            fclose(temp_file);
        }
    }

    destroy_openexr_state(openexr_state);

    return SAIL_OK;
}
