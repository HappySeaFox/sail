/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <sail-c++/sail-c++.h>
#include <sail/sail.h>

namespace sail
{

class SAIL_HIDDEN image_output::pimpl
{
public:
    pimpl(sail::abstract_io *abstract_io_ext, const sail::codec_info &other_codec_info)
        : abstract_io(abstract_io_ext)
        , abstract_io_ref(*abstract_io)
        , abstract_io_adapter(new sail::abstract_io_adapter(abstract_io_ref))
        , state(nullptr)
        , codec_info(other_codec_info)
        , override_save_options(false)
    {
    }

    pimpl(sail::abstract_io &abstract_io_ext, const sail::codec_info &other_codec_info)
        : abstract_io()
        , abstract_io_ref(abstract_io_ext)
        , abstract_io_adapter(new sail::abstract_io_adapter(abstract_io_ref))
        , state(nullptr)
        , codec_info(other_codec_info)
        , override_save_options(false)
    {
    }

    sail_status_t start();

private:
    const std::unique_ptr<sail::abstract_io> abstract_io;
    sail::abstract_io &abstract_io_ref;

public:
    const std::unique_ptr<sail::abstract_io_adapter> abstract_io_adapter;
    void *state;

    sail::codec_info codec_info;
    bool override_save_options;
    sail::save_options save_options;
};

sail_status_t image_output::pimpl::start()
{
    const sail_codec_info *sail_codec_info = codec_info.sail_codec_info_c();

    sail_save_options *sail_save_options = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_save_options(sail_save_options);
    );

    if (override_save_options) {
        SAIL_TRY(save_options.to_sail_save_options(&sail_save_options));
    }

    SAIL_TRY(sail_start_saving_into_io_with_options(&abstract_io_adapter->sail_io_c(), sail_codec_info, sail_save_options, &state));

    return SAIL_OK;
}

image_output::image_output(const std::string &path)
    : d(new pimpl(new io_file(path, io_file::Operation::ReadWrite), sail::codec_info::from_path(path)))
{
}

image_output::image_output(void *buffer, std::size_t buffer_size, const sail::codec_info &codec_info)
    : d(new pimpl(new io_memory(buffer, buffer_size), codec_info))
{
}

image_output::image_output(sail::arbitrary_data *arbitrary_data, const sail::codec_info &codec_info)
    : image_output(arbitrary_data->data(), arbitrary_data->size(), codec_info)
{
}

image_output::image_output(sail::abstract_io &abstract_io, const sail::codec_info &codec_info)
    : d(new pimpl(abstract_io, codec_info))
{
}

image_output::~image_output()
{
    if (d) {
        finish();
    }
}

image_output::image_output(image_output &&other)
{
    *this = std::move(other);
}

image_output& image_output::operator=(image_output &&other)
{
    d = std::move(other.d);
    other.d = {};

    return *this;
}

image_output& image_output::with(const sail::codec_info &codec_info)
{
    d->codec_info = codec_info;

    return *this;
}

image_output& image_output::with(const sail::save_options &save_options)
{
    d->override_save_options = true;
    d->save_options          = save_options;

    return *this;
}

sail_status_t image_output::next_frame(const sail::image &image)
{
    if (d->state == nullptr) {
        SAIL_TRY(d->start());
    }

    sail_image *sail_image = nullptr;
    SAIL_TRY(image.to_sail_image(&sail_image));

    SAIL_AT_SCOPE_EXIT(
        sail_image->pixels = nullptr;
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_write_next_frame(d->state, sail_image));

    return SAIL_OK;
}

sail_status_t image_output::finish()
{
    sail_status_t saved_status = SAIL_OK;
    SAIL_TRY_OR_EXECUTE(sail_stop_saving(d->state),
                        /* on error */ saved_status = __sail_error_result);

    d->state = nullptr;

    return saved_status;
}

}
