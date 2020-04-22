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

#include "sail-common.h"

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

sail_error_t sail_pixel_format_to_string(int pixel_format, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   *result = "UNKNOWN";   break;
        case SAIL_PIXEL_FORMAT_SOURCE:    *result = "SOURCE";    break;

        case SAIL_PIXEL_FORMAT_MONO:      *result = "MONO";      break;
        case SAIL_PIXEL_FORMAT_GRAYSCALE: *result = "GRAYSCALE"; break;
        case SAIL_PIXEL_FORMAT_INDEXED:   *result = "INDEXED";   break;
        case SAIL_PIXEL_FORMAT_RGB:       *result = "RGB";       break;
        case SAIL_PIXEL_FORMAT_YCBCR:     *result = "YCbCr";     break;
        case SAIL_PIXEL_FORMAT_CMYK:      *result = "CMYK";      break;
        case SAIL_PIXEL_FORMAT_YCCK:      *result = "YCCK";      break;
        case SAIL_PIXEL_FORMAT_RGBX:      *result = "RGBX";      break;
        case SAIL_PIXEL_FORMAT_BGR:       *result = "BGR";       break;
        case SAIL_PIXEL_FORMAT_BGRX:      *result = "BGRX";      break;
        case SAIL_PIXEL_FORMAT_XBGR:      *result = "XBGR";      break;
        case SAIL_PIXEL_FORMAT_XRGB:      *result = "XRGB";      break;
        case SAIL_PIXEL_FORMAT_RGBA:      *result = "RGBA";      break;
        case SAIL_PIXEL_FORMAT_BGRA:      *result = "BGRA";      break;
        case SAIL_PIXEL_FORMAT_ABGR:      *result = "ABGR";      break;
        case SAIL_PIXEL_FORMAT_ARGB:      *result = "ARGB";      break;
        case SAIL_PIXEL_FORMAT_RGB565:    *result = "RGB565";    break;

        default: return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    return 0;
}

sail_error_t sail_pixel_format_from_string(const char *str, int *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (strcmp(str, "UNKNOWN") == 0) {
        *result = SAIL_PIXEL_FORMAT_UNKNOWN;
    } else if (strcmp(str, "SOURCE") == 0) {
        *result = SAIL_PIXEL_FORMAT_SOURCE;
    } else if (strcmp(str, "MONO") == 0) {
        *result = SAIL_PIXEL_FORMAT_MONO;
    } else if (strcmp(str, "GRAYSCALE") == 0) {
        *result = SAIL_PIXEL_FORMAT_GRAYSCALE;
    } else if (strcmp(str, "INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_INDEXED;
    } else if (strcmp(str, "RGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_RGB;
    } else if (strcmp(str, "YCBCR") == 0) {
        *result = SAIL_PIXEL_FORMAT_YCBCR;
    } else if (strcmp(str, "CMYK") == 0) {
        *result = SAIL_PIXEL_FORMAT_CMYK;
    } else if (strcmp(str, "YCCK") == 0) {
        *result = SAIL_PIXEL_FORMAT_YCCK;
    } else if (strcmp(str, "RGBX") == 0) {
        *result = SAIL_PIXEL_FORMAT_RGBX;
    } else if (strcmp(str, "BGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BGR;
    } else if (strcmp(str, "BGRX") == 0) {
        *result = SAIL_PIXEL_FORMAT_BGRX;
    } else if (strcmp(str, "XBGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_XBGR;
    } else if (strcmp(str, "XRGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_XRGB;
    } else if (strcmp(str, "RGBA") == 0) {
        *result = SAIL_PIXEL_FORMAT_RGBA;
    } else if (strcmp(str, "BGRA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BGRA;
    } else if (strcmp(str, "ABGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_ABGR;
    } else if (strcmp(str, "ARGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_ARGB;
    } else if (strcmp(str, "RGB565") == 0) {
        *result = SAIL_PIXEL_FORMAT_RGB565;
    } else {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    return 0;
}

sail_error_t sail_image_property_to_string(int image_property, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (image_property) {
        case SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY: *result = "FLIPPED-VERTICALLY"; break;
        case SAIL_IMAGE_PROPERTY_INTERLACED:         *result = "INTERLACED";         break;

        default: return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
    }

    return 0;
}

sail_error_t sail_image_property_from_string(const char *str, int *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
    }

    if (strcmp(str, "FLIPPED-VERTICALLY") == 0) {
        *result = SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY;
    } else if (strcmp(str, "INTERLACED") == 0) {
        *result = SAIL_IMAGE_PROPERTY_INTERLACED;
    } else {
        return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
    }

    return 0;
}

sail_error_t sail_compression_type_to_string(int compression, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (compression) {
        case SAIL_COMPRESSION_RLE: *result = "RLE"; break;

        default: return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    return 0;
}

sail_error_t sail_compression_type_from_string(const char *str, int *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    if (strcmp(str, "RLE") == 0) {
        *result = SAIL_COMPRESSION_RLE;
    } else {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    return 0;
}

sail_error_t sail_plugin_feature_to_string(int plugin_feature, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (plugin_feature) {
        case SAIL_PLUGIN_FEATURE_STATIC:     *result = "STATIC";     break;
        case SAIL_PLUGIN_FEATURE_ANIMATED:   *result = "ANIMATED";   break;
        case SAIL_PLUGIN_FEATURE_MULTIPAGED: *result = "MULTIPAGED"; break;
        case SAIL_PLUGIN_FEATURE_META_INFO:  *result = "META-INFO";  break;
        case SAIL_PLUGIN_FEATURE_EXIF:       *result = "EXIF";       break;
        case SAIL_PLUGIN_FEATURE_INTERLACED: *result = "INTERLACED"; break;

        default: return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
    }

    return 0;
}

sail_error_t sail_plugin_feature_from_string(const char *str, int *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
    }

    if (strcmp(str, "STATIC") == 0) {
        *result = SAIL_PLUGIN_FEATURE_STATIC;
    } else if (strcmp(str, "ANIMATED") == 0) {
        *result = SAIL_PLUGIN_FEATURE_ANIMATED;
    } else if (strcmp(str, "MULTIPAGED") == 0) {
        *result = SAIL_PLUGIN_FEATURE_MULTIPAGED;
    } else if (strcmp(str, "META-INFO") == 0) {
        *result = SAIL_PLUGIN_FEATURE_META_INFO;
    } else if (strcmp(str, "EXIF") == 0) {
        *result = SAIL_PLUGIN_FEATURE_EXIF;
    } else if (strcmp(str, "INTERLACED") == 0) {
        *result = SAIL_PLUGIN_FEATURE_INTERLACED;
    } else {
        return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
    }

    return 0;
}

sail_error_t sail_bits_per_pixel(int pixel_format, int *result) {

    SAIL_CHECK_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   *result = 0;  break;
        case SAIL_PIXEL_FORMAT_SOURCE:    *result = 0;  break;

        case SAIL_PIXEL_FORMAT_MONO:      *result = 1;  break;

        case SAIL_PIXEL_FORMAT_GRAYSCALE: *result = 8;  break;
        case SAIL_PIXEL_FORMAT_INDEXED:   *result = 8;  break;

        case SAIL_PIXEL_FORMAT_RGB565:    *result = 16; break;

        case SAIL_PIXEL_FORMAT_RGB:       *result = 24; break;
        case SAIL_PIXEL_FORMAT_YCBCR:     *result = 24; break;
        case SAIL_PIXEL_FORMAT_BGR:       *result = 24; break;

        case SAIL_PIXEL_FORMAT_CMYK:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_YCCK:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_RGBX:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_BGRX:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_XBGR:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_XRGB:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_RGBA:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_BGRA:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_ABGR:      *result = 32; break;
        case SAIL_PIXEL_FORMAT_ARGB:      *result = 32; break;

        default: return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    return 0;
}

sail_error_t sail_bytes_per_line(const struct sail_image *image, int *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_PTR(result);

    int bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    const int add = bits_per_pixel % 8 == 0 ? 0 : 1;

    *result = (int)(((double)image->width * bits_per_pixel / 8) + add);

    return 0;
}

sail_error_t sail_bytes_per_image(const struct sail_image *image, int *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_PTR(result);

    int bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image, &bytes_per_line));

    *result = bytes_per_line * image->height;

    return 0;
}
