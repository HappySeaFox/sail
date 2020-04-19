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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#endif

#include "common.h"
#include "error.h"
#include "utils.h"

int sail_strdup(const char *input, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    const size_t len = strlen((char *)input);

    *output = (char *)malloc(len+1);

    if (*output == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(*output, input, len);
    (*output)[len] = '\0';

    return 0;
}

int sail_strdup_length(const char *input, size_t length, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    if (length == 0) {
        return SAIL_INVALID_ARGUMENT;
    }

    *output = (char *)malloc(length+1);

    if (*output == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(*output, input, length);
    (*output)[length] = '\0';

    return 0;
}

sail_error_t sail_concat(char **output, int num, ...) {

    if (num < 1) {
        return SAIL_INVALID_ARGUMENT;
    }

    SAIL_CHECK_PTR(output);

    va_list args;

    /* Calculate the necessary string length. */
    va_start(args, num);
    const char *arg;
    size_t length = 1; /* for NULL */
    int counter = num;

    while (counter--) {
        arg = va_arg(args, const char *);
        length += strlen(arg);
    }

    va_end(args);

    *output = (char *)malloc(length);

    if (*output == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    (*output)[0] = '\0';

    /* Concat strings */
    counter = num;
    va_start(args, num);

    while (counter--) {
        arg = va_arg(args, const char *);
#ifdef SAIL_WIN32
        strcat_s(*output, length, arg);
#else
        strcat(*output, arg);
#endif
    }

    va_end(args);

    return 0;
}

void sail_to_lower(char *str) {

    if (str == NULL) {
        return;
    }

    size_t length = strlen(str);

    for (size_t i = 0; i < length; i++) {
        str[i] = (char)tolower(str[i]);
    }
}

sail_error_t sail_to_wchar(const char *input, wchar_t **output) {

    SAIL_CHECK_PTR(input);
    SAIL_CHECK_PTR(output);

    size_t length = strlen(input);

    *output = (wchar_t *)malloc((length+1) * sizeof(wchar_t));

    if (*output == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

#ifdef SAIL_WIN32
    size_t ret;

    if (mbstowcs_s(&ret, *output, length+1, input, length) != 0) {
        free(*output);
        return SAIL_INVALID_ARGUMENT;
    }
#else
    if (mbstowcs(*output, input, length) == (size_t)-1) {
        free(*output);
        return SAIL_INVALID_ARGUMENT;
    }
#endif

    return 0;
}

const char* sail_pixel_format_to_string(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   return "Unknown";
        case SAIL_PIXEL_FORMAT_SOURCE:    return "Source";

        case SAIL_PIXEL_FORMAT_MONO:      return "Mono";
        case SAIL_PIXEL_FORMAT_GRAYSCALE: return "Grayscale";
        case SAIL_PIXEL_FORMAT_INDEXED:   return "Indexed";
        case SAIL_PIXEL_FORMAT_RGB:       return "RGB";
        case SAIL_PIXEL_FORMAT_YCBCR:     return "YCbCr";
        case SAIL_PIXEL_FORMAT_CMYK:      return "CMYK";
        case SAIL_PIXEL_FORMAT_YCCK:      return "YCCK";
        case SAIL_PIXEL_FORMAT_RGBX:      return "RGBX";
        case SAIL_PIXEL_FORMAT_BGR:       return "BGR";
        case SAIL_PIXEL_FORMAT_BGRX:      return "BGRX";
        case SAIL_PIXEL_FORMAT_XBGR:      return "XBGR";
        case SAIL_PIXEL_FORMAT_XRGB:      return "XRGB";
        case SAIL_PIXEL_FORMAT_RGBA:      return "RGBA";
        case SAIL_PIXEL_FORMAT_BGRA:      return "BGRA";
        case SAIL_PIXEL_FORMAT_ABGR:      return "ABGR";
        case SAIL_PIXEL_FORMAT_ARGB:      return "ARGB";
        case SAIL_PIXEL_FORMAT_RGB565:    return "RGB565";
    }

    return "Unknown";
}

int sail_bits_per_pixel(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   return 0;
        case SAIL_PIXEL_FORMAT_SOURCE:    return 0;

        case SAIL_PIXEL_FORMAT_MONO:      return 1;

        case SAIL_PIXEL_FORMAT_GRAYSCALE: return 8;
        case SAIL_PIXEL_FORMAT_INDEXED:   return 8;

        case SAIL_PIXEL_FORMAT_RGB565:    return 16;

        case SAIL_PIXEL_FORMAT_RGB:       return 24;
        case SAIL_PIXEL_FORMAT_YCBCR:     return 24;
        case SAIL_PIXEL_FORMAT_BGR:       return 24;

        case SAIL_PIXEL_FORMAT_CMYK:      return 32;
        case SAIL_PIXEL_FORMAT_YCCK:      return 32;
        case SAIL_PIXEL_FORMAT_RGBX:      return 32;
        case SAIL_PIXEL_FORMAT_BGRX:      return 32;
        case SAIL_PIXEL_FORMAT_XBGR:      return 32;
        case SAIL_PIXEL_FORMAT_XRGB:      return 32;
        case SAIL_PIXEL_FORMAT_RGBA:      return 32;
        case SAIL_PIXEL_FORMAT_BGRA:      return 32;
        case SAIL_PIXEL_FORMAT_ABGR:      return 32;
        case SAIL_PIXEL_FORMAT_ARGB:      return 32;
    }

    return 0;
}

int sail_bytes_per_line(int width, int pixel_format) {

    const int bits_per_pixel = sail_bits_per_pixel(pixel_format);

    const int add = bits_per_pixel % 8 == 0 ? 0 : 1;

    return (int)(((double)width * bits_per_pixel / 8) + add);
}

int sail_bytes_per_image(const struct sail_image *image) {

    SAIL_CHECK_IMAGE(image);

    return sail_bytes_per_line(image->width, image->pixel_format) * image->height;
}
