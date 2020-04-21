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

#include "config.h"

#include <cstdlib>
#include <cstring>

// libsail-common.
#include "common.h"
#include "error.h"
#include "log.h"
#include "utils.h"

// libsail.
#include "plugin_info.h"
#include "sail.h"

#include "at_scope_exit-c++.h"
#include "context-c++.h"
#include "image_writer-c++.h"
#include "image-c++.h"
#include "plugin_info-c++.h"

namespace sail
{

class SAIL_HIDDEN image_writer::pimpl
{
public:
    pimpl(context *_ctx)
        : ctx(_ctx)
    {
    }

    context *ctx;
    void *pmpl;
};

image_writer::image_writer(context *ctx)
    : d(new pimpl(ctx))
{
    if (ctx == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to image_writer()");
    }
}

image_writer::~image_writer()
{
    delete d;
}

bool image_writer::is_valid() const
{
    return d->ctx != nullptr && d->ctx->is_valid();
}

sail_error_t image_writer::write(const std::string &path, const image *simage)
{
    SAIL_TRY(write(path.c_str(), simage));

    return 0;
}

sail_error_t image_writer::write(const char *path, const image *simage)
{
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(simage->to_sail_image(&sail_image));

    SAIL_TRY(sail_write(path,
                        d->ctx->sail_context_c(),
                        sail_image,
                        simage->bits()));

    return 0;
}

sail_error_t image_writer::start_writing(const std::string &path, const plugin_info *splugin_info)
{
    SAIL_TRY(start_writing(path.c_str(), splugin_info));

    return 0;
}

sail_error_t image_writer::start_writing(const char *path, const plugin_info *splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);

    const sail_plugin_info *sail_plugin_info = splugin_info == nullptr ? nullptr : splugin_info->to_sail_plugin_info();

    SAIL_TRY(sail_start_writing(path, d->ctx->sail_context_c(), sail_plugin_info, &d->pmpl));

    return 0;
}

sail_error_t image_writer::write_next_frame(const image *simage)
{
    sail_image *image = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(image);
    );

    SAIL_TRY(simage->to_sail_image(&image));

    const void *bits = simage->bits() ? simage->bits() : simage->shallow_bits();

    SAIL_TRY(sail_write_next_frame(d->pmpl, image, bits));

    return 0;
}

sail_error_t image_writer::stop_writing()
{
    SAIL_TRY(sail_stop_writing(d->pmpl));

    d->pmpl = nullptr;

    return 0;
}

}
