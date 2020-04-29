/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN image_writer::pimpl
{
public:
    pimpl()
        : state(nullptr)
    {
    }

    context *ctx;
    bool own_context;
    void *state;
    struct sail_io sail_io;
};

image_writer::image_writer()
    : d(new pimpl)
{
    d->ctx = new context;
    d->own_context = true;
}

image_writer::image_writer(context *ctx)
    : d(new pimpl)
{
    d->ctx = ctx;
    d->own_context = false;

    if (d->ctx == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to image_writer()");
    }
}

image_writer::~image_writer()
{
    stop_writing();

    if (d->own_context) {
        delete d->ctx;
    }

    delete d;
}

bool image_writer::is_valid() const
{
    return d->ctx != nullptr && d->ctx->is_valid();
}

sail_error_t image_writer::write(const std::string &path, const image &simage)
{
    SAIL_TRY(write(path.c_str(), simage));

    return 0;
}

sail_error_t image_writer::write(const char *path, const image &simage)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_image *sail_image;
    SAIL_TRY(sail_alloc_image(&sail_image));

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(simage.to_sail_image(sail_image));

    const void *bits = simage.bits() ? simage.bits() : simage.shallow_bits();

    SAIL_TRY(sail_write(path,
                        sail_image,
                        bits));

    return 0;
}

sail_error_t image_writer::start_writing(const std::string &path)
{
    SAIL_TRY(start_writing(path.c_str()));

    return 0;
}

sail_error_t image_writer::start_writing(const char *path)
{
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_writing_file(path, d->ctx->sail_context_c(), nullptr, &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(const std::string &path, const plugin_info &splugin_info)
{
    SAIL_TRY(start_writing(path.c_str(), splugin_info));

    return 0;
}

sail_error_t image_writer::start_writing(const char *path, const plugin_info &splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_writing_file(path, d->ctx->sail_context_c(), splugin_info.sail_plugin_info_c(), &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(const std::string &path, const write_options &swrite_options)
{
    SAIL_TRY(start_writing(path.c_str(), swrite_options));

    return 0;
}

sail_error_t image_writer::start_writing(const char *path, const write_options &swrite_options)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_file_with_options(path, d->ctx->sail_context_c(), nullptr, &sail_write_options, &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(const std::string &path, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_TRY(start_writing(path.c_str(), splugin_info, swrite_options));

    return 0;
}

sail_error_t image_writer::start_writing(const char *path, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_file_with_options(path, d->ctx->sail_context_c(), splugin_info.sail_plugin_info_c(), &sail_write_options, &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(void *buffer, long buffer_length, const plugin_info &splugin_info)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    SAIL_TRY(sail_start_writing_mem_with_options(buffer,
                                                 buffer_length,
                                                 d->ctx->sail_context_c(),
                                                 splugin_info.sail_plugin_info_c(),
                                                 NULL,
                                                 &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(void *buffer, long buffer_length, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_mem_with_options(buffer,
                                                 buffer_length,
                                                 d->ctx->sail_context_c(),
                                                 splugin_info.sail_plugin_info_c(),
                                                 &sail_write_options,
                                                 &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(const io &sio, const plugin_info &splugin_info)
{
    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    sail_io *sail_io = &d->sail_io;
    SAIL_CHECK_IO(sail_io);

    SAIL_TRY(sail_start_writing_io_with_options(&d->sail_io,
                                                d->ctx->sail_context_c(),
                                                splugin_info.sail_plugin_info_c(),
                                                NULL,
                                                &d->state));

    return 0;
}

sail_error_t image_writer::start_writing(const io &sio, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    sail_io *sail_io = &d->sail_io;
    SAIL_CHECK_IO(sail_io);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_io_with_options(&d->sail_io,
                                                d->ctx->sail_context_c(),
                                                splugin_info.sail_plugin_info_c(),
                                                &sail_write_options,
                                                &d->state));

    return 0;
}

sail_error_t image_writer::write_next_frame(const image &simage)
{
    sail_image *sail_image;
    SAIL_TRY(sail_alloc_image(&sail_image));

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(simage.to_sail_image(sail_image));

    const void *bits = simage.bits() ? simage.bits() : simage.shallow_bits();

    SAIL_TRY(sail_write_next_frame(d->state, sail_image, bits));

    return 0;
}

sail_error_t image_writer::stop_writing()
{
    SAIL_TRY(sail_stop_writing(d->state));

    d->state = nullptr;

    return 0;
}

}
