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

class SAIL_HIDDEN image_output::pimpl
{
public:
    pimpl()
        : state(nullptr)
        , written(0)
    {
    }

    sail_status_t start()
    {
        SAIL_TRY(ensure_not_started());

        written = 0;

        return SAIL_OK;
    }

    void *state;
    std::unique_ptr<sail::abstract_io_adapter> abstract_io_adapter;
    std::size_t written;

private:
    sail_status_t ensure_not_started()
    {
        if (state != nullptr) {
            SAIL_LOG_ERROR("Saving operation is in progress. Stop it before starting a new one");
            return SAIL_ERROR_CONFLICTING_OPERATION;
        }

        return SAIL_OK;
    }
};

image_output::image_output()
    : d(new pimpl)
{
}

image_output::~image_output()
{
    stop();
}

sail_status_t image_output::start(const std::string &path)
{
    SAIL_TRY(d->start());

    SAIL_TRY(sail_start_saving_file(path.c_str(), nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_output::start(const std::string &path, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->start());

    SAIL_TRY(sail_start_saving_file(path.c_str(), codec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_output::start(const std::string &path, const sail::save_options &save_options)
{
    SAIL_TRY(d->start());

    sail_save_options *sail_save_options;
    SAIL_TRY(save_options.to_sail_save_options(&sail_save_options));

    SAIL_TRY_OR_CLEANUP(sail_start_saving_file_with_options(path.c_str(), nullptr, sail_save_options, &d->state),
                        /* cleanup */ sail_destroy_save_options(sail_save_options));

    sail_destroy_save_options(sail_save_options);

    return SAIL_OK;
}

sail_status_t image_output::start(const std::string &path, const sail::codec_info &codec_info, const sail::save_options &save_options)
{
    SAIL_TRY(d->start());

    sail_save_options *sail_save_options;
    SAIL_TRY(save_options.to_sail_save_options(&sail_save_options));

    SAIL_TRY_OR_CLEANUP(sail_start_saving_file_with_options(path.c_str(), codec_info.sail_codec_info_c(), sail_save_options, &d->state),
                        /* cleanup */ sail_destroy_save_options(sail_save_options));

    sail_destroy_save_options(sail_save_options);

    return SAIL_OK;
}

sail_status_t image_output::start(void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->start());

    SAIL_TRY(sail_start_saving_memory(buffer,
                                      buffer_length,
                                      codec_info.sail_codec_info_c(),
                                      &d->state));

    return SAIL_OK;
}

sail_status_t image_output::start(void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info, const sail::save_options &save_options)
{
    SAIL_TRY(d->start());

    sail_save_options *sail_save_options;
    SAIL_TRY(save_options.to_sail_save_options(&sail_save_options));

    SAIL_TRY_OR_CLEANUP(sail_start_saving_memory_with_options(buffer, buffer_length, codec_info.sail_codec_info_c(), sail_save_options, &d->state),
                        /* cleanup */ sail_destroy_save_options(sail_save_options));

    sail_destroy_save_options(sail_save_options);

    return SAIL_OK;
}

sail_status_t image_output::start(sail::arbitrary_data *arbitrary_data, const sail::codec_info &codec_info)
{
    SAIL_TRY(start(arbitrary_data->data(), arbitrary_data->size(), codec_info));

    return SAIL_OK;
}

sail_status_t image_output::start(sail::arbitrary_data *arbitrary_data, const sail::codec_info &codec_info, const sail::save_options &save_options)
{
    SAIL_TRY(start(arbitrary_data->data(), arbitrary_data->size(), codec_info, save_options));

    return SAIL_OK;
}

sail_status_t image_output::start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->start());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    SAIL_TRY(sail_start_saving_io_with_options(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_output::start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info, const sail::save_options &save_options)
{
    SAIL_TRY(d->start());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    sail_save_options *sail_save_options;
    SAIL_TRY(save_options.to_sail_save_options(&sail_save_options));

    SAIL_TRY_OR_CLEANUP(sail_start_saving_io_with_options(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), sail_save_options, &d->state),
                        /* cleanup */ sail_destroy_save_options(sail_save_options));

    sail_destroy_save_options(sail_save_options);

    return SAIL_OK;
}

sail_status_t image_output::next_frame(const sail::image &image) const
{
    sail_image *sail_image = nullptr;
    SAIL_TRY(image.to_sail_image(&sail_image));

    SAIL_AT_SCOPE_EXIT(
        sail_image->pixels = nullptr;
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_write_next_frame(d->state, sail_image));

    return SAIL_OK;
}

sail_status_t image_output::stop()
{
    sail_status_t saved_status = SAIL_OK;
    SAIL_TRY_OR_EXECUTE(sail_stop_saving_with_written(d->state, &d->written),
                        /* on error */ saved_status = __sail_error_result);

    d->state = nullptr;
    d->abstract_io_adapter.reset();

    return saved_status;
}

std::size_t image_output::written() const
{
    return d->written;
}

sail_status_t image_output::save(const std::string &path, const sail::image &image)
{
    sail_image *sail_image = nullptr;
    SAIL_TRY(image.to_sail_image(&sail_image));

    SAIL_AT_SCOPE_EXIT(
        sail_image->pixels = nullptr;
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_save_image_into_file(path.c_str(), sail_image));

    return SAIL_OK;
}

sail_status_t image_output::save(void *buffer, std::size_t buffer_length, const sail::image &image)
{
    SAIL_TRY(save(buffer, buffer_length, image, nullptr));

    return SAIL_OK;
}

sail_status_t image_output::save(void *buffer, std::size_t buffer_length, const sail::image &image, std::size_t *written)
{
    SAIL_CHECK_PTR(buffer);

    sail_image *sail_image = nullptr;
    SAIL_TRY(image.to_sail_image(&sail_image));

    SAIL_AT_SCOPE_EXIT(
        sail_image->pixels = nullptr;
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_save_image_into_memory(buffer, buffer_length, sail_image, written));

    return SAIL_OK;
}

sail_status_t image_output::save(sail::arbitrary_data *arbitrary_data, const sail::image &image)
{
    SAIL_TRY(save(arbitrary_data->data(), arbitrary_data->size(), image));

    return SAIL_OK;
}

sail_status_t image_output::save(sail::arbitrary_data *arbitrary_data, const sail::image &image, std::size_t *written)
{
    SAIL_TRY(save(arbitrary_data->data(), arbitrary_data->size(), image, written));

    return SAIL_OK;
}

}
