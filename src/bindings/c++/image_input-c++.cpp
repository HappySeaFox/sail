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

#include <memory>

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN image_input::pimpl
{
public:
    pimpl(sail::abstract_io *abstract_io_ext)
        : abstract_io(abstract_io_ext)
        , abstract_io_ref(*abstract_io)
        , abstract_io_adapter(new sail::abstract_io_adapter(abstract_io_ref))
        , state(nullptr)
        , override_codec_info(false)
        , override_load_options(false)
    {
    }

    pimpl(sail::abstract_io &abstract_io_ext)
        : abstract_io()
        , abstract_io_ref(abstract_io_ext)
        , abstract_io_adapter(new sail::abstract_io_adapter(abstract_io_ref))
        , state(nullptr)
        , override_codec_info(false)
        , override_load_options(false)
    {
    }

    sail_status_t start();

private:
    const std::unique_ptr<sail::abstract_io> abstract_io;
    sail::abstract_io &abstract_io_ref;

public:
    const std::unique_ptr<sail::abstract_io_adapter> abstract_io_adapter;
    void *state;

    bool override_codec_info;
    sail::codec_info codec_info;
    bool override_load_options;
    sail::load_options load_options;
};

sail_status_t image_input::pimpl::start()
{
    if (!override_codec_info) {
        codec_info = abstract_io_ref.codec_info();
    }

    const sail_codec_info *sail_codec_info = codec_info.sail_codec_info_c();

    sail_load_options *sail_load_options = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_load_options(sail_load_options);
    );

    if (override_load_options) {
        SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));
    }

    SAIL_TRY(sail_start_loading_from_io_with_options(&abstract_io_adapter->sail_io_c(), sail_codec_info, sail_load_options, &state));

    return SAIL_OK;
}

image_input::image_input(const std::string &path)
    : d(new pimpl(new io_file(path)))
{
}

image_input::image_input(const void *buffer, std::size_t buffer_length)
    : d(new pimpl(new io_memory(buffer, buffer_length)))
{
}

image_input::image_input(const sail::arbitrary_data &arbitrary_data)
    : image_input(arbitrary_data.data(), arbitrary_data.size())
{
}

image_input::image_input(sail::abstract_io &abstract_io)
    : d(new pimpl(abstract_io))
{
}

image_input::~image_input()
{
    if (d) {
        finish();
    }
}

image_input::image_input(image_input &&other)
{
    *this = std::move(other);
}

image_input& image_input::operator=(image_input &&other)
{
    d = std::move(other.d);
    other.d = {};

    return *this;
}

image_input& image_input::with(const sail::codec_info &codec_info)
{
    d->override_codec_info = true;
    d->codec_info          = codec_info;

    return *this;
}

image_input& image_input::with(const sail::load_options &load_options)
{
    d->override_load_options = true;
    d->load_options          = load_options;

    return *this;
}

sail_status_t image_input::next_frame(sail::image *image)
{
    if (d->state == nullptr) {
        SAIL_TRY(d->start());
    }

    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_load_next_frame(d->state, &sail_image));

    *image = sail::image(sail_image);
    sail_image->pixels = nullptr;

    return SAIL_OK;
}

image image_input::next_frame()
{
    sail::image image;

    SAIL_TRY_OR_EXECUTE(next_frame(&image),
                        /* on error */ return {});

    return image;
}

sail_status_t image_input::finish()
{
    sail_status_t saved_status = SAIL_OK;
    SAIL_TRY_OR_EXECUTE(sail_stop_loading(d->state),
                        /* on error */ saved_status = __sail_error_result);

    d->state = nullptr;

    return saved_status;
}

std::tuple<image, codec_info> image_input::probe()
{
    const sail_codec_info *sail_codec_info;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_probe_io(&d->abstract_io_adapter->sail_io_c(), &sail_image, &sail_codec_info),
                        /* on error */ return {});

    return std::tuple<image, codec_info>{ image(sail_image), codec_info(sail_codec_info) };
}

}
