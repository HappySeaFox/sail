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
        case JCS_GRAYSCALE: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;

        case JCS_RGB565:    return SAIL_PIXEL_FORMAT_BPP16_RGB565;

        case JCS_EXT_RGB:
        case JCS_RGB:       return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case JCS_EXT_BGR:   return SAIL_PIXEL_FORMAT_BPP24_BGR;

        case JCS_EXT_RGBA:  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case JCS_EXT_BGRA:  return SAIL_PIXEL_FORMAT_BPP32_BGRA;
        case JCS_EXT_ABGR:  return SAIL_PIXEL_FORMAT_BPP32_ABGR;
        case JCS_EXT_ARGB:  return SAIL_PIXEL_FORMAT_BPP32_ARGB;

        case JCS_YCbCr:     return SAIL_PIXEL_FORMAT_BPP24_YCBCR;
        case JCS_CMYK:      return SAIL_PIXEL_FORMAT_BPP32_CMYK;
        case JCS_YCCK:      return SAIL_PIXEL_FORMAT_BPP32_YCCK;

        default:            return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

J_COLOR_SPACE pixel_format_to_color_space(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:  return JCS_GRAYSCALE;

        case SAIL_PIXEL_FORMAT_BPP16_RGB565:    return JCS_RGB565;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:       return JCS_RGB;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:       return JCS_EXT_BGR;

        case SAIL_PIXEL_FORMAT_BPP32_RGBA:      return JCS_EXT_RGBA;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:      return JCS_EXT_BGRA;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:      return JCS_EXT_ABGR;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:      return JCS_EXT_ARGB;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:     return JCS_YCbCr;
        case SAIL_PIXEL_FORMAT_BPP32_CMYK:      return JCS_CMYK;
        case SAIL_PIXEL_FORMAT_BPP32_YCCK:      return JCS_YCCK;

        default:                                return JCS_UNKNOWN;
    }
}

bool jpeg_supported_pixel_format(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:
        case SAIL_PIXEL_FORMAT_BPP32_CMYK:
        case SAIL_PIXEL_FORMAT_BPP32_YCCK: return true;

        default:                           return false;
    }
}
