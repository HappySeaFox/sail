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

#ifndef SAIL_JPEG_HELPERS_H
#define SAIL_JPEG_HELPERS_H

#include <setjmp.h>

#include <jpeglib.h>

#include "export.h"

struct my_error_context {
    struct jpeg_error_mgr jpeg_error_mgr;
    jmp_buf setjmp_buffer;
};

SAIL_HIDDEN void my_output_message(j_common_ptr cinfo);

SAIL_HIDDEN void my_error_exit(j_common_ptr cinfo);

SAIL_HIDDEN int color_space_to_pixel_format(J_COLOR_SPACE color_space);

SAIL_HIDDEN J_COLOR_SPACE pixel_format_to_color_space(int pixel_format);

#endif
