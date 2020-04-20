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
}

bool image_writer::is_valid() const
{
    return d->ctx != nullptr && d->ctx->is_valid();
}

sail_error_t image_writer::write(const char *path, const image *simage, plugin_info **splugin_info)
{
    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_IMAGE_PTR(simage);

    const sail_plugin_info *sail_plugin_info = nullptr;
    sail_image *sail_image = nullptr;

    SAIL_AT_SCOPE_EXIT (
        sail_destroy_image(sail_image);
    );

    SAIL_TRY(simage->to_sail_image(&sail_image));

    SAIL_TRY(sail_write(path,
                        d->ctx->to_sail_context(),
                        sail_image,
                        simage->bits(),
                        &sail_plugin_info));

    if (splugin_info != nullptr) {
        *splugin_info = new plugin_info(sail_plugin_info);

        if (*splugin_info == nullptr) {
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }
    }

    return 0;
}

}
