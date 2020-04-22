/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

class SAIL_HIDDEN image_reader::pimpl
{
public:
    pimpl(context *_ctx)
        : ctx(_ctx)
        , pmpl(nullptr)
    {
    }

    context *ctx;
    void *pmpl;
};

image_reader::image_reader(context *ctx)
    : d(new pimpl(ctx))
{
    if (ctx == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to image_reader()");
    }
}

image_reader::~image_reader()
{
    delete d;
}

bool image_reader::is_valid() const
{
    return d->ctx != nullptr && d->ctx->is_valid();
}

sail_error_t image_reader::probe(const std::string &path, image **simage, plugin_info **splugin_info)
{
    SAIL_TRY(probe(path.c_str(), simage, splugin_info));

    return 0;
}

sail_error_t image_reader::probe(const char *path, image **simage, plugin_info **splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_plugin_info *sail_plugin_info = nullptr;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(sail_probe(path,
                        d->ctx->sail_context_c(),
                        &sail_image,
                        &sail_plugin_info));

    *simage = new image(sail_image);

    if (*simage == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    if (splugin_info != nullptr) {
        *splugin_info = new plugin_info(sail_plugin_info);

        if (*splugin_info == nullptr) {
            delete *simage;
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }
    }

    return 0;
}

sail_error_t image_reader::read(const std::string &path, image **simage)
{
    SAIL_TRY(read(path.c_str(), simage));

    return 0;
}

sail_error_t image_reader::read(const char *path, image **simage)
{
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image = nullptr;
    void *image_bits = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
        free(image_bits);
    );

    SAIL_TRY(sail_read(path,
                       d->ctx->sail_context_c(),
                       &sail_image,
                       &image_bits));

    *simage = new image(sail_image);

    if (*simage == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    int bytes_per_image;
    SAIL_TRY(sail_bytes_per_image(sail_image, &bytes_per_image));

    (*simage)->with_bits(image_bits, bytes_per_image);

    return 0;
}

sail_error_t image_reader::start_reading(const std::string &path, const plugin_info *splugin_info)
{
    SAIL_TRY(start_reading(path.c_str(), splugin_info));

    return 0;
}

sail_error_t image_reader::start_reading(const char *path, const plugin_info *splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);

    const sail_plugin_info *sail_plugin_info = splugin_info == nullptr ? nullptr : splugin_info->sail_plugin_info_c();

    SAIL_TRY(sail_start_reading(path, d->ctx->sail_context_c(), sail_plugin_info, &d->pmpl));

    return 0;
}

sail_error_t image_reader::start_reading(const std::string &path, const plugin_info *splugin_info, const read_options &sread_options)
{
    SAIL_TRY(start_reading(path.c_str(), splugin_info, sread_options));

    return 0;
}

sail_error_t image_reader::start_reading(const char *path, const plugin_info *splugin_info, const read_options &sread_options)
{
    SAIL_CHECK_PATH_PTR(path);

    const sail_plugin_info *sail_plugin_info = splugin_info == nullptr ? nullptr : splugin_info->sail_plugin_info_c();
    sail_read_options *sail_read_options = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_read_options(sail_read_options);
    );

    SAIL_TRY(sread_options.to_sail_read_options(&sail_read_options));
    SAIL_TRY(sail_start_reading_with_options(path, d->ctx->sail_context_c(), sail_plugin_info, sail_read_options, &d->pmpl));

    return 0;
}

sail_error_t image_reader::read_next_frame(image **simage)
{
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image = nullptr;
    void *image_bits = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
        free(image_bits);
    );

    SAIL_TRY(sail_read_next_frame(d->pmpl, &sail_image, (void **)&image_bits));

    *simage = new image(sail_image);

    if (*simage == nullptr) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    int bytes_per_image;
    SAIL_TRY(sail_bytes_per_image(sail_image, &bytes_per_image));

    (*simage)->with_bits(image_bits, bytes_per_image);

    return 0;
}

sail_error_t image_reader::stop_reading()
{
    SAIL_TRY(sail_stop_reading(d->pmpl));

    d->pmpl = nullptr;

    return 0;
}

}
