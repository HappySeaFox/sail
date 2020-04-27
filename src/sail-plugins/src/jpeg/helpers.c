/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

void my_output_message(j_common_ptr cinfo) {
    char buffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message)(cinfo, buffer);

    SAIL_LOG_ERROR("JPEG: %s", buffer);
}

void my_error_exit(j_common_ptr cinfo) {
    struct my_error_context *myerr = (struct my_error_context *)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

int color_space_to_pixel_format(J_COLOR_SPACE color_space) {
    switch (color_space) {
        case JCS_GRAYSCALE: return SAIL_PIXEL_FORMAT_GRAYSCALE;

        case JCS_EXT_RGB:
        case JCS_RGB:       return SAIL_PIXEL_FORMAT_RGB;

        case JCS_YCbCr:     return SAIL_PIXEL_FORMAT_YCBCR;
        case JCS_CMYK:      return SAIL_PIXEL_FORMAT_CMYK;
        case JCS_YCCK:      return SAIL_PIXEL_FORMAT_YCCK;
        case JCS_EXT_RGBX:  return SAIL_PIXEL_FORMAT_RGBX;
        case JCS_EXT_BGR:   return SAIL_PIXEL_FORMAT_BGR;
        case JCS_EXT_BGRX:  return SAIL_PIXEL_FORMAT_BGRX;
        case JCS_EXT_XBGR:  return SAIL_PIXEL_FORMAT_XBGR;
        case JCS_EXT_XRGB:  return SAIL_PIXEL_FORMAT_XRGB;
        case JCS_EXT_RGBA:  return SAIL_PIXEL_FORMAT_RGBA;
        case JCS_EXT_BGRA:  return SAIL_PIXEL_FORMAT_BGRA;
        case JCS_EXT_ABGR:  return SAIL_PIXEL_FORMAT_ABGR;
        case JCS_EXT_ARGB:  return SAIL_PIXEL_FORMAT_ARGB;
        case JCS_RGB565:    return SAIL_PIXEL_FORMAT_RGB565;

        default:            return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

J_COLOR_SPACE pixel_format_to_color_space(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_GRAYSCALE: return JCS_GRAYSCALE;
        case SAIL_PIXEL_FORMAT_RGB:       return JCS_RGB;
        case SAIL_PIXEL_FORMAT_YCBCR:     return JCS_YCbCr;
        case SAIL_PIXEL_FORMAT_CMYK:      return JCS_CMYK;
        case SAIL_PIXEL_FORMAT_YCCK:      return JCS_YCCK;
        case SAIL_PIXEL_FORMAT_RGBX:      return JCS_EXT_RGBX;
        case SAIL_PIXEL_FORMAT_BGR:       return JCS_EXT_BGR;
        case SAIL_PIXEL_FORMAT_BGRX:      return JCS_EXT_BGRX;
        case SAIL_PIXEL_FORMAT_XBGR:      return JCS_EXT_XBGR;
        case SAIL_PIXEL_FORMAT_XRGB:      return JCS_EXT_XRGB;
        case SAIL_PIXEL_FORMAT_RGBA:      return JCS_EXT_RGBA;
        case SAIL_PIXEL_FORMAT_BGRA:      return JCS_EXT_BGRA;
        case SAIL_PIXEL_FORMAT_ABGR:      return JCS_EXT_ABGR;
        case SAIL_PIXEL_FORMAT_ARGB:      return JCS_EXT_ARGB;
        case SAIL_PIXEL_FORMAT_RGB565:    return JCS_RGB565;

        default:                          return JCS_UNKNOWN;
    }
}
