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
    pimpl()
        : state(nullptr)
    {
    }

    sail_status_t ensure_not_started()
    {
        if (state != nullptr) {
            SAIL_LOG_ERROR("Reading operation is in progress. Stop it before starting a new one");
            return SAIL_ERROR_CONFLICTING_OPERATION;
        }

        return SAIL_OK;
    }

    void *state;
    std::unique_ptr<sail::abstract_io_adapter> abstract_io_adapter;
};

image_input::image_input()
    : d(new pimpl)
{
}

image_input::~image_input()
{
    stop();
}

sail_status_t image_input::start(const std::string &path)
{
    SAIL_TRY(d->ensure_not_started());

    SAIL_TRY(sail_start_loading_file(path.c_str(), nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(const std::string &path, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->ensure_not_started());

    SAIL_TRY(sail_start_loading_file(path.c_str(), codec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(const std::string &path, const sail::codec_info &codec_info, const sail::load_options &load_options)
{
    SAIL_TRY(d->ensure_not_started());

    sail_load_options *sail_load_options;
    SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_file_with_options(path.c_str(), codec_info.sail_codec_info_c(), sail_load_options, &d->state),
                        /* cleanup */ sail_destroy_load_options(sail_load_options));

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

sail_status_t image_input::start(const void *buffer, std::size_t buffer_length)
{
    SAIL_TRY(d->ensure_not_started());

    SAIL_TRY(sail_start_loading_memory(buffer, buffer_length, nullptr, &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(const void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->ensure_not_started());

    SAIL_TRY(sail_start_loading_memory(buffer, buffer_length, codec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(const void *buffer, std::size_t buffer_length, const sail::load_options &load_options)
{
    SAIL_TRY(d->ensure_not_started());

    sail_load_options *sail_load_options;
    SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_memory_with_options(buffer, buffer_length, nullptr, sail_load_options, &d->state),
                        /* cleanup */ sail_destroy_load_options(sail_load_options));

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

sail_status_t image_input::start(const void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info, const sail::load_options &load_options)
{
    SAIL_TRY(d->ensure_not_started());

    sail_load_options *sail_load_options;
    SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_memory_with_options(buffer, buffer_length, codec_info.sail_codec_info_c(), sail_load_options, &d->state),
                        /* cleanup */ sail_destroy_load_options(sail_load_options));

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

sail_status_t image_input::start(const sail::arbitrary_data &arbitrary_data)
{
    SAIL_TRY(start(arbitrary_data.data(), arbitrary_data.size()));

    return SAIL_OK;
}

sail_status_t image_input::start(const sail::arbitrary_data &arbitrary_data, const sail::codec_info &codec_info)
{
    SAIL_TRY(start(arbitrary_data.data(), arbitrary_data.size(), codec_info));

    return SAIL_OK;
}

sail_status_t image_input::start(const sail::arbitrary_data &arbitrary_data, const sail::load_options &load_options)
{
    SAIL_TRY(start(arbitrary_data.data(), arbitrary_data.size(), load_options));

    return SAIL_OK;
}

sail_status_t image_input::start(const sail::arbitrary_data &arbitrary_data, const sail::codec_info &codec_info, const sail::load_options &load_options)
{
    SAIL_TRY(start(arbitrary_data.data(), arbitrary_data.size(), codec_info, load_options));

    return SAIL_OK;
}

sail_status_t image_input::start(sail::abstract_io &abstract_io)
{
    SAIL_TRY(d->ensure_not_started());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    const sail::codec_info codec_info = abstract_io.codec_info();

    if (!codec_info.is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    SAIL_TRY(sail_start_loading_io(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info)
{
    SAIL_TRY(d->ensure_not_started());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    SAIL_TRY(sail_start_loading_io(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), &d->state));

    return SAIL_OK;
}

sail_status_t image_input::start(sail::abstract_io &abstract_io, const sail::load_options &load_options)
{
    SAIL_TRY(d->ensure_not_started());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    const sail::codec_info codec_info = abstract_io.codec_info();

    if (!codec_info.is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    sail_load_options *sail_load_options;
    SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_io_with_options(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), sail_load_options, &d->state),
                        /* cleanup */ sail_destroy_load_options(sail_load_options));

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

sail_status_t image_input::start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info, const sail::load_options &load_options)
{
    SAIL_TRY(d->ensure_not_started());

    d->abstract_io_adapter.reset(new sail::abstract_io_adapter(abstract_io));

    sail_load_options *sail_load_options;
    SAIL_TRY(load_options.to_sail_load_options(&sail_load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_io_with_options(&d->abstract_io_adapter->sail_io_c(), codec_info.sail_codec_info_c(), sail_load_options, &d->state),
                        /* cleanup */ sail_destroy_load_options(sail_load_options));

    sail_destroy_load_options(sail_load_options);

    return SAIL_OK;
}

sail_status_t image_input::next_frame(sail::image *image)
{
    SAIL_CHECK_PTR(image);

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

sail_status_t image_input::stop()
{
    sail_status_t saved_status = SAIL_OK;
    SAIL_TRY_OR_EXECUTE(sail_stop_loading(d->state),
                        /* on error */ saved_status = __sail_error_result);

    d->state = nullptr;
    d->abstract_io_adapter.reset();

    return saved_status;
}

std::tuple<image, codec_info> image_input::probe(const std::string &path)
{
    const sail_codec_info *sail_codec_info;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_probe_file(path.c_str(), &sail_image, &sail_codec_info),
                        /* on error */ return {});

    return std::tuple<image, codec_info>{ image(sail_image), codec_info(sail_codec_info) };
}

std::tuple<image, codec_info> image_input::probe(const void *buffer, std::size_t buffer_length)
{
    const sail_codec_info *sail_codec_info;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_probe_memory(buffer, buffer_length, &sail_image, &sail_codec_info),
                        /* on error */ return {});

    return std::tuple<image, codec_info>{ image(sail_image), codec_info(sail_codec_info) };
}

std::tuple<image, codec_info> image_input::probe(const sail::arbitrary_data &arbitrary_data)
{
    return probe(arbitrary_data.data(), arbitrary_data.size());
}

std::tuple<image, codec_info> image_input::probe(sail::abstract_io &abstract_io)
{
    sail::abstract_io_adapter abstract_io_adapter(abstract_io);

    const sail_codec_info *sail_codec_info;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_probe_io(&abstract_io_adapter.sail_io_c(), &sail_image, &sail_codec_info),
                        /* on error */ return {});

    return std::tuple<image, codec_info>{ image(sail_image), codec_info(sail_codec_info) };
}

image image_input::load(const std::string &path)
{
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_load_image_from_file(path.c_str(), &sail_image),
                        /* on error */ return {});

    const sail::image image(sail_image);
    sail_image->pixels = nullptr;

    return image;
}

image image_input::load(const void *buffer, std::size_t buffer_length)
{
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_image(sail_image);
    );

    SAIL_TRY_OR_EXECUTE(sail_load_image_from_memory(buffer, buffer_length, &sail_image),
                        /* on error */ return {});

    const sail::image image(sail_image);
    sail_image->pixels = nullptr;

    return image;
}

image image_input::load(const sail::arbitrary_data &arbitrary_data)
{
    return load(arbitrary_data.data(), arbitrary_data.size());
}

}
