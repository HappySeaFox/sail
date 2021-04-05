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
        : state(nullptr)
        , sail_io(nullptr)
    {
    }

    void *state;
    struct sail_io *sail_io;
};

image_reader::image_reader()
    : d(new pimpl)
{
}

image_reader::~image_reader()
{
    stop_reading();
    delete d;
}

sail_status_t image_reader::probe(const std::string_view path, image *simage, codec_info *scodec_info)
{
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_codec_info *sail_codec_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_file(path.data(),
                             &sail_image,
                             &sail_codec_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (scodec_info != nullptr) {
        *scodec_info = codec_info(sail_codec_info);
    }

    return SAIL_OK;
}

sail_status_t image_reader::probe(const void *buffer, size_t buffer_length, image *simage, codec_info *scodec_info)
{
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_codec_info *sail_codec_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_mem(buffer,
                            buffer_length,
                            &sail_image,
                            &sail_codec_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (scodec_info != nullptr) {
        *scodec_info = codec_info(sail_codec_info);
    }

    return SAIL_OK;
}

sail_status_t image_reader::probe(const sail::io &io, image *simage, codec_info *scodec_info)
{
    SAIL_CHECK_IMAGE_PTR(simage);
    SAIL_TRY(io.verify_valid());

    struct sail_io *sail_io = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_io(sail_io);
    );

    SAIL_TRY(io.to_sail_io(&sail_io));

    const sail_codec_info *sail_codec_info;
    sail_image *sail_image;

    SAIL_TRY(sail_probe_io(sail_io,
                           &sail_image,
                           &sail_codec_info));

    *simage = image(sail_image);
    sail_destroy_image(sail_image);

    if (scodec_info != nullptr) {
        *scodec_info = codec_info(sail_codec_info);
    }

    return SAIL_OK;
}

sail_status_t image_reader::read(const std::string_view path, image *simage)
{
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;

    SAIL_TRY(sail_read_file(path.data(), &sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_reader::read(const void *buffer, size_t buffer_length, image *simage)
{
    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;

    SAIL_TRY(sail_read_mem(buffer,
                            buffer_length,
                            &sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const std::string_view path)
{
    SAIL_TRY(sail_start_reading_file(path.data(), nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const std::string_view path, const codec_info &scodec_info)
{
    SAIL_TRY(sail_start_reading_file(path.data(), scodec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const std::string_view path, const codec_info &scodec_info, const read_options &sread_options)
{
    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_file_with_options(path.data(),
                                                  scodec_info.sail_codec_info_c(),
                                                  &sail_read_options,
                                                  &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const void *buffer, size_t buffer_length, const codec_info &scodec_info)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    SAIL_TRY(sail_start_reading_mem(buffer,
                                    buffer_length,
                                    scodec_info.sail_codec_info_c(),
                                    &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const void *buffer, size_t buffer_length, const codec_info &scodec_info, const read_options &sread_options)
{
    SAIL_CHECK_BUFFER_PTR(buffer);

    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_mem_with_options(buffer,
                                                 buffer_length,
                                                 scodec_info.sail_codec_info_c(),
                                                 &sail_read_options,
                                                 &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const io &sio, const codec_info &scodec_info)
{
    SAIL_TRY(sio.to_sail_io(&d->sail_io));
    SAIL_TRY(sail_check_io_valid(d->sail_io));

    SAIL_TRY(sail_start_reading_io_with_options(d->sail_io,
                                                scodec_info.sail_codec_info_c(),
                                                NULL,
                                                &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::start_reading(const io &sio, const codec_info &scodec_info, const read_options &sread_options)
{
    SAIL_TRY(sio.to_sail_io(&d->sail_io));
    SAIL_TRY(sail_check_io_valid(d->sail_io));

    sail_read_options sail_read_options;
    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));

    SAIL_TRY(sail_start_reading_io_with_options(d->sail_io,
                                                scodec_info.sail_codec_info_c(),
                                                &sail_read_options,
                                                &d->state));

    return SAIL_OK;
}

sail_status_t image_reader::read_next_frame(image *simage)
{
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image;
    SAIL_TRY(sail_read_next_frame(d->state, &sail_image));

    *simage = image(sail_image);
    sail_image->pixels = NULL;
    sail_destroy_image(sail_image);

    return SAIL_OK;
}

sail_status_t image_reader::stop_reading()
{
    SAIL_TRY(sail_stop_reading(d->state));
    d->state = nullptr;

    sail_destroy_io(d->sail_io);
    d->sail_io = nullptr;

    return SAIL_OK;
}

}
