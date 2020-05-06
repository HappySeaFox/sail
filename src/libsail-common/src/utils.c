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

#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <errno.h>
    #include <sys/time.h>
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

sail_error_t sail_string_hash(const char *str, unsigned long *hash) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(hash);

    const unsigned char *ustr = (const unsigned char *)str;

    *hash = 5381;
    int c;
    while ((c = *ustr++) != 0) {
        *hash = ((*hash << 5) + *hash) + c; /* hash * 33 + c */
    }

    return 0;
}

sail_error_t sail_pixel_format_to_string(int pixel_format, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:               *result = "UNKNOWN";   return 0;
        case SAIL_PIXEL_FORMAT_SOURCE:                *result = "SOURCE";    return 0;

        case SAIL_PIXEL_FORMAT_BPP1_MONO:             *result = "BPP1-MONO"; return 0;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:          *result = "BPP1-INDEXED"; return 0;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:          *result = "BPP2-INDEXED"; return 0;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:          *result = "BPP4-INDEXED"; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:          *result = "BPP8-INDEXED"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED:         *result = "BPP16-INDEXED"; return 0;

        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:        *result = "BPP2-GRAYSCALE"; return 0;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:        *result = "BPP4-GRAYSCALE"; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:        *result = "BPP8-GRAYSCALE"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       *result = "BPP16-GRAYSCALE"; return 0;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  *result = "BPP4-GRAYSCALE-ALPHA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  *result = "BPP8_GRAYSCALE-ALPHA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: *result = "BPP16-GRAYSCALE-ALPHA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: *result = "BPP32-GRAYSCALE-ALPHA"; return 0;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:          *result = "BPP16-RGB555"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:          *result = "BPP16-BGR555"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:          *result = "BPP16-RGB565"; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:          *result = "BPP16-BGR565"; return 0;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:             *result = "BPP24-RGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:             *result = "BPP24-BGR"; return 0;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:             *result = "BPP48-RGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP48_BGR:             *result = "BPP48-BGR"; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:            *result = "BPP32-RGBX"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:            *result = "BPP32-BGRX"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:            *result = "BPP32-XRGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:            *result = "BPP32-XBGR"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            *result = "BPP32-RGBA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:            *result = "BPP32-BGRA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:            *result = "BPP32-ARGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:            *result = "BPP32-ABGR"; return 0;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:            *result = "BPP64-RGBX"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:            *result = "BPP64-BGRX"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:            *result = "BPP64-XRGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:            *result = "BPP64-XBGR"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            *result = "BPP64-RGBA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:            *result = "BPP64-BGRA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:            *result = "BPP64-ARGB"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:            *result = "BPP64-ABGR"; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK:            *result = "BPP32-CMYK"; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK:            *result = "BPP64-CMYK"; return 0;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:           *result = "BPP24-YCBCR"; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK:            *result = "BPP32-YCCK"; return 0;

        /* Don't use 'default:' so a modern compiler warns us about a missing case switch if any. */
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t sail_pixel_format_from_string(const char *str, int *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (strcmp(str, "UNKNOWN") == 0) {
        *result = SAIL_PIXEL_FORMAT_UNKNOWN;
    } else if (strcmp(str, "SOURCE") == 0) {
        *result = SAIL_PIXEL_FORMAT_SOURCE;
    } else if (strcmp(str, "BPP1-MONO") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP1_MONO;
    } else if (strcmp(str, "BPP1-INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
    } else if (strcmp(str, "BPP2-INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP2_INDEXED;
    } else if (strcmp(str, "BPP4-INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED;
    } else if (strcmp(str, "BPP8-INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    } else if (strcmp(str, "BPP16-INDEXED") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_INDEXED;
    } else if (strcmp(str, "BPP2-GRAYSCALE") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;
    } else if (strcmp(str, "BPP4-GRAYSCALE") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;
    } else if (strcmp(str, "BPP8-GRAYSCALE") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
    } else if (strcmp(str, "BPP16-GRAYSCALE") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
    } else if (strcmp(str, "BPP4-GRAYSCALE-ALPHA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA;
    } else if (strcmp(str, "BPP8_GRAYSCALE-ALPHA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;
    } else if (strcmp(str, "BPP16-GRAYSCALE-ALPHA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
    } else if (strcmp(str, "BPP32-GRAYSCALE-ALPHA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;
    } else if (strcmp(str, "BPP16-RGB555") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_RGB555;
    } else if (strcmp(str, "BPP16-BGR555") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_BGR555;
    } else if (strcmp(str, "BPP16-RGB565") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_RGB565;
    } else if (strcmp(str, "BPP16-BGR565") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP16_BGR565;
    } else if (strcmp(str, "BPP24-RGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP24_RGB;
    } else if (strcmp(str, "BPP24-BGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP24_BGR;
    } else if (strcmp(str, "BPP48-RGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP48_RGB;
    } else if (strcmp(str, "BPP48-BGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP48_BGR;
    } else if (strcmp(str, "BPP32-RGBX") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_RGBX;
    } else if (strcmp(str, "BPP32-BGRX") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_BGRX;
    } else if (strcmp(str, "BPP32-XRGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_XRGB;
    } else if (strcmp(str, "BPP32-XBGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_XBGR;
    } else if (strcmp(str, "BPP32-RGBA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_RGBA;
    } else if (strcmp(str, "BPP32-BGRA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_BGRA;
    } else if (strcmp(str, "BPP32-ARGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_ARGB;
    } else if (strcmp(str, "BPP32-ABGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_ABGR;
    } else if (strcmp(str, "BPP64-RGBX") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_RGBX;
    } else if (strcmp(str, "BPP64-BGRX") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_BGRX;
    } else if (strcmp(str, "BPP64-XRGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_XRGB;
    } else if (strcmp(str, "BPP64-XBGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_XBGR;
    } else if (strcmp(str, "BPP64-RGBA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_RGBA;
    } else if (strcmp(str, "BPP64-BGRA") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_BGRA;
    } else if (strcmp(str, "BPP64-ARGB") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_ARGB;
    } else if (strcmp(str, "BPP64-ABGR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_ABGR;
    } else if (strcmp(str, "BPP32-CMYK") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_CMYK;
    } else if (strcmp(str, "BPP64-CMYK") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP64_CMYK;
    } else if (strcmp(str, "BPP24-YCBCR") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP24_YCBCR;
    } else if (strcmp(str, "BPP32-YCCK") == 0) {
        *result = SAIL_PIXEL_FORMAT_BPP32_YCCK;
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
    SAIL_CHECK_RESULT_PTR(result);

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
    SAIL_CHECK_RESULT_PTR(result);

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
    SAIL_CHECK_RESULT_PTR(result);

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

    SAIL_CHECK_RESULT_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   *result = 0;  break;
        case SAIL_PIXEL_FORMAT_SOURCE:    *result = 0;  break;

        case SAIL_PIXEL_FORMAT_BPP1_MONO: *result = 1; return 0;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:  *result = 1; return 0;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:  *result = 2; return 0;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:  *result = 4; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:  *result = 8; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED: *result = 16; return 0;

        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:  *result = 2; return 0;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:  *result = 4; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:  *result = 8; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: *result = 16; return 0;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  *result = 4; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  *result = 8; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: *result = 16; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: *result = 32; return 0;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:
        case SAIL_PIXEL_FORMAT_BPP16_BGR565: *result = 16; return 0;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR: *result = 24; return 0;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR: *result = 48; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: *result = 32; return 0;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: *result = 64; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK: *result = 32; return 0;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK: *result = 64; return 0;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: *result = 24; return 0;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK: *result = 32; return 0;

        /* Don't use 'default:' so a modern compiler warns us about a missing case switch if any. */
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t sail_bytes_per_line(const struct sail_image *image, int *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_RESULT_PTR(result);

    int bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(image->pixel_format, &bits_per_pixel));

    const int add = bits_per_pixel % 8 == 0 ? 0 : 1;

    *result = (int)(((double)image->width * bits_per_pixel / 8) + add);

    return 0;
}

sail_error_t sail_bytes_per_image(const struct sail_image *image, int *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_RESULT_PTR(result);

    int bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image, &bytes_per_line));

    *result = bytes_per_line * image->height;

    return 0;
}

sail_error_t sail_print_errno(const char *format) {

    SAIL_CHECK_STRING_PTR(format);

    if (strstr(format, "%s") == NULL) {
        return SAIL_INVALID_ARGUMENT;
    }

#ifdef SAIL_WIN32
    char buffer[80];
    strerror_s(buffer, sizeof(buffer), errno);
    SAIL_LOG_ERROR(format, buffer);
#else
    SAIL_LOG_ERROR(format, strerror(errno));
#endif

    return 0;
}

sail_error_t sail_now(uint64_t *result) {

    SAIL_CHECK_RESULT_PTR(result);

#ifdef SAIL_WIN32
    SAIL_THREAD_LOCAL static bool initialized = false;
    SAIL_THREAD_LOCAL static double frequency = 0;

    LARGE_INTEGER li;

    if (!initialized) {
        initialized = true;

        if (!QueryPerformanceFrequency(&li)) {
            SAIL_LOG_ERROR("Failed to get the current time. Error: %d", GetLastError());
            return SAIL_INVALID_ARGUMENT;
        }

        frequency = (double)li.QuadPart / 1000;
    }

    if (!QueryPerformanceCounter(&li)) {
        SAIL_LOG_ERROR("Failed to get the current time. Error: %d", GetLastError());
        return SAIL_INVALID_ARGUMENT;
    }

    *result = (uint64_t)((double)li.QuadPart / frequency);
#else
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        sail_print_errno("Failed to get the current time: %s");
        return 0;
    }

    *result = (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif

    return 0;
}
