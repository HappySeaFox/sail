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

class SAIL_HIDDEN image_writer::pimpl
{
public:
    pimpl()
        : state(nullptr)
    {
    }

    void *state;
    struct sail_io sail_io;
};

image_writer::image_writer()
    : d(new pimpl)
{
}

image_writer::~image_writer()
{
    stop_writing();
    delete d;
}

sail_status_t image_writer::write(const std::string &path, const image &simage)
{
    SAIL_TRY(write(path.c_str(), simage));

    return SAIL_OK;
}

sail_status_t image_writer::write(const char *path, const image &simage)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_image *sail_image;
    SAIL_TRY(sail_alloc_image(&sail_image));

    SAIL_TRY_OR_CLEANUP(simage.to_sail_image(sail_image),
                        /* cleanup */ sail_image->pixels = NULL,
                                      sail_destroy_image(sail_image));

    SAIL_TRY_OR_CLEANUP(sail_write_file(path,
                                        sail_image),
                                        /* cleanup */ sail_image->pixels = NULL,
                                        sail_destroy_image(sail_image));

    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_writer::write(void *buffer, size_t buffer_length, const image &simage)
{
    SAIL_TRY(write(buffer, buffer_length, simage, nullptr));

    return SAIL_OK;
}

sail_status_t image_writer::write(void *buffer, size_t buffer_length, const image &simage, size_t *written)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    sail_image *sail_image;
    SAIL_TRY(sail_alloc_image(&sail_image));

    SAIL_TRY_OR_CLEANUP(simage.to_sail_image(sail_image),
                        /* cleanup */ sail_image->pixels = NULL,
                                      sail_destroy_image(sail_image));

    SAIL_TRY_OR_CLEANUP(sail_write_mem(buffer,
                                        buffer_length,
                                        sail_image,
                                        written),
                        /* cleanup */ sail_image->pixels = NULL,
                                      sail_destroy_image(sail_image));

    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const std::string &path)
{
    SAIL_TRY(start_writing(path.c_str()));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const char *path)
{
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_writing_file(path, nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const std::string &path, const plugin_info &splugin_info)
{
    SAIL_TRY(start_writing(path.c_str(), splugin_info));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const char *path, const plugin_info &splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);

    SAIL_TRY(sail_start_writing_file(path, splugin_info.sail_plugin_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const std::string &path, const write_options &swrite_options)
{
    SAIL_TRY(start_writing(path.c_str(), swrite_options));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const char *path, const write_options &swrite_options)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_file_with_options(path, nullptr, &sail_write_options, &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const std::string &path, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_TRY(start_writing(path.c_str(), splugin_info, swrite_options));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const char *path, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_CHECK_PATH_PTR(path);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_file_with_options(path, splugin_info.sail_plugin_info_c(), &sail_write_options, &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(void *buffer, size_t buffer_length, const plugin_info &splugin_info)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    SAIL_TRY(sail_start_writing_mem(buffer,
                                    buffer_length,
                                    splugin_info.sail_plugin_info_c(),
                                    &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(void *buffer, size_t buffer_length, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_mem_with_options(buffer,
                                                 buffer_length,
                                                 splugin_info.sail_plugin_info_c(),
                                                 &sail_write_options,
                                                 &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const io &sio, const plugin_info &splugin_info)
{
    SAIL_TRY(sio.verify_valid());

    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    SAIL_TRY(sail_start_writing_io_with_options(&d->sail_io,
                                                splugin_info.sail_plugin_info_c(),
                                                NULL,
                                                &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::start_writing(const io &sio, const plugin_info &splugin_info, const write_options &swrite_options)
{
    SAIL_TRY(sio.verify_valid());

    SAIL_TRY(sio.to_sail_io(&d->sail_io));

    sail_write_options sail_write_options;
    SAIL_TRY(swrite_options.to_sail_write_options(&sail_write_options));

    SAIL_TRY(sail_start_writing_io_with_options(&d->sail_io,
                                                splugin_info.sail_plugin_info_c(),
                                                &sail_write_options,
                                                &d->state));

    return SAIL_OK;
}

sail_status_t image_writer::write_next_frame(const image &simage)
{
    sail_image *sail_image;
    SAIL_TRY(sail_alloc_image(&sail_image));

    SAIL_TRY_OR_CLEANUP(simage.to_sail_image(sail_image),
                        /* cleanup */ sail_image->pixels = NULL,
                                      sail_destroy_image(sail_image));

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(d->state, sail_image),
                        /* cleanup */ sail_image->pixels = NULL,
                                      sail_destroy_image(sail_image));

    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_writer::stop_writing()
{
    SAIL_TRY(sail_stop_writing(d->state));

    d->state = nullptr;

    return SAIL_OK;
}

sail_status_t image_writer::stop_writing(size_t *written)
{
    SAIL_TRY(sail_stop_writing_with_written(d->state, written));

    d->state = nullptr;

    return SAIL_OK;
}

}
