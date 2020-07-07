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
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "sail-common.h"

sail_error_t sail_strdup(const char *input, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    const size_t len = strlen((const char *)input);

    *output = (char *)malloc(len+1);

    if (*output == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(*output, input, len);
    (*output)[len] = '\0';

    return 0;
}

sail_error_t sail_strdup_length(const char *input, size_t length, char **output) {

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

sail_error_t sail_string_hash(const char *str, uint64_t *hash) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(hash);

    const unsigned char *ustr = (const unsigned char *)str;

    *hash = 5381;
    unsigned c;

    while ((c = *ustr++) != 0) {
        *hash = ((*hash << 5) + *hash) + c; /* hash * 33 + c */
    }

    return 0;
}

sail_error_t sail_pixel_format_to_string(enum SailPixelFormat pixel_format, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:               *result = "UNKNOWN";               return 0;
        case SAIL_PIXEL_FORMAT_SOURCE:                *result = "SOURCE";                return 0;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:          *result = "BPP1-INDEXED";          return 0;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:          *result = "BPP2-INDEXED";          return 0;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:          *result = "BPP4-INDEXED";          return 0;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:          *result = "BPP8-INDEXED";          return 0;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED:         *result = "BPP16-INDEXED";         return 0;

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:        *result = "BPP1-GRAYSCALE";        return 0;
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:        *result = "BPP2-GRAYSCALE";        return 0;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:        *result = "BPP4-GRAYSCALE";        return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:        *result = "BPP8-GRAYSCALE";        return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       *result = "BPP16-GRAYSCALE";       return 0;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  *result = "BPP4-GRAYSCALE-ALPHA";  return 0;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  *result = "BPP8-GRAYSCALE-ALPHA";  return 0;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: *result = "BPP16-GRAYSCALE-ALPHA"; return 0;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: *result = "BPP32-GRAYSCALE-ALPHA"; return 0;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:          *result = "BPP16-RGB555";          return 0;
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:          *result = "BPP16-BGR555";          return 0;
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:          *result = "BPP16-RGB565";          return 0;
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:          *result = "BPP16-BGR565";          return 0;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:             *result = "BPP24-RGB";             return 0;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:             *result = "BPP24-BGR";             return 0;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:             *result = "BPP48-RGB";             return 0;
        case SAIL_PIXEL_FORMAT_BPP48_BGR:             *result = "BPP48-BGR";             return 0;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:            *result = "BPP32-RGBX";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:            *result = "BPP32-BGRX";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:            *result = "BPP32-XRGB";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:            *result = "BPP32-XBGR";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            *result = "BPP32-RGBA";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:            *result = "BPP32-BGRA";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:            *result = "BPP32-ARGB";            return 0;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:            *result = "BPP32-ABGR";            return 0;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:            *result = "BPP64-RGBX";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:            *result = "BPP64-BGRX";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:            *result = "BPP64-XRGB";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:            *result = "BPP64-XBGR";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            *result = "BPP64-RGBA";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:            *result = "BPP64-BGRA";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:            *result = "BPP64-ARGB";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:            *result = "BPP64-ABGR";            return 0;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK:            *result = "BPP32-CMYK";            return 0;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK:            *result = "BPP64-CMYK";            return 0;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:           *result = "BPP24-YCBCR";           return 0;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK:            *result = "BPP32-YCCK";            return 0;
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t sail_pixel_format_from_string(const char *str, enum SailPixelFormat *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    /*
     * The switch doesn't look very nice, I know :) However, it's fast and doesn't require
     * extra data structures and initializations. It's not C++11, so we choose between two evils:
     *
     *     1. Introduce extra data structures and their initializations to work with hashes.
     *     2. Use a single ugly looking switch/case.
     */
    switch (hash) {
        case UINT64_C(229442760833397):      *result = SAIL_PIXEL_FORMAT_UNKNOWN;               return 0;
        case UINT64_C(6952734212790):        *result = SAIL_PIXEL_FORMAT_SOURCE;                return 0;
        case UINT64_C(13257949335914442470): *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED;          return 0;
        case UINT64_C(13257950742323060711): *result = SAIL_PIXEL_FORMAT_BPP2_INDEXED;          return 0;
        case UINT64_C(13257953555140297193): *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED;          return 0;
        case UINT64_C(13257959180774770157): *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED;          return 0;
        case UINT64_C(13237225848150241308): *result = SAIL_PIXEL_FORMAT_BPP16_INDEXED;         return 0;
        case UINT64_C(12552958524517323328): *result = SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;        return 0;
        case UINT64_C(12554490103502587777): *result = SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;        return 0;
        case UINT64_C(12557553261473116675): *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;        return 0;
        case UINT64_C(12563679577414174471): *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;        return 0;
        case UINT64_C(8431824423011809526):  *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;       return 0;
        case UINT64_C(9367569596161118198):  *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA;  return 0;
        case UINT64_C(12512997289017890810): *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;  return 0;
        case UINT64_C(3292614999547101481):  *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA; return 0;
        case UINT64_C(5929884054553197927):  *result = SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA; return 0;
        case UINT64_C(13257949683479278997): *result = SAIL_PIXEL_FORMAT_BPP16_RGB555;          return 0;
        case UINT64_C(13257949682853687701): *result = SAIL_PIXEL_FORMAT_BPP16_BGR555;          return 0;
        case UINT64_C(13257949683479279030): *result = SAIL_PIXEL_FORMAT_BPP16_RGB565;          return 0;
        case UINT64_C(13257949682853687734): *result = SAIL_PIXEL_FORMAT_BPP16_BGR565;          return 0;
        case UINT64_C(249836535348735093):   *result = SAIL_PIXEL_FORMAT_BPP24_RGB;             return 0;
        case UINT64_C(249836535348717685):   *result = SAIL_PIXEL_FORMAT_BPP24_BGR;             return 0;
        case UINT64_C(249836535431749563):   *result = SAIL_PIXEL_FORMAT_BPP48_RGB;             return 0;
        case UINT64_C(249836535431732155):   *result = SAIL_PIXEL_FORMAT_BPP48_BGR;             return 0;
        case UINT64_C(8244605667721455340):  *result = SAIL_PIXEL_FORMAT_BPP32_RGBX;            return 0;
        case UINT64_C(8244605667720880876):  *result = SAIL_PIXEL_FORMAT_BPP32_BGRX;            return 0;
        case UINT64_C(8244605667721683084):  *result = SAIL_PIXEL_FORMAT_BPP32_XRGB;            return 0;
        case UINT64_C(8244605667721665676):  *result = SAIL_PIXEL_FORMAT_BPP32_XBGR;            return 0;
        case UINT64_C(8244605667721455317):  *result = SAIL_PIXEL_FORMAT_BPP32_RGBA;            return 0;
        case UINT64_C(8244605667720880853):  *result = SAIL_PIXEL_FORMAT_BPP32_BGRA;            return 0;
        case UINT64_C(8244605667720856533):  *result = SAIL_PIXEL_FORMAT_BPP32_ARGB;            return 0;
        case UINT64_C(8244605667720839125):  *result = SAIL_PIXEL_FORMAT_BPP32_ABGR;            return 0;
        case UINT64_C(8244605671674130033):  *result = SAIL_PIXEL_FORMAT_BPP64_RGBX;            return 0;
        case UINT64_C(8244605671673555569):  *result = SAIL_PIXEL_FORMAT_BPP64_BGRX;            return 0;
        case UINT64_C(8244605671674357777):  *result = SAIL_PIXEL_FORMAT_BPP64_XRGB;            return 0;
        case UINT64_C(8244605671674340369):  *result = SAIL_PIXEL_FORMAT_BPP64_XBGR;            return 0;
        case UINT64_C(8244605671674130010):  *result = SAIL_PIXEL_FORMAT_BPP64_RGBA;            return 0;
        case UINT64_C(8244605671673555546):  *result = SAIL_PIXEL_FORMAT_BPP64_BGRA;            return 0;
        case UINT64_C(8244605671673531226):  *result = SAIL_PIXEL_FORMAT_BPP64_ARGB;            return 0;
        case UINT64_C(8244605671673513818):  *result = SAIL_PIXEL_FORMAT_BPP64_ABGR;            return 0;
        case UINT64_C(8244605667720923565):  *result = SAIL_PIXEL_FORMAT_BPP32_CMYK;            return 0;
        case UINT64_C(8244605671673598258):  *result = SAIL_PIXEL_FORMAT_BPP64_CMYK;            return 0;
        case UINT64_C(13817569962846953645): *result = SAIL_PIXEL_FORMAT_BPP24_YCBCR;           return 0;
        case UINT64_C(8244605667721702563):  *result = SAIL_PIXEL_FORMAT_BPP32_YCCK;            return 0;
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t sail_image_property_to_string(enum SailImageProperties image_property, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (image_property) {
        case SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY: *result = "FLIPPED-VERTICALLY"; return 0;
        case SAIL_IMAGE_PROPERTY_INTERLACED:         *result = "INTERLACED";         return 0;
    }

    return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
}

sail_error_t sail_image_property_from_string(const char *str, enum SailImageProperties *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(17202465669660106453): *result = SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY; return 0;
        case UINT64_C(8244927930303708800):  *result = SAIL_IMAGE_PROPERTY_INTERLACED;         return 0;
    }

    return SAIL_UNSUPPORTED_IMAGE_PROPERTY;
}

sail_error_t sail_compression_type_to_string(enum SailCompressionType compression, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (compression) {
        case SAIL_COMPRESSION_UNSUPPORTED: *result = "UNSUPPORTED"; return 0;
        case SAIL_COMPRESSION_NO:          *result = "NO";          return 0;
        case SAIL_COMPRESSION_RLE:         *result = "RLE";         return 0;
    }

    return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
}

sail_error_t sail_compression_type_from_string(const char *str, enum SailCompressionType *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(13846582888989074574): *result = SAIL_COMPRESSION_UNSUPPORTED; return 0;
        case UINT64_C(5862562):              *result = SAIL_COMPRESSION_NO;          return 0;
        case UINT64_C(193468872):            *result = SAIL_COMPRESSION_RLE;         return 0;
    }

    return SAIL_UNSUPPORTED_COMPRESSION_TYPE;
}

sail_error_t sail_plugin_feature_to_string(enum SailPluginFeatures plugin_feature, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (plugin_feature) {
        case SAIL_PLUGIN_FEATURE_STATIC:     *result = "STATIC";     return 0;
        case SAIL_PLUGIN_FEATURE_ANIMATED:   *result = "ANIMATED";   return 0;
        case SAIL_PLUGIN_FEATURE_MULTIPAGED: *result = "MULTIPAGED"; return 0;
        case SAIL_PLUGIN_FEATURE_META_INFO:  *result = "META-INFO";  return 0;
        case SAIL_PLUGIN_FEATURE_EXIF:       *result = "EXIF";       return 0;
        case SAIL_PLUGIN_FEATURE_INTERLACED: *result = "INTERLACED"; return 0;
        case SAIL_PLUGIN_FEATURE_ICCP:       *result = "ICCP";       return 0;
    }

    return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
}

sail_error_t sail_plugin_feature_from_string(const char *str, enum SailPluginFeatures *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(6952739426029):       *result = SAIL_PLUGIN_FEATURE_STATIC;     return 0;
        case UINT64_C(7570758658679240):    *result = SAIL_PLUGIN_FEATURE_ANIMATED;   return 0;
        case UINT64_C(8245123099178649521): *result = SAIL_PLUGIN_FEATURE_MULTIPAGED; return 0;
        case UINT64_C(249851542786266181):  *result = SAIL_PLUGIN_FEATURE_META_INFO;  return 0;
        case UINT64_C(6384018865):          *result = SAIL_PLUGIN_FEATURE_EXIF;       return 0;
        case UINT64_C(8244927930303708800): *result = SAIL_PLUGIN_FEATURE_INTERLACED; return 0;
        case UINT64_C(6384139556):          *result = SAIL_PLUGIN_FEATURE_ICCP;       return 0;
    }

    return SAIL_UNSUPPORTED_PLUGIN_FEATURE;
}

sail_error_t sail_bits_per_pixel(enum SailPixelFormat pixel_format, unsigned *result) {

    SAIL_CHECK_RESULT_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   *result = 0;  break;
        case SAIL_PIXEL_FORMAT_SOURCE:    *result = 0;  break;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:  *result = 1; return 0;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:  *result = 2; return 0;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:  *result = 4; return 0;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:  *result = 8; return 0;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED: *result = 16; return 0;

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:  *result = 1; return 0;
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
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format, unsigned *result) {

    if (width == 0) {
        return SAIL_INVALID_ARGUMENT;
    }

    SAIL_CHECK_RESULT_PTR(result);

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(pixel_format, &bits_per_pixel));

    const int add = bits_per_pixel % 8 == 0 ? 0 : 1;

    *result = (unsigned)(((double)width * bits_per_pixel / 8) + add);

    return 0;
}

sail_error_t sail_bytes_per_image(const struct sail_image *image, unsigned *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_RESULT_PTR(result);

    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

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

bool sail_path_exists(const char *path) {

    SAIL_CHECK_PATH_PTR(path);

#ifdef SAIL_WIN32
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat lib_attribs;
    return stat(path, &lib_attribs) == 0;
#endif
}

bool sail_is_dir(const char *path) {

    SAIL_CHECK_PATH_PTR(path);

#ifdef SAIL_WIN32
    const DWORD lib_attribs = GetFileAttributes(path);

    if (lib_attribs == INVALID_FILE_ATTRIBUTES) {
        SAIL_LOG_DEBUG("Failed to get the attributes of '%s'. Error: %d", path, GetLastError());
        return false;
    }

    return lib_attribs & FILE_ATTRIBUTE_DIRECTORY;
#else
    struct stat lib_attribs;

    if (stat(path, &lib_attribs) != 0) {
        SAIL_LOG_DEBUG("Failed to get the attributes of '%s': %s", path, strerror(errno));
        return false;
    }

    return S_ISDIR(lib_attribs.st_mode);
#endif
}

bool sail_is_file(const char *path) {

    SAIL_CHECK_PATH_PTR(path);

#ifdef SAIL_WIN32
    const DWORD lib_attribs = GetFileAttributes(path);

    if (lib_attribs == INVALID_FILE_ATTRIBUTES) {
        SAIL_LOG_DEBUG("Failed to get the attributes of '%s'. Error: %d", path, GetLastError());
        return false;
    }

    return !(lib_attribs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat lib_attribs;

    if (stat(path, &lib_attribs) != 0) {
        SAIL_LOG_DEBUG("Failed to get the attributes of '%s': %s", path, strerror(errno));
        return false;
    }

    return S_ISREG(lib_attribs.st_mode);
#endif
}
