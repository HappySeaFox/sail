/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN image_reader::pimpl
{
public:
    pimpl()
        : ctx(nullptr)
        , own_context(false)
        , state(nullptr)
    {
    }

    context *ctx;
    bool own_context;
    void *state;
    struct sail_io sail_io;
};

image_reader::image_reader()
    : d(new pimpl)
{
    d->ctx = new context;
    d->own_context = true;
}

image_reader::image_reader(context *ctx)
    : d(new pimpl)
{
    d->ctx = ctx;
    d->own_context = false;

    if (d->ctx == nullptr) {
        SAIL_LOG_ERROR("NULL context pointer has been passed to image_reader()");
    }
}

image_reader::~image_reader()
{
    stop_reading();

    if (d->own_context) {
        delete d->ctx;
    }

    delete d;
}

bool image_reader::is_valid() const
{
    return d->ctx != nullptr && d->ctx->status() == 0;
}

sail_error_t image_reader::probe_path(const std::string &path, image *simage, plugin_info *splugin_info)
{
    SAIL_TRY(probe_path(path.c_str(), simage, splugin_info));

    return 0;
}

sail_error_t image_reader::probe_path(const char *path, image *simage, plugin_info *splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_plugin_info *sail_plugin_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_path(path,
                             d->ctx->sail_context_c(),
                             &sail_image,
                             &sail_plugin_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (splugin_info != nullptr) {
        *splugin_info = plugin_info(sail_plugin_info);
    }

    return 0;
}

sail_error_t image_reader::probe_mem(const void *buffer, size_t buffer_length, image *simage, plugin_info *splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_plugin_info *sail_plugin_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_mem(buffer,
                            buffer_length,
                            d->ctx->sail_context_c(),
                            &sail_image,
                            &sail_plugin_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (splugin_info != nullptr) {
        *splugin_info = plugin_info(sail_plugin_info);
    }

    return 0;
}

sail_error_t image_reader::probe_io(const sail::io &io, image *simage, plugin_info *splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_IMAGE_PTR(simage);
    SAIL_TRY(io.verify_valid());

    struct sail_io sail_io;
    SAIL_TRY(io.to_sail_io(&sail_io));

    const sail_plugin_info *sail_plugin_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_io(&sail_io,
                           d->ctx->sail_context_c(),
                           &sail_image,
                           &sail_plugin_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (splugin_info != nullptr) {
        *splugin_info = plugin_info(sail_plugin_info);
    }

    return 0;
}

sail_error_t image_reader::read(const std::string &path, image *simage)
{
    SAIL_TRY(read(path.c_str(), simage));

    return 0;
}

sail_error_t image_reader::read(const char *path, image *simage)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;

    SAIL_TRY(sail_read_path(path,
                            d->ctx->sail_context_c(),
                            &sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return 0;
}

sail_error_t image_reader::read(const void *buffer, size_t buffer_length, image *simage)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;

    SAIL_TRY(sail_read_mem(buffer,
                            buffer_length,
                            d->ctx->sail_context_c(),
                            &sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return 0;
}

sail_error_t image_reader::start_reading(const std::string &path)
{
    SAIL_TRY(start_reading(path.c_str()));

    return 0;
}

sail_error_t image_reader::start_reading(const char *path)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_reading_file(path, d->ctx->sail_context_c(), nullptr, &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const std::string &path, const plugin_info &splugin_info)
{
    SAIL_TRY(start_reading(path.c_str(), splugin_info));

    return 0;
}

sail_error_t image_reader::start_reading(const char *path, const plugin_info &splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_reading_file(path, d->ctx->sail_context_c(), splugin_info.sail_plugin_info_c(), &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const std::string &path, const plugin_info &splugin_info, const read_options &sread_options)
{
    SAIL_TRY(start_reading(path.c_str(), splugin_info, sread_options));

    return 0;
}

sail_error_t image_reader::start_reading(const char *path, const plugin_info &splugin_info, const read_options &sread_options)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_PATH_PTR(path);

    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_file_with_options(path,
                                                  d->ctx->sail_context_c(),
                                                  splugin_info.sail_plugin_info_c(),
                                                  &sail_read_options,
                                                  &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const void *buffer, size_t buffer_length, const plugin_info &splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_BUFFER_PTR(buffer);

    SAIL_TRY(sail_start_reading_mem(buffer,
                                    buffer_length,
                                    d->ctx->sail_context_c(),
                                    splugin_info.sail_plugin_info_c(),
                                    &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const void *buffer, size_t buffer_length, const plugin_info &splugin_info, const read_options &sread_options)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);
    SAIL_CHECK_BUFFER_PTR(buffer);

    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_mem_with_options(buffer,
                                                 buffer_length,
                                                 d->ctx->sail_context_c(),
                                                 splugin_info.sail_plugin_info_c(),
                                                 &sail_read_options,
                                                 &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const io &sio, const plugin_info &splugin_info)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);

    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    sail_io *sail_io = &d->sail_io;
    SAIL_CHECK_IO(sail_io);

    SAIL_TRY(sail_start_reading_io_with_options(&d->sail_io,
                                                d->ctx->sail_context_c(),
                                                splugin_info.sail_plugin_info_c(),
                                                NULL,
                                                &d->state));

    return 0;
}

sail_error_t image_reader::start_reading(const io &sio, const plugin_info &splugin_info, const read_options &sread_options)
{
    SAIL_CHECK_CONTEXT_PTR(d->ctx);

    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    sail_io *sail_io = &d->sail_io;
    SAIL_CHECK_IO(sail_io);

    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_io_with_options(&d->sail_io,
                                                d->ctx->sail_context_c(),
                                                splugin_info.sail_plugin_info_c(),
                                                &sail_read_options,
                                                &d->state));

    return 0;
}

sail_error_t image_reader::read_next_frame(image *simage)
{
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;
    SAIL_TRY(sail_read_next_frame(d->state, &sail_image));

    unsigned bytes_per_image;
    SAIL_TRY_OR_CLEANUP(sail_bytes_per_image(sail_image, &bytes_per_image),
                        /* cleanup */ sail_destroy_image(sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return 0;
}

sail_error_t image_reader::stop_reading()
{
    SAIL_TRY(sail_stop_reading(d->state));

    d->state = nullptr;

    return 0;
}

}
