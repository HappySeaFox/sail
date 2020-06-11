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

#ifndef SAIL_IMAGE_WRITER_CPP_H
#define SAIL_IMAGE_WRITER_CPP_H

#include <cstddef>
#include <string>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

namespace sail
{

class context;
class image;
class io;
class plugin_info;
class write_options;

/*
 * A C++ interface to the SAIL image writing functions.
 */
class SAIL_EXPORT image_writer
{
public:
    image_writer();
    image_writer(context *ctx);
    ~image_writer();

    bool is_valid() const;

    /*
     * An interface to sail_write(). See sail_write() for more.
     */
    sail_error_t write(const std::string &path, const image &simage);
    sail_error_t write(const char *path, const image &simage);

    /*
     * An interface to sail_start_writing(). See sail_start_writing() for more.
     */
    sail_error_t start_writing(const std::string &path);
    sail_error_t start_writing(const char *path);

    /*
     * An interface to sail_start_writing(). See sail_start_writing() for more.
     */
    sail_error_t start_writing(const std::string &path, const plugin_info &splugin_info);
    sail_error_t start_writing(const char *path, const plugin_info &splugin_info);

    /*
     * An interface to sail_start_writing_with_options(). See sail_start_writing_with_options() for more.
     */
    sail_error_t start_writing(const std::string &path, const write_options &swrite_options);
    sail_error_t start_writing(const char *path, const write_options &swrite_options);

    /*
     * An interface to sail_start_writing_with_options(). See sail_start_writing_with_options() for more.
     */
    sail_error_t start_writing(const std::string &path, const plugin_info &splugin_info, const write_options &swrite_options);
    sail_error_t start_writing(const char *path, const plugin_info &splugin_info, const write_options &swrite_options);

    /*
     * An interface to sail_start_writing_mem(). See sail_start_writing_mem() for more.
     */
    sail_error_t start_writing(void *buffer, size_t buffer_length, const plugin_info &splugin_info);
    sail_error_t start_writing(void *buffer, size_t buffer_length, const plugin_info &splugin_info, const write_options &swrite_options);

    /*
     * An interface to sail_start_writing_io(). See sail_start_writing_io() for more.
     */
    sail_error_t start_writing(const io &sio, const plugin_info &splugin_info);
    sail_error_t start_writing(const io &sio, const plugin_info &splugin_info, const write_options &swrite_options);

    /*
     * An interface to sail_write_next_frame(). See sail_write_next_frame() for more.
     */
    sail_error_t write_next_frame(const image &simage);

    /*
     * An interface to sail_stop_writing(). See sail_stop_writing() for more.
     */
    sail_error_t stop_writing();

    /*
     * An interface to sail_stop_writing_with_written(). See sail_stop_writing_with_written() for more.
     */
    sail_error_t stop_writing(size_t *written);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
