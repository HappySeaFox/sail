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

#ifndef SAIL_IMAGE_READER_CPP_H
#define SAIL_IMAGE_READER_CPP_H

#include <cstddef>
#include <string_view>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

namespace sail
{

class image;
class io;
class codec_info;
class read_options;

/*
 * A C++ interface to the SAIL image reading functions.
 */
class SAIL_EXPORT image_reader
{
public:
    image_reader();
    ~image_reader();

    /*
     * An interface to sail_probe_file(). See sail_probe_file() for more.
     */
    sail_status_t probe(std::string_view path, image *simage, codec_info *scodec_info = nullptr);

    /*
     * An interface to sail_probe_mem(). See sail_probe_mem() for more.
     */
    sail_status_t probe(const void *buffer, size_t buffer_length, image *simage, codec_info *scodec_info = nullptr);

    /*
     * An interface to sail_probe_io(). See sail_probe_io() for more.
     */
    sail_status_t probe(const sail::io &io, image *simage, codec_info *scodec_info = nullptr);

    /*
     * An interface to sail_read_file(). See sail_read_file() for more.
     */
    sail_status_t read(std::string_view path, image *simage);

    /*
     * An interface to sail_read_mem(). See sail_read_mem() for more.
     */
    sail_status_t read(const void *buffer, size_t buffer_length, image *simage);

    /*
     * An interface to sail_start_reading_file(). See sail_start_reading() for more.
     */
    sail_status_t start_reading(std::string_view path);

    /*
     * An interface to sail_start_reading_file(). See sail_start_reading_file() for more.
     */
    sail_status_t start_reading(std::string_view path, const codec_info &scodec_info);

    /*
     * An interface to sail_start_reading_file_with_options(). See sail_start_reading_file_with_options() for more.
     */
    sail_status_t start_reading(std::string_view path, const read_options &sread_options);

    /*
     * An interface to sail_start_reading_file_with_options(). See sail_start_reading_file_with_options() for more.
     */
    sail_status_t start_reading(std::string_view path, const codec_info &scodec_info, const read_options &sread_options);

    /*
     * An interface to sail_start_reading_mem(). See sail_start_reading_mem() for more.
     */
    sail_status_t start_reading(const void *buffer, size_t buffer_length, const codec_info &scodec_info);
    sail_status_t start_reading(const void *buffer, size_t buffer_length, const codec_info &scodec_info, const read_options &sread_options);

    /*
     * An interface to sail_start_reading_io(). See sail_start_reading_io() for more.
     */
    sail_status_t start_reading(const io &sio, const codec_info &scodec_info);
    sail_status_t start_reading(const io &sio, const codec_info &scodec_info, const read_options &sread_options);

    /*
     * An interface to sail_read_next_frame(). See sail_read_next_frame() for more.
     */
    sail_status_t read_next_frame(image *simage);

    /*
     * An interface to sail_stop_reading(). See sail_stop_reading() for more.
     */
    sail_status_t stop_reading();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
