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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef SAIL_WIN32
    #include <io.h>
    #include <windows.h>
#else
    #include <errno.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

#include "sail-common.h"

sail_status_t sail_memdup(const void *input, size_t input_size, void **output) {

    if (input == NULL) {
        *output = NULL;
        return SAIL_OK;
    }

    if (input_size == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(sail_malloc(input_size, output));

    memcpy(*output, input, input_size);

    return SAIL_OK;
}

sail_status_t sail_strdup(const char *input, char **output) {

    if (input == NULL) {
        *output = NULL;
        return SAIL_OK;
    }

    void *ptr;
    SAIL_TRY(sail_memdup(input, strlen(input) + 1, &ptr));
    *output = ptr;

    return SAIL_OK;
}

sail_status_t sail_strdup_length(const char *input, size_t length, char **output) {

    if (input == NULL) {
        *output = NULL;
        return SAIL_OK;
    }

    if (length == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    void *ptr;
    SAIL_TRY(sail_malloc(length+1, &ptr));
    *output = ptr;

    memcpy(*output, input, length);
    (*output)[length] = '\0';

    return SAIL_OK;
}

sail_status_t sail_concat(char **output, int num, ...) {

    if (num < 1) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_STRING_PTR(output);

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

    void *ptr;
    SAIL_TRY(sail_malloc(length, &ptr));
    *output = ptr;

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

    return SAIL_OK;
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

sail_status_t sail_to_wchar(const char *input, wchar_t **output) {

    SAIL_CHECK_STRING_PTR(input);
    SAIL_CHECK_STRING_PTR(output);

    size_t length = strlen(input);

    void *ptr;
    SAIL_TRY(sail_malloc((length+1) * sizeof(wchar_t), &ptr));
    wchar_t *output_local = ptr;

#ifdef SAIL_WIN32
    size_t ret;

    if (mbstowcs_s(&ret, output_local, length+1, input, length) != 0) {
        sail_free(output_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#else
    if (mbstowcs(output_local, input, length) == (size_t)-1) {
        sail_free(output_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif

    *output = output_local;

    return SAIL_OK;
}

sail_status_t sail_string_hash(const char *str, uint64_t *hash) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(hash);

    const unsigned char *ustr = (const unsigned char *)str;

    *hash = 5381;
    unsigned c;

    while ((c = *ustr++) != 0) {
        *hash = ((*hash << 5) + *hash) + c; /* hash * 33 + c */
    }

    return SAIL_OK;
}

sail_status_t sail_pixel_format_to_string(enum SailPixelFormat pixel_format, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:               *result = "UNKNOWN";               return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP1:                  *result = "BPP1";                  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2:                  *result = "BPP2";                  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4:                  *result = "BPP4";                  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8:                  *result = "BPP8";                  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16:                 *result = "BPP16";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP24:                 *result = "BPP24";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32:                 *result = "BPP32";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48:                 *result = "BPP48";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64:                 *result = "BPP64";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP72:                 *result = "BPP72";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP96:                 *result = "BPP96";                 return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP128:                *result = "BPP128";                return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:          *result = "BPP1-INDEXED";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:          *result = "BPP2-INDEXED";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:          *result = "BPP4-INDEXED";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:          *result = "BPP8-INDEXED";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED:         *result = "BPP16-INDEXED";         return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:        *result = "BPP1-GRAYSCALE";        return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:        *result = "BPP2-GRAYSCALE";        return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:        *result = "BPP4-GRAYSCALE";        return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:        *result = "BPP8-GRAYSCALE";        return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       *result = "BPP16-GRAYSCALE";       return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  *result = "BPP4-GRAYSCALE-ALPHA";  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  *result = "BPP8-GRAYSCALE-ALPHA";  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: *result = "BPP16-GRAYSCALE-ALPHA"; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: *result = "BPP32-GRAYSCALE-ALPHA"; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:          *result = "BPP16-RGB555";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:          *result = "BPP16-BGR555";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:          *result = "BPP16-RGB565";          return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:          *result = "BPP16-BGR565";          return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:             *result = "BPP24-RGB";             return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:             *result = "BPP24-BGR";             return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:             *result = "BPP48-RGB";             return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48_BGR:             *result = "BPP48-BGR";             return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:            *result = "BPP32-RGBX";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:            *result = "BPP32-BGRX";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:            *result = "BPP32-XRGB";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:            *result = "BPP32-XBGR";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            *result = "BPP32-RGBA";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:            *result = "BPP32-BGRA";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:            *result = "BPP32-ARGB";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:            *result = "BPP32-ABGR";            return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:            *result = "BPP64-RGBX";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:            *result = "BPP64-BGRX";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:            *result = "BPP64-XRGB";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:            *result = "BPP64-XBGR";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            *result = "BPP64-RGBA";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:            *result = "BPP64-BGRA";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:            *result = "BPP64-ARGB";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:            *result = "BPP64-ABGR";            return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK:            *result = "BPP32-CMYK";            return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK:            *result = "BPP64-CMYK";            return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:           *result = "BPP24-YCBCR";           return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK:            *result = "BPP32-YCCK";            return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LAB:         *result = "BPP24-CIE-LAB";         return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48_CIE_LAB:         *result = "BPP48-CIE-LAB";         return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

sail_status_t sail_pixel_format_from_string(const char *str, enum SailPixelFormat *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
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
        case UINT64_C(229442760833397):      *result = SAIL_PIXEL_FORMAT_UNKNOWN;               return SAIL_OK;

        case UINT64_C(6383902552):           *result = SAIL_PIXEL_FORMAT_BPP1;                  return SAIL_OK;
        case UINT64_C(6383902553):           *result = SAIL_PIXEL_FORMAT_BPP2;                  return SAIL_OK;
        case UINT64_C(6383902555):           *result = SAIL_PIXEL_FORMAT_BPP4;                  return SAIL_OK;
        case UINT64_C(6383902559):           *result = SAIL_PIXEL_FORMAT_BPP8;                  return SAIL_OK;
        case UINT64_C(210668784270):         *result = SAIL_PIXEL_FORMAT_BPP16;                 return SAIL_OK;
        case UINT64_C(210668784301):         *result = SAIL_PIXEL_FORMAT_BPP24;                 return SAIL_OK;
        case UINT64_C(210668784332):         *result = SAIL_PIXEL_FORMAT_BPP32;                 return SAIL_OK;
        case UINT64_C(210668784371):         *result = SAIL_PIXEL_FORMAT_BPP48;                 return SAIL_OK;
        case UINT64_C(210668784433):         *result = SAIL_PIXEL_FORMAT_BPP64;                 return SAIL_OK;
        case UINT64_C(210668784464):         *result = SAIL_PIXEL_FORMAT_BPP72;                 return SAIL_OK;
        case UINT64_C(210668784534):         *result = SAIL_PIXEL_FORMAT_BPP96;                 return SAIL_OK;
        case UINT64_C(6952069880834):        *result = SAIL_PIXEL_FORMAT_BPP128;                return SAIL_OK;

        case UINT64_C(13257949335914442470): *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED;          return SAIL_OK;
        case UINT64_C(13257950742323060711): *result = SAIL_PIXEL_FORMAT_BPP2_INDEXED;          return SAIL_OK;
        case UINT64_C(13257953555140297193): *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED;          return SAIL_OK;
        case UINT64_C(13257959180774770157): *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED;          return SAIL_OK;
        case UINT64_C(13237225848150241308): *result = SAIL_PIXEL_FORMAT_BPP16_INDEXED;         return SAIL_OK;

        case UINT64_C(12552958524517323328): *result = SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;        return SAIL_OK;
        case UINT64_C(12554490103502587777): *result = SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;        return SAIL_OK;
        case UINT64_C(12557553261473116675): *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;        return SAIL_OK;
        case UINT64_C(12563679577414174471): *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;        return SAIL_OK;
        case UINT64_C(8431824423011809526):  *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;       return SAIL_OK;

        case UINT64_C(9367569596161118198):  *result = SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA;  return SAIL_OK;
        case UINT64_C(12512997289017890810): *result = SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;  return SAIL_OK;
        case UINT64_C(3292614999547101481):  *result = SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA; return SAIL_OK;
        case UINT64_C(5929884054553197927):  *result = SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA; return SAIL_OK;

        case UINT64_C(13257949683479278997): *result = SAIL_PIXEL_FORMAT_BPP16_RGB555;          return SAIL_OK;
        case UINT64_C(13257949682853687701): *result = SAIL_PIXEL_FORMAT_BPP16_BGR555;          return SAIL_OK;
        case UINT64_C(13257949683479279030): *result = SAIL_PIXEL_FORMAT_BPP16_RGB565;          return SAIL_OK;
        case UINT64_C(13257949682853687734): *result = SAIL_PIXEL_FORMAT_BPP16_BGR565;          return SAIL_OK;

        case UINT64_C(249836535348735093):   *result = SAIL_PIXEL_FORMAT_BPP24_RGB;             return SAIL_OK;
        case UINT64_C(249836535348717685):   *result = SAIL_PIXEL_FORMAT_BPP24_BGR;             return SAIL_OK;

        case UINT64_C(249836535431749563):   *result = SAIL_PIXEL_FORMAT_BPP48_RGB;             return SAIL_OK;
        case UINT64_C(249836535431732155):   *result = SAIL_PIXEL_FORMAT_BPP48_BGR;             return SAIL_OK;

        case UINT64_C(8244605667721455340):  *result = SAIL_PIXEL_FORMAT_BPP32_RGBX;            return SAIL_OK;
        case UINT64_C(8244605667720880876):  *result = SAIL_PIXEL_FORMAT_BPP32_BGRX;            return SAIL_OK;
        case UINT64_C(8244605667721683084):  *result = SAIL_PIXEL_FORMAT_BPP32_XRGB;            return SAIL_OK;
        case UINT64_C(8244605667721665676):  *result = SAIL_PIXEL_FORMAT_BPP32_XBGR;            return SAIL_OK;
        case UINT64_C(8244605667721455317):  *result = SAIL_PIXEL_FORMAT_BPP32_RGBA;            return SAIL_OK;
        case UINT64_C(8244605667720880853):  *result = SAIL_PIXEL_FORMAT_BPP32_BGRA;            return SAIL_OK;
        case UINT64_C(8244605667720856533):  *result = SAIL_PIXEL_FORMAT_BPP32_ARGB;            return SAIL_OK;
        case UINT64_C(8244605667720839125):  *result = SAIL_PIXEL_FORMAT_BPP32_ABGR;            return SAIL_OK;

        case UINT64_C(8244605671674130033):  *result = SAIL_PIXEL_FORMAT_BPP64_RGBX;            return SAIL_OK;
        case UINT64_C(8244605671673555569):  *result = SAIL_PIXEL_FORMAT_BPP64_BGRX;            return SAIL_OK;
        case UINT64_C(8244605671674357777):  *result = SAIL_PIXEL_FORMAT_BPP64_XRGB;            return SAIL_OK;
        case UINT64_C(8244605671674340369):  *result = SAIL_PIXEL_FORMAT_BPP64_XBGR;            return SAIL_OK;
        case UINT64_C(8244605671674130010):  *result = SAIL_PIXEL_FORMAT_BPP64_RGBA;            return SAIL_OK;
        case UINT64_C(8244605671673555546):  *result = SAIL_PIXEL_FORMAT_BPP64_BGRA;            return SAIL_OK;
        case UINT64_C(8244605671673531226):  *result = SAIL_PIXEL_FORMAT_BPP64_ARGB;            return SAIL_OK;
        case UINT64_C(8244605671673513818):  *result = SAIL_PIXEL_FORMAT_BPP64_ABGR;            return SAIL_OK;

        case UINT64_C(8244605667720923565):  *result = SAIL_PIXEL_FORMAT_BPP32_CMYK;            return SAIL_OK;
        case UINT64_C(8244605671673598258):  *result = SAIL_PIXEL_FORMAT_BPP64_CMYK;            return SAIL_OK;

        case UINT64_C(13817569962846953645): *result = SAIL_PIXEL_FORMAT_BPP24_YCBCR;           return SAIL_OK;

        case UINT64_C(8244605667721702563):  *result = SAIL_PIXEL_FORMAT_BPP32_YCCK;            return SAIL_OK;

        case UINT64_C(13237269438873232231): *result = SAIL_PIXEL_FORMAT_BPP24_CIE_LAB;         return SAIL_OK;
        case UINT64_C(13237367887476509101): *result = SAIL_PIXEL_FORMAT_BPP48_CIE_LAB;         return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

sail_status_t sail_image_property_to_string(enum SailImageProperty image_property, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (image_property) {
        case SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY: *result = "FLIPPED-VERTICALLY"; return SAIL_OK;
        case SAIL_IMAGE_PROPERTY_INTERLACED:         *result = "INTERLACED";         return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY);
}

sail_status_t sail_image_property_from_string(const char *str, enum SailImageProperty *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(17202465669660106453): *result = SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY; return SAIL_OK;
        case UINT64_C(8244927930303708800):  *result = SAIL_IMAGE_PROPERTY_INTERLACED;         return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY);
}

sail_status_t sail_compression_to_string(enum SailCompression compression, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (compression) {
        case SAIL_COMPRESSION_UNSUPPORTED:   *result = "UNSUPPORTED";   return SAIL_OK;
        case SAIL_COMPRESSION_UNKNOWN:       *result = "UNKNOWN";       return SAIL_OK;
        case SAIL_COMPRESSION_NONE:          *result = "NONE";          return SAIL_OK;
        case SAIL_COMPRESSION_ADOBE_DEFLATE: *result = "ADOBE-DEFLATE"; return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX3:    *result = "CCITT-FAX3";    return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_FAX4:    *result = "CCITT-FAX4";    return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLE:     *result = "CCITT-RLE";     return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_RLEW:    *result = "CCITT-RLEW";    return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T4:      *result = "CCITT-T4";      return SAIL_OK;
        case SAIL_COMPRESSION_CCITT_T6:      *result = "CCITT-T6";      return SAIL_OK;
        case SAIL_COMPRESSION_DCS:           *result = "DCS";           return SAIL_OK;
        case SAIL_COMPRESSION_DEFLATE:       *result = "DEFLATE";       return SAIL_OK;
        case SAIL_COMPRESSION_IT8_BL:        *result = "IT8-BL";        return SAIL_OK;
        case SAIL_COMPRESSION_IT8_CTPAD:     *result = "IT8-CTPAD";     return SAIL_OK;
        case SAIL_COMPRESSION_IT8_LW:        *result = "IT8-LW";        return SAIL_OK;
        case SAIL_COMPRESSION_IT8_MP:        *result = "IT8-MP";        return SAIL_OK;
        case SAIL_COMPRESSION_JBIG:          *result = "JBIG";          return SAIL_OK;
        case SAIL_COMPRESSION_JPEG:          *result = "JPEG";          return SAIL_OK;
        case SAIL_COMPRESSION_JPEG2000:      *result = "JPEG2000";      return SAIL_OK;
        case SAIL_COMPRESSION_LERC:          *result = "LERC";          return SAIL_OK;
        case SAIL_COMPRESSION_LZMA:          *result = "LZMA";          return SAIL_OK;
        case SAIL_COMPRESSION_LZW:           *result = "LZW";           return SAIL_OK;
        case SAIL_COMPRESSION_NEXT:          *result = "NEXT";          return SAIL_OK;
        case SAIL_COMPRESSION_OJPEG:         *result = "OJPEG";         return SAIL_OK;
        case SAIL_COMPRESSION_PACKBITS:      *result = "PACKBITS";      return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_FILM:    *result = "PIXAR-FILM";    return SAIL_OK;
        case SAIL_COMPRESSION_PIXAR_LOG:     *result = "PIXAR-LOG";     return SAIL_OK;
        case SAIL_COMPRESSION_RLE:           *result = "RLE";           return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG:       *result = "SGI-LOG";       return SAIL_OK;
        case SAIL_COMPRESSION_SGI_LOG24:     *result = "SGI-LOG24";     return SAIL_OK;
        case SAIL_COMPRESSION_T43:           *result = "T43";           return SAIL_OK;
        case SAIL_COMPRESSION_T85:           *result = "T85";           return SAIL_OK;
        case SAIL_COMPRESSION_THUNDERSCAN:   *result = "THUNDERSCAN";   return SAIL_OK;
        case SAIL_COMPRESSION_WEBP:          *result = "WEBP";          return SAIL_OK;
        case SAIL_COMPRESSION_ZSTD:          *result = "ZSTD";          return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
}

sail_status_t sail_compression_from_string(const char *str, enum SailCompression *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(13846582888989074574): *result = SAIL_COMPRESSION_UNSUPPORTED;   return SAIL_OK;
        case UINT64_C(229442760833397):      *result = SAIL_COMPRESSION_UNKNOWN;       return SAIL_OK;
        case UINT64_C(6384332661):           *result = SAIL_COMPRESSION_NONE;          return SAIL_OK;
        case UINT64_C(10962109560604417378): *result = SAIL_COMPRESSION_ADOBE_DEFLATE; return SAIL_OK;
        case UINT64_C(8244633541513328571):  *result = SAIL_COMPRESSION_CCITT_FAX3;    return SAIL_OK;
        case UINT64_C(8244633541513328572):  *result = SAIL_COMPRESSION_CCITT_FAX4;    return SAIL_OK;
        case UINT64_C(249837380045871852):   *result = SAIL_COMPRESSION_CCITT_RLE;     return SAIL_OK;
        case UINT64_C(8244633541513771203):  *result = SAIL_COMPRESSION_CCITT_RLEW;    return SAIL_OK;
        case UINT64_C(7570829698359793):     *result = SAIL_COMPRESSION_CCITT_T4;      return SAIL_OK;
        case UINT64_C(7570829698359795):     *result = SAIL_COMPRESSION_CCITT_T6;      return SAIL_OK;
        case UINT64_C(193453343):            *result = SAIL_COMPRESSION_DCS;           return SAIL_OK;
        case UINT64_C(229420447642554):      *result = SAIL_COMPRESSION_DEFLATE;       return SAIL_OK;
        case UINT64_C(6952347705973):        *result = SAIL_COMPRESSION_IT8_BL;        return SAIL_OK;
        case UINT64_C(249846519511114451):   *result = SAIL_COMPRESSION_IT8_CTPAD;     return SAIL_OK;
        case UINT64_C(6952347706314):        *result = SAIL_COMPRESSION_IT8_LW;        return SAIL_OK;
        case UINT64_C(6952347706340):        *result = SAIL_COMPRESSION_IT8_MP;        return SAIL_OK;
        case UINT64_C(6384174593):           *result = SAIL_COMPRESSION_JBIG;          return SAIL_OK;
        case UINT64_C(6384189707):           *result = SAIL_COMPRESSION_JPEG;          return SAIL_OK;
        case UINT64_C(7571144643365901):     *result = SAIL_COMPRESSION_JPEG2000;      return SAIL_OK;
        case UINT64_C(6384250027):           *result = SAIL_COMPRESSION_LERC;          return SAIL_OK;
        case UINT64_C(6384272729):           *result = SAIL_COMPRESSION_LZMA;          return SAIL_OK;
        case UINT64_C(193462818):            *result = SAIL_COMPRESSION_LZW;           return SAIL_OK;
        case UINT64_C(6384322116):           *result = SAIL_COMPRESSION_NEXT;          return SAIL_OK;
        case UINT64_C(210683986298):         *result = SAIL_COMPRESSION_OJPEG;         return SAIL_OK;
        case UINT64_C(7571380909080566):     *result = SAIL_COMPRESSION_PACKBITS;      return SAIL_OK;
        case UINT64_C(8245245943922754206):  *result = SAIL_COMPRESSION_PIXAR_FILM;    return SAIL_OK;
        case UINT64_C(249855937694635640):   *result = SAIL_COMPRESSION_PIXAR_LOG;     return SAIL_OK;
        case UINT64_C(193468872):            *result = SAIL_COMPRESSION_RLE;           return SAIL_OK;
        case UINT64_C(229439900388407):      *result = SAIL_COMPRESSION_SGI_LOG;       return SAIL_OK;
        case UINT64_C(249860051522976925):   *result = SAIL_COMPRESSION_SGI_LOG24;     return SAIL_OK;
        case UINT64_C(193470240):            *result = SAIL_COMPRESSION_T43;           return SAIL_OK;
        case UINT64_C(193470374):            *result = SAIL_COMPRESSION_T85;           return SAIL_OK;
        case UINT64_C(13844775339661004164): *result = SAIL_COMPRESSION_THUNDERSCAN;   return SAIL_OK;
        case UINT64_C(6384644819):           *result = SAIL_COMPRESSION_WEBP;          return SAIL_OK;
        case UINT64_C(6384768458):           *result = SAIL_COMPRESSION_ZSTD;          return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
}

sail_status_t sail_meta_data_to_string(enum SailMetaData meta_data, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (meta_data) {
        case SAIL_META_DATA_UNKNOWN:         *result = "Unknown";         return SAIL_OK;

        case SAIL_META_DATA_ARTIST:          *result = "Artist";          return SAIL_OK;
        case SAIL_META_DATA_AUTHOR:          *result = "Author";          return SAIL_OK;
        case SAIL_META_DATA_COMMENT:         *result = "Comment";         return SAIL_OK;
        case SAIL_META_DATA_COMPUTER:        *result = "Computer";        return SAIL_OK;
        case SAIL_META_DATA_COPYRIGHT:       *result = "Copyright";       return SAIL_OK;
        case SAIL_META_DATA_CREATION_TIME:   *result = "Creation Time";   return SAIL_OK;
        case SAIL_META_DATA_DESCRIPTION:     *result = "Description";     return SAIL_OK;
        case SAIL_META_DATA_DISCLAIMER:      *result = "Disclaimer";      return SAIL_OK;
        case SAIL_META_DATA_DOCUMENT:        *result = "Document";        return SAIL_OK;
        case SAIL_META_DATA_EXIF:            *result = "EXIF";            return SAIL_OK;
        case SAIL_META_DATA_HEX_EXIF:        *result = "Hex EXIF";        return SAIL_OK;
        case SAIL_META_DATA_HEX_IPTC:        *result = "Hex IPTC";        return SAIL_OK;
        case SAIL_META_DATA_HEX_XMP:         *result = "Hex XMP";         return SAIL_OK;
        case SAIL_META_DATA_LABEL:           *result = "Label";           return SAIL_OK;
        case SAIL_META_DATA_MAKE:            *result = "Make";            return SAIL_OK;
        case SAIL_META_DATA_MODEL:           *result = "Model";           return SAIL_OK;
        case SAIL_META_DATA_NAME:            *result = "Name";            return SAIL_OK;
        case SAIL_META_DATA_PRINTER:         *result = "Printer";         return SAIL_OK;
        case SAIL_META_DATA_SOFTWARE:        *result = "Software";        return SAIL_OK;
        case SAIL_META_DATA_SOURCE:          *result = "Source";          return SAIL_OK;
        case SAIL_META_DATA_TITLE:           *result = "Title";           return SAIL_OK;
        case SAIL_META_DATA_URL:             *result = "URL";             return SAIL_OK;
        case SAIL_META_DATA_WARNING:         *result = "Warning";         return SAIL_OK;
        case SAIL_META_DATA_XMP:             *result = "XMP";             return SAIL_OK;
    }

    *result = "UNKNOWN";
    return SAIL_OK;
}

sail_status_t sail_meta_data_from_string(const char *str, enum SailMetaData *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(6952072423676):        *result = SAIL_META_DATA_ARTIST;          return SAIL_OK;
        case UINT64_C(6952075980216):        *result = SAIL_META_DATA_AUTHOR;          return SAIL_OK;
        case UINT64_C(229420847338040):      *result = SAIL_META_DATA_COMMENT;         return SAIL_OK;
        case UINT64_C(7570887966294228):     *result = SAIL_META_DATA_COMPUTER;        return SAIL_OK;
        case UINT64_C(249839307110380862):   *result = SAIL_META_DATA_COPYRIGHT;       return SAIL_OK;
        case UINT64_C(16658027699238675945): *result = SAIL_META_DATA_CREATION_TIME;   return SAIL_OK;
        case UINT64_C(13821659157043486569): *result = SAIL_META_DATA_DESCRIPTION;     return SAIL_OK;
        case UINT64_C(8244735206874071778):  *result = SAIL_META_DATA_DISCLAIMER;      return SAIL_OK;
        case UINT64_C(7570930199009348):     *result = SAIL_META_DATA_DOCUMENT;        return SAIL_OK;
        case UINT64_C(6384018865):           *result = SAIL_META_DATA_EXIF;            return SAIL_OK;
        case UINT64_C(7571088477688630):     *result = SAIL_META_DATA_HEX_EXIF;        return SAIL_OK;
        case UINT64_C(7571088477824026):     *result = SAIL_META_DATA_HEX_IPTC;        return SAIL_OK;
        case UINT64_C(229426923586655):      *result = SAIL_META_DATA_HEX_XMP;         return SAIL_OK;
        case UINT64_C(210681275781):         *result = SAIL_META_DATA_LABEL;           return SAIL_OK;
        case UINT64_C(6384317315):           *result = SAIL_META_DATA_MAKE;            return SAIL_OK;
        case UINT64_C(210682966998):         *result = SAIL_META_DATA_MODEL;           return SAIL_OK;
        case UINT64_C(6384353318):           *result = SAIL_META_DATA_NAME;            return SAIL_OK;
        case UINT64_C(229437749136105):      *result = SAIL_META_DATA_PRINTER;         return SAIL_OK;
        case UINT64_C(7571569592229392):     *result = SAIL_META_DATA_SOFTWARE;        return SAIL_OK;
        case UINT64_C(6952773348182):        *result = SAIL_META_DATA_SOURCE;          return SAIL_OK;
        case UINT64_C(210691070471):         *result = SAIL_META_DATA_TITLE;           return SAIL_OK;
        case UINT64_C(193472344):            *result = SAIL_META_DATA_URL;             return SAIL_OK;
        case UINT64_C(229446134771803):      *result = SAIL_META_DATA_WARNING;         return SAIL_OK;
        case UINT64_C(193475450):            *result = SAIL_META_DATA_XMP;             return SAIL_OK;
    }

    *result = SAIL_META_DATA_UNKNOWN;
    return SAIL_OK;
}

sail_status_t sail_codec_feature_to_string(enum SailCodecFeature codec_feature, const char **result) {

    SAIL_CHECK_STRING_PTR(result);

    switch (codec_feature) {
        case SAIL_CODEC_FEATURE_STATIC:      *result = "STATIC";      return SAIL_OK;
        case SAIL_CODEC_FEATURE_ANIMATED:    *result = "ANIMATED";    return SAIL_OK;
        case SAIL_CODEC_FEATURE_MULTI_FRAME: *result = "MULTI-FRAME"; return SAIL_OK;
        case SAIL_CODEC_FEATURE_META_DATA:   *result = "META-DATA";   return SAIL_OK;
        case SAIL_CODEC_FEATURE_EXIF:        *result = "EXIF";        return SAIL_OK;
        case SAIL_CODEC_FEATURE_INTERLACED:  *result = "INTERLACED";  return SAIL_OK;
        case SAIL_CODEC_FEATURE_ICCP:        *result = "ICCP";        return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_CODEC_FEATURE);
}

sail_status_t sail_codec_feature_from_string(const char *str, enum SailCodecFeature *result) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_RESULT_PTR(result);

    if (strlen(str) == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
    }

    uint64_t hash;
    SAIL_TRY(sail_string_hash(str, &hash));

    switch (hash) {
        case UINT64_C(6952739426029):        *result = SAIL_CODEC_FEATURE_STATIC;      return SAIL_OK;
        case UINT64_C(7570758658679240):     *result = SAIL_CODEC_FEATURE_ANIMATED;    return SAIL_OK;
        case UINT64_C(13834645239598293736): *result = SAIL_CODEC_FEATURE_MULTI_FRAME; return SAIL_OK;
        case UINT64_C(249851542786072787):   *result = SAIL_CODEC_FEATURE_META_DATA;   return SAIL_OK;
        case UINT64_C(6384018865):           *result = SAIL_CODEC_FEATURE_EXIF;        return SAIL_OK;
        case UINT64_C(8244927930303708800):  *result = SAIL_CODEC_FEATURE_INTERLACED;  return SAIL_OK;
        case UINT64_C(6384139556):           *result = SAIL_CODEC_FEATURE_ICCP;        return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_CODEC_FEATURE);
}

sail_status_t sail_bits_per_pixel(enum SailPixelFormat pixel_format, unsigned *result) {

    SAIL_CHECK_RESULT_PTR(result);

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);

        case SAIL_PIXEL_FORMAT_BPP1:   *result = 1;   return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2:   *result = 2;   return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4:   *result = 4;   return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8:   *result = 8;   return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16:  *result = 16;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP24:  *result = 24;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32:  *result = 32;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48:  *result = 48;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64:  *result = 64;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP72:  *result = 72;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP96:  *result = 96;  return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP128: *result = 128; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:  *result = 1; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:  *result = 2; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:  *result = 4; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:  *result = 8; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED: *result = 16; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:  *result = 1; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:  *result = 2; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:  *result = 4; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:  *result = 8; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: *result = 16; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  *result = 4; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  *result = 8; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: *result = 16; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: *result = 32; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:
        case SAIL_PIXEL_FORMAT_BPP16_BGR565: *result = 16; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR: *result = 24; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR: *result = 48; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: *result = 32; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: *result = 64; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK: *result = 32; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK: *result = 64; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: *result = 24; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK: *result = 32; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LAB: *result = 24; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48_CIE_LAB: *result = 48; return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

enum SailPixelFormatComparisonPrivate {
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER,
};

static sail_status_t sail_compare_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2,
                                                    enum SailPixelFormatComparisonPrivate op, bool *result) {

    SAIL_CHECK_PTR(result);

    unsigned pixel_format_bits1;
    SAIL_TRY(sail_bits_per_pixel(pixel_format1, &pixel_format_bits1));

    unsigned pixel_format_bits2;
    SAIL_TRY(sail_bits_per_pixel(pixel_format2, &pixel_format_bits2));

    switch(op) {
        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS:
            *result = pixel_format_bits1 < pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL:
            *result = pixel_format_bits1 == pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER:
            *result = pixel_format_bits1 > pixel_format_bits2;
        break;
    }

    return SAIL_OK;
}

sail_status_t sail_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL, result));

    return SAIL_OK;
}

sail_status_t sail_greater_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER, result));

    return SAIL_OK;
}

sail_status_t sail_less_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS, result));

    return SAIL_OK;
}

sail_status_t sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format, unsigned *result) {

    if (width == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_RESULT_PTR(result);

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(pixel_format, &bits_per_pixel));

    const int add = bits_per_pixel % 8 == 0 ? 0 : 1;

    *result = (unsigned)(((double)width * bits_per_pixel / 8) + add);

    return SAIL_OK;
}

sail_status_t sail_bytes_per_image(const struct sail_image *image, unsigned *result) {

    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_RESULT_PTR(result);

    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

    *result = bytes_per_line * image->height;

    return SAIL_OK;
}

sail_status_t sail_print_errno(const char *format) {

    SAIL_CHECK_STRING_PTR(format);

    if (strstr(format, "%s") == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

#ifdef SAIL_WIN32
    char buffer[80];
    strerror_s(buffer, sizeof(buffer), errno);
    SAIL_LOG_ERROR(format, buffer);
#else
    SAIL_LOG_ERROR(format, strerror(errno));
#endif

    return SAIL_OK;
}

sail_status_t sail_malloc(size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = malloc(size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

sail_status_t sail_realloc(size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = realloc(*ptr, size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

sail_status_t sail_calloc(size_t nmemb, size_t size, void **ptr) {

    SAIL_CHECK_PTR(ptr);

    void *ptr_local = calloc(nmemb, size);

    if (ptr_local == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    *ptr = ptr_local;

    return SAIL_OK;
}

void sail_free(void *ptr) {

    free(ptr);
}

uint64_t sail_now(void) {

#ifdef SAIL_WIN32
    SAIL_THREAD_LOCAL static bool initialized = false;
    SAIL_THREAD_LOCAL static double frequency = 0;

    LARGE_INTEGER li;

    if (!initialized) {
        initialized = true;

        if (!QueryPerformanceFrequency(&li)) {
            SAIL_LOG_ERROR("Failed to get the current time. Error: %d", GetLastError());
            return SAIL_OK;
        }

        frequency = (double)li.QuadPart / 1000;
    }

    if (!QueryPerformanceCounter(&li)) {
        SAIL_LOG_ERROR("Failed to get the current time. Error: %d", GetLastError());
        return SAIL_OK;
    }

    return (uint64_t)((double)li.QuadPart / frequency);
#else
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        sail_print_errno("Failed to get the current time: %s");
        return SAIL_OK;
    }

    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

bool sail_path_exists(const char *path) {

    if (path == NULL) {
        SAIL_LOG_ERROR("Path is NULL");
        return false;
    }

#ifdef SAIL_WIN32
    return _access(path, 0) == 0;
#else
    return access(path, 0) == 0;
#endif
}

bool sail_is_dir(const char *path) {

    if (path == NULL) {
        SAIL_LOG_ERROR("Path is NULL");
        return false;
    }

#ifdef SAIL_WIN32
    struct _stat attrs;

    if (_stat(path, &attrs) != 0) {
        return false;
    }

    return (attrs.st_mode & _S_IFMT) == _S_IFDIR;
#else
    struct stat attrs;

    if (stat(path, &attrs) != 0) {
        return false;
    }

    return S_ISDIR(attrs.st_mode);
#endif
}

bool sail_is_file(const char *path) {

    if (path == NULL) {
        SAIL_LOG_ERROR("Path is NULL");
        return false;
    }

#ifdef SAIL_WIN32
    struct _stat attrs;

    if (_stat(path, &attrs) != 0) {
        return false;
    }

    return (attrs.st_mode & _S_IFMT) == _S_IFREG;
#else
    struct stat attrs;

    if (stat(path, &attrs) != 0) {
        return false;
    }

    return S_ISREG(attrs.st_mode);
#endif
}

sail_status_t sail_file_size(const char *path, size_t *size) {

    if (path == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    bool is_file;

#ifdef SAIL_WIN32
    struct _stat attrs;

    if (_stat(path, &attrs) != 0) {
        return false;
    }

    is_file = (attrs.st_mode & _S_IFMT) == _S_IFREG;
#else
    struct stat attrs;

    if (stat(path, &attrs) != 0) {
        return false;
    }

    is_file = S_ISREG(attrs.st_mode);
#endif

    if (!is_file) {
        SAIL_LOG_ERROR("'%s' is not a file", path);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    *size = attrs.st_size;

    return SAIL_OK;
}

sail_status_t sail_read_file_contents(const char *path, void *buffer) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_BUFFER_PTR(buffer);

    size_t size;
    SAIL_TRY(sail_file_size(path, &size));

#ifdef SAIL_WIN32
    FILE *f = _fsopen(path, "rb", _SH_DENYWR);
#else
    FILE *f = fopen(path, "rb");
#endif

    if (f == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    if (fread(buffer, 1, size, f) != size) {
        fclose(f);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    fclose(f);

    return SAIL_OK;
}

sail_status_t sail_alloc_buffer_from_file_contents(const char *path, void **buffer, size_t *buffer_length) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_PTR(buffer_length);

    size_t size;
    SAIL_TRY(sail_file_size(path, &size));

    void *buffer_local;
    SAIL_TRY(sail_malloc(size, &buffer_local));

    SAIL_TRY_OR_CLEANUP(sail_read_file_contents(path, buffer_local),
                        /* cleanup */ sail_free(buffer_local));

    *buffer = buffer_local;
    *buffer_length = size;

    return SAIL_OK;
}
