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

/*
 * Private functions.
 */

static sail_status_t hex_string_into_data(const char *str, void *data, size_t *data_saved) {

    unsigned char *data_local = data;
    *data_saved = 0;
    unsigned byte;
    int bytes_consumed;

#ifdef SAIL_WIN32
    while (sscanf_s(str, "%02x%n", &byte, &bytes_consumed) == 1) {
#else
    while (sscanf(str, "%02x%n", &byte, &bytes_consumed) == 1) {
#endif
        str += bytes_consumed;
        data_local[(*data_saved)++] = (unsigned char)byte;
    }

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t sail_memdup(const void *input, size_t input_size, void **output) {

    if (input == NULL) {
        *output = NULL;
        return SAIL_OK;
    }

    if (input_size == 0) {
        SAIL_LOG_ERROR("Cannot duplicate 0 bytes");
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
        SAIL_LOG_ERROR("Cannot duplicate 0 characters");
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
        SAIL_LOG_ERROR("The second argument of %s() must be >= 1", __func__);
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
#ifdef _MSC_VER
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

#ifdef _MSC_VER
    size_t ret;

    if (mbstowcs_s(&ret, output_local, length+1, input, length) != 0) {
        sail_free(output_local);
        SAIL_LOG_ERROR("Multibyte conversion failed");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#else
    if (mbstowcs(output_local, input, length) == (size_t)-1) {
        sail_free(output_local);
        SAIL_LOG_ERROR("Multibyte conversion failed");
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

    if (*ustr == '\0') {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_EMPTY_STRING);
    }

    *hash = 5381;
    unsigned c;

    while ((c = *ustr++) != 0) {
        *hash = ((*hash << 5) + *hash) + c; /* hash * 33 + c */
    }

    return SAIL_OK;
}

const char* sail_pixel_format_to_string(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:               return "UNKNOWN";

        case SAIL_PIXEL_FORMAT_BPP1:                  return "BPP1";
        case SAIL_PIXEL_FORMAT_BPP2:                  return "BPP2";
        case SAIL_PIXEL_FORMAT_BPP4:                  return "BPP4";
        case SAIL_PIXEL_FORMAT_BPP8:                  return "BPP8";
        case SAIL_PIXEL_FORMAT_BPP16:                 return "BPP16";
        case SAIL_PIXEL_FORMAT_BPP24:                 return "BPP24";
        case SAIL_PIXEL_FORMAT_BPP32:                 return "BPP32";
        case SAIL_PIXEL_FORMAT_BPP48:                 return "BPP48";
        case SAIL_PIXEL_FORMAT_BPP64:                 return "BPP64";
        case SAIL_PIXEL_FORMAT_BPP72:                 return "BPP72";
        case SAIL_PIXEL_FORMAT_BPP96:                 return "BPP96";
        case SAIL_PIXEL_FORMAT_BPP128:                return "BPP128";

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:          return "BPP1-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:          return "BPP2-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:          return "BPP4-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:          return "BPP8-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED:         return "BPP16-INDEXED";

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:        return "BPP1-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:        return "BPP2-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:        return "BPP4-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:        return "BPP8-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       return "BPP16-GRAYSCALE";

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  return "BPP4-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  return "BPP8-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: return "BPP16-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: return "BPP32-GRAYSCALE-ALPHA";

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:          return "BPP16-RGB555";
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:          return "BPP16-BGR555";
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:          return "BPP16-RGB565";
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:          return "BPP16-BGR565";

        case SAIL_PIXEL_FORMAT_BPP24_RGB:             return "BPP24-RGB";
        case SAIL_PIXEL_FORMAT_BPP24_BGR:             return "BPP24-BGR";

        case SAIL_PIXEL_FORMAT_BPP48_RGB:             return "BPP48-RGB";
        case SAIL_PIXEL_FORMAT_BPP48_BGR:             return "BPP48-BGR";

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:            return "BPP32-RGBX";
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:            return "BPP32-BGRX";
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:            return "BPP32-XRGB";
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:            return "BPP32-XBGR";
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            return "BPP32-RGBA";
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:            return "BPP32-BGRA";
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:            return "BPP32-ARGB";
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:            return "BPP32-ABGR";

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:            return "BPP64-RGBX";
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:            return "BPP64-BGRX";
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:            return "BPP64-XRGB";
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:            return "BPP64-XBGR";
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            return "BPP64-RGBA";
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:            return "BPP64-BGRA";
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:            return "BPP64-ARGB";
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:            return "BPP64-ABGR";

        case SAIL_PIXEL_FORMAT_BPP32_CMYK:            return "BPP32-CMYK";
        case SAIL_PIXEL_FORMAT_BPP64_CMYK:            return "BPP64-CMYK";

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:           return "BPP24-YCBCR";

        case SAIL_PIXEL_FORMAT_BPP32_YCCK:            return "BPP32-YCCK";

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LAB:         return "BPP24-CIE-LAB";
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LAB:         return "BPP40-CIE-LAB";

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LUV:         return "BPP24-CIE-LUV";
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LUV:         return "BPP40-CIE-LUV";

        case SAIL_PIXEL_FORMAT_BPP24_YUV:             return "BPP24-YUV";
        case SAIL_PIXEL_FORMAT_BPP30_YUV:             return "BPP30-YUV";
        case SAIL_PIXEL_FORMAT_BPP36_YUV:             return "BPP36-YUV";
        case SAIL_PIXEL_FORMAT_BPP48_YUV:             return "BPP48-YUV";

        case SAIL_PIXEL_FORMAT_BPP32_YUVA:            return "BPP32-YUVA";
        case SAIL_PIXEL_FORMAT_BPP40_YUVA:            return "BPP40-YUVA";
        case SAIL_PIXEL_FORMAT_BPP48_YUVA:            return "BPP48-YUVA";
        case SAIL_PIXEL_FORMAT_BPP64_YUVA:            return "BPP64-YUVA";
    }

    return NULL;
}

enum SailPixelFormat sail_pixel_format_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_PIXEL_FORMAT_UNKNOWN);

    /*
     * The switch doesn't look very nice, I know :) However, it's fast and doesn't require
     * extra data structures and initializations. It's not C++11, so we choose between two evils:
     *
     *     1. Introduce extra data structures and their initializations to work with hashes.
     *     2. Use a single ugly looking switch/case.
     */
    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_PIXEL_FORMAT_UNKNOWN;

        case UINT64_C(6383902552):           return SAIL_PIXEL_FORMAT_BPP1;
        case UINT64_C(6383902553):           return SAIL_PIXEL_FORMAT_BPP2;
        case UINT64_C(6383902555):           return SAIL_PIXEL_FORMAT_BPP4;
        case UINT64_C(6383902559):           return SAIL_PIXEL_FORMAT_BPP8;
        case UINT64_C(210668784270):         return SAIL_PIXEL_FORMAT_BPP16;
        case UINT64_C(210668784301):         return SAIL_PIXEL_FORMAT_BPP24;
        case UINT64_C(210668784332):         return SAIL_PIXEL_FORMAT_BPP32;
        case UINT64_C(210668784371):         return SAIL_PIXEL_FORMAT_BPP48;
        case UINT64_C(210668784433):         return SAIL_PIXEL_FORMAT_BPP64;
        case UINT64_C(210668784464):         return SAIL_PIXEL_FORMAT_BPP72;
        case UINT64_C(210668784534):         return SAIL_PIXEL_FORMAT_BPP96;
        case UINT64_C(6952069880834):        return SAIL_PIXEL_FORMAT_BPP128;

        case UINT64_C(13257949335914442470): return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
        case UINT64_C(13257950742323060711): return SAIL_PIXEL_FORMAT_BPP2_INDEXED;
        case UINT64_C(13257953555140297193): return SAIL_PIXEL_FORMAT_BPP4_INDEXED;
        case UINT64_C(13257959180774770157): return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        case UINT64_C(13237225848150241308): return SAIL_PIXEL_FORMAT_BPP16_INDEXED;

        case UINT64_C(12552958524517323328): return SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;
        case UINT64_C(12554490103502587777): return SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;
        case UINT64_C(12557553261473116675): return SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;
        case UINT64_C(12563679577414174471): return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case UINT64_C(8431824423011809526):  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

        case UINT64_C(9367569596161118198):  return SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA;
        case UINT64_C(12512997289017890810): return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;
        case UINT64_C(3292614999547101481):  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
        case UINT64_C(5929884054553197927):  return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;

        case UINT64_C(13257949683479278997): return SAIL_PIXEL_FORMAT_BPP16_RGB555;
        case UINT64_C(13257949682853687701): return SAIL_PIXEL_FORMAT_BPP16_BGR555;
        case UINT64_C(13257949683479279030): return SAIL_PIXEL_FORMAT_BPP16_RGB565;
        case UINT64_C(13257949682853687734): return SAIL_PIXEL_FORMAT_BPP16_BGR565;

        case UINT64_C(249836535348735093):   return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case UINT64_C(249836535348717685):   return SAIL_PIXEL_FORMAT_BPP24_BGR;

        case UINT64_C(249836535431749563):   return SAIL_PIXEL_FORMAT_BPP48_RGB;
        case UINT64_C(249836535431732155):   return SAIL_PIXEL_FORMAT_BPP48_BGR;

        case UINT64_C(8244605667721455340):  return SAIL_PIXEL_FORMAT_BPP32_RGBX;
        case UINT64_C(8244605667720880876):  return SAIL_PIXEL_FORMAT_BPP32_BGRX;
        case UINT64_C(8244605667721683084):  return SAIL_PIXEL_FORMAT_BPP32_XRGB;
        case UINT64_C(8244605667721665676):  return SAIL_PIXEL_FORMAT_BPP32_XBGR;
        case UINT64_C(8244605667721455317):  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case UINT64_C(8244605667720880853):  return SAIL_PIXEL_FORMAT_BPP32_BGRA;
        case UINT64_C(8244605667720856533):  return SAIL_PIXEL_FORMAT_BPP32_ARGB;
        case UINT64_C(8244605667720839125):  return SAIL_PIXEL_FORMAT_BPP32_ABGR;

        case UINT64_C(8244605671674130033):  return SAIL_PIXEL_FORMAT_BPP64_RGBX;
        case UINT64_C(8244605671673555569):  return SAIL_PIXEL_FORMAT_BPP64_BGRX;
        case UINT64_C(8244605671674357777):  return SAIL_PIXEL_FORMAT_BPP64_XRGB;
        case UINT64_C(8244605671674340369):  return SAIL_PIXEL_FORMAT_BPP64_XBGR;
        case UINT64_C(8244605671674130010):  return SAIL_PIXEL_FORMAT_BPP64_RGBA;
        case UINT64_C(8244605671673555546):  return SAIL_PIXEL_FORMAT_BPP64_BGRA;
        case UINT64_C(8244605671673531226):  return SAIL_PIXEL_FORMAT_BPP64_ARGB;
        case UINT64_C(8244605671673513818):  return SAIL_PIXEL_FORMAT_BPP64_ABGR;

        case UINT64_C(8244605667720923565):  return SAIL_PIXEL_FORMAT_BPP32_CMYK;
        case UINT64_C(8244605671673598258):  return SAIL_PIXEL_FORMAT_BPP64_CMYK;

        case UINT64_C(13817569962846953645): return SAIL_PIXEL_FORMAT_BPP24_YCBCR;

        case UINT64_C(8244605667721702563):  return SAIL_PIXEL_FORMAT_BPP32_YCCK;

        case UINT64_C(13237269438873232231): return SAIL_PIXEL_FORMAT_BPP24_CIE_LAB;
        case UINT64_C(13237356636207563173): return SAIL_PIXEL_FORMAT_BPP40_CIE_LAB;

        case UINT64_C(13237269438873232911): return SAIL_PIXEL_FORMAT_BPP24_CIE_LUV;
        case UINT64_C(13237356636207563853): return SAIL_PIXEL_FORMAT_BPP40_CIE_LUV;

        case UINT64_C(249836535348743198):   return SAIL_PIXEL_FORMAT_BPP24_YUV;
        case UINT64_C(249836535383134907):   return SAIL_PIXEL_FORMAT_BPP30_YUV;
        case UINT64_C(249836535390250433):   return SAIL_PIXEL_FORMAT_BPP36_YUV;
        case UINT64_C(249836535431757668):   return SAIL_PIXEL_FORMAT_BPP48_YUV;

        case UINT64_C(8244605667721722782):  return SAIL_PIXEL_FORMAT_BPP32_YUVA;
        case UINT64_C(8244605668934919965):  return SAIL_PIXEL_FORMAT_BPP40_YUVA;
        case UINT64_C(8244605669248003109):  return SAIL_PIXEL_FORMAT_BPP48_YUVA;
        case UINT64_C(8244605671674397475):  return SAIL_PIXEL_FORMAT_BPP64_YUVA;
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

const char* sail_image_property_to_string(enum SailImageProperty image_property) {

    switch (image_property) {
        case SAIL_IMAGE_PROPERTY_UNKNOWN:            return "UNKNOWN";
        case SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY: return "FLIPPED-VERTICALLY";
        case SAIL_IMAGE_PROPERTY_INTERLACED:         return "INTERLACED";
    }

    return NULL;
}

enum SailImageProperty sail_image_property_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_IMAGE_PROPERTY_UNKNOWN);

    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_IMAGE_PROPERTY_UNKNOWN;
        case UINT64_C(17202465669660106453): return SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY;
        case UINT64_C(8244927930303708800):  return SAIL_IMAGE_PROPERTY_INTERLACED;
    }

    return SAIL_IMAGE_PROPERTY_UNKNOWN;
}

const char* sail_compression_to_string(enum SailCompression compression) {

    switch (compression) {
        case SAIL_COMPRESSION_UNSUPPORTED:   return "UNSUPPORTED";
        case SAIL_COMPRESSION_UNKNOWN:       return "UNKNOWN";
        case SAIL_COMPRESSION_NONE:          return "NONE";
        case SAIL_COMPRESSION_ADOBE_DEFLATE: return "ADOBE-DEFLATE";
        case SAIL_COMPRESSION_CCITT_FAX3:    return "CCITT-FAX3";
        case SAIL_COMPRESSION_CCITT_FAX4:    return "CCITT-FAX4";
        case SAIL_COMPRESSION_CCITT_RLE:     return "CCITT-RLE";
        case SAIL_COMPRESSION_CCITT_RLEW:    return "CCITT-RLEW";
        case SAIL_COMPRESSION_CCITT_T4:      return "CCITT-T4";
        case SAIL_COMPRESSION_CCITT_T6:      return "CCITT-T6";
        case SAIL_COMPRESSION_DCS:           return "DCS";
        case SAIL_COMPRESSION_DEFLATE:       return "DEFLATE";
        case SAIL_COMPRESSION_IT8_BL:        return "IT8-BL";
        case SAIL_COMPRESSION_IT8_CTPAD:     return "IT8-CTPAD";
        case SAIL_COMPRESSION_IT8_LW:        return "IT8-LW";
        case SAIL_COMPRESSION_IT8_MP:        return "IT8-MP";
        case SAIL_COMPRESSION_JBIG:          return "JBIG";
        case SAIL_COMPRESSION_JPEG:          return "JPEG";
        case SAIL_COMPRESSION_JPEG_2000:     return "JPEG-2000";
        case SAIL_COMPRESSION_JPEG_XL:       return "JPEG-XL";
        case SAIL_COMPRESSION_JPEG_XR:       return "JPEG-XR";
        case SAIL_COMPRESSION_LERC:          return "LERC";
        case SAIL_COMPRESSION_LZMA:          return "LZMA";
        case SAIL_COMPRESSION_LZW:           return "LZW";
        case SAIL_COMPRESSION_NEXT:          return "NEXT";
        case SAIL_COMPRESSION_OJPEG:         return "OJPEG";
        case SAIL_COMPRESSION_PACKBITS:      return "PACKBITS";
        case SAIL_COMPRESSION_PIXAR_FILM:    return "PIXAR-FILM";
        case SAIL_COMPRESSION_PIXAR_LOG:     return "PIXAR-LOG";
        case SAIL_COMPRESSION_RLE:           return "RLE";
        case SAIL_COMPRESSION_SGI_LOG:       return "SGI-LOG";
        case SAIL_COMPRESSION_SGI_LOG24:     return "SGI-LOG24";
        case SAIL_COMPRESSION_T43:           return "T43";
        case SAIL_COMPRESSION_T85:           return "T85";
        case SAIL_COMPRESSION_THUNDERSCAN:   return "THUNDERSCAN";
        case SAIL_COMPRESSION_WEBP:          return "WEBP";
        case SAIL_COMPRESSION_ZSTD:          return "ZSTD";
    }

    return NULL;
}

enum SailCompression sail_compression_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_COMPRESSION_UNKNOWN);

    switch (hash) {
        case UINT64_C(13846582888989074574): return SAIL_COMPRESSION_UNSUPPORTED;
        case UINT64_C(229442760833397):      return SAIL_COMPRESSION_UNKNOWN;
        case UINT64_C(6384332661):           return SAIL_COMPRESSION_NONE;
        case UINT64_C(10962109560604417378): return SAIL_COMPRESSION_ADOBE_DEFLATE;
        case UINT64_C(8244633541513328571):  return SAIL_COMPRESSION_CCITT_FAX3;
        case UINT64_C(8244633541513328572):  return SAIL_COMPRESSION_CCITT_FAX4;
        case UINT64_C(249837380045871852):   return SAIL_COMPRESSION_CCITT_RLE;
        case UINT64_C(8244633541513771203):  return SAIL_COMPRESSION_CCITT_RLEW;
        case UINT64_C(7570829698359793):     return SAIL_COMPRESSION_CCITT_T4;
        case UINT64_C(7570829698359795):     return SAIL_COMPRESSION_CCITT_T6;
        case UINT64_C(193453343):            return SAIL_COMPRESSION_DCS;
        case UINT64_C(229420447642554):      return SAIL_COMPRESSION_DEFLATE;
        case UINT64_C(6952347705973):        return SAIL_COMPRESSION_IT8_BL;
        case UINT64_C(249846519511114451):   return SAIL_COMPRESSION_IT8_CTPAD;
        case UINT64_C(6952347706314):        return SAIL_COMPRESSION_IT8_LW;
        case UINT64_C(6952347706340):        return SAIL_COMPRESSION_IT8_MP;
        case UINT64_C(6384174593):           return SAIL_COMPRESSION_JBIG;
        case UINT64_C(6384189707):           return SAIL_COMPRESSION_JPEG;
        case UINT64_C(249847773225217050):   return SAIL_COMPRESSION_JPEG_2000;
        case UINT64_C(229428625552444):      return SAIL_COMPRESSION_JPEG_XL;
        case UINT64_C(229428625552450):      return SAIL_COMPRESSION_JPEG_XR;
        case UINT64_C(6384250027):           return SAIL_COMPRESSION_LERC;
        case UINT64_C(6384272729):           return SAIL_COMPRESSION_LZMA;
        case UINT64_C(193462818):            return SAIL_COMPRESSION_LZW;
        case UINT64_C(6384322116):           return SAIL_COMPRESSION_NEXT;
        case UINT64_C(210683986298):         return SAIL_COMPRESSION_OJPEG;
        case UINT64_C(7571380909080566):     return SAIL_COMPRESSION_PACKBITS;
        case UINT64_C(8245245943922754206):  return SAIL_COMPRESSION_PIXAR_FILM;
        case UINT64_C(249855937694635640):   return SAIL_COMPRESSION_PIXAR_LOG;
        case UINT64_C(193468872):            return SAIL_COMPRESSION_RLE;
        case UINT64_C(229439900388407):      return SAIL_COMPRESSION_SGI_LOG;
        case UINT64_C(249860051522976925):   return SAIL_COMPRESSION_SGI_LOG24;
        case UINT64_C(193470240):            return SAIL_COMPRESSION_T43;
        case UINT64_C(193470374):            return SAIL_COMPRESSION_T85;
        case UINT64_C(13844775339661004164): return SAIL_COMPRESSION_THUNDERSCAN;
        case UINT64_C(6384644819):           return SAIL_COMPRESSION_WEBP;
        case UINT64_C(6384768458):           return SAIL_COMPRESSION_ZSTD;
    }

    return SAIL_COMPRESSION_UNKNOWN;
}

const char* sail_meta_data_to_string(enum SailMetaData meta_data) {

    switch (meta_data) {
        case SAIL_META_DATA_UNKNOWN:       return "Unknown";

        case SAIL_META_DATA_ARTIST:        return "Artist";
        case SAIL_META_DATA_AUTHOR:        return "Author";
        case SAIL_META_DATA_COMMENT:       return "Comment";
        case SAIL_META_DATA_COMPUTER:      return "Computer";
        case SAIL_META_DATA_COPYRIGHT:     return "Copyright";
        case SAIL_META_DATA_CREATION_TIME: return "Creation Time";
        case SAIL_META_DATA_DESCRIPTION:   return "Description";
        case SAIL_META_DATA_DISCLAIMER:    return "Disclaimer";
        case SAIL_META_DATA_DOCUMENT:      return "Document";
        case SAIL_META_DATA_EXIF:          return "EXIF";
        case SAIL_META_DATA_IPTC:          return "IPTC";
        case SAIL_META_DATA_LABEL:         return "Label";
        case SAIL_META_DATA_MAKE:          return "Make";
        case SAIL_META_DATA_MODEL:         return "Model";
        case SAIL_META_DATA_NAME:          return "Name";
        case SAIL_META_DATA_PRINTER:       return "Printer";
        case SAIL_META_DATA_SOFTWARE:      return "Software";
        case SAIL_META_DATA_SOURCE:        return "Source";
        case SAIL_META_DATA_TITLE:         return "Title";
        case SAIL_META_DATA_URL:           return "URL";
        case SAIL_META_DATA_WARNING:       return "Warning";
        case SAIL_META_DATA_XMP:           return "XMP";
    }

    return NULL;
}

enum SailMetaData sail_meta_data_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_META_DATA_UNKNOWN);

    switch (hash) {
        case UINT64_C(229444052301365):      return SAIL_META_DATA_UNKNOWN;

        case UINT64_C(6952072423676):        return SAIL_META_DATA_ARTIST;
        case UINT64_C(6952075980216):        return SAIL_META_DATA_AUTHOR;
        case UINT64_C(229420847338040):      return SAIL_META_DATA_COMMENT;
        case UINT64_C(7570887966294228):     return SAIL_META_DATA_COMPUTER;
        case UINT64_C(249839307110380862):   return SAIL_META_DATA_COPYRIGHT;
        case UINT64_C(16658027699238675945): return SAIL_META_DATA_CREATION_TIME;
        case UINT64_C(13821659157043486569): return SAIL_META_DATA_DESCRIPTION;
        case UINT64_C(8244735206874071778):  return SAIL_META_DATA_DISCLAIMER;
        case UINT64_C(7570930199009348):     return SAIL_META_DATA_DOCUMENT;
        case UINT64_C(6384018865):           return SAIL_META_DATA_EXIF;
        case UINT64_C(6384154261):           return SAIL_META_DATA_IPTC;
        case UINT64_C(210681275781):         return SAIL_META_DATA_LABEL;
        case UINT64_C(6384317315):           return SAIL_META_DATA_MAKE;
        case UINT64_C(210682966998):         return SAIL_META_DATA_MODEL;
        case UINT64_C(6384353318):           return SAIL_META_DATA_NAME;
        case UINT64_C(229437749136105):      return SAIL_META_DATA_PRINTER;
        case UINT64_C(7571569592229392):     return SAIL_META_DATA_SOFTWARE;
        case UINT64_C(6952773348182):        return SAIL_META_DATA_SOURCE;
        case UINT64_C(210691070471):         return SAIL_META_DATA_TITLE;
        case UINT64_C(193472344):            return SAIL_META_DATA_URL;
        case UINT64_C(229446134771803):      return SAIL_META_DATA_WARNING;
        case UINT64_C(193475450):            return SAIL_META_DATA_XMP;
    }

    return SAIL_META_DATA_UNKNOWN;
}

const char* sail_codec_feature_to_string(enum SailCodecFeature codec_feature) {

    switch (codec_feature) {
        case SAIL_CODEC_FEATURE_UNKNOWN:     return "UNKNOWN";
        case SAIL_CODEC_FEATURE_STATIC:      return "STATIC";
        case SAIL_CODEC_FEATURE_ANIMATED:    return "ANIMATED";
        case SAIL_CODEC_FEATURE_MULTI_PAGED: return "MULTI-PAGED";
        case SAIL_CODEC_FEATURE_META_DATA:   return "META-DATA";
        case SAIL_CODEC_FEATURE_INTERLACED:  return "INTERLACED";
        case SAIL_CODEC_FEATURE_ICCP:        return "ICCP";
    }

    return NULL;
}

enum SailCodecFeature sail_codec_feature_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_CODEC_FEATURE_UNKNOWN);

    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_CODEC_FEATURE_UNKNOWN;
        case UINT64_C(6952739426029):        return SAIL_CODEC_FEATURE_STATIC;
        case UINT64_C(7570758658679240):     return SAIL_CODEC_FEATURE_ANIMATED;
        case UINT64_C(13834645239609548286): return SAIL_CODEC_FEATURE_MULTI_PAGED;
        case UINT64_C(249851542786072787):   return SAIL_CODEC_FEATURE_META_DATA;
        case UINT64_C(8244927930303708800):  return SAIL_CODEC_FEATURE_INTERLACED;
        case UINT64_C(6384139556):           return SAIL_CODEC_FEATURE_ICCP;
    }

    return SAIL_CODEC_FEATURE_UNKNOWN;
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
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LAB: *result = 40; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LUV: *result = 24; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LUV: *result = 40; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP24_YUV: *result = 24; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP30_YUV: *result = 30; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP36_YUV: *result = 36; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48_YUV: *result = 48; return SAIL_OK;

        case SAIL_PIXEL_FORMAT_BPP32_YUVA: *result = 32; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP40_YUVA: *result = 40; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP48_YUVA: *result = 48; return SAIL_OK;
        case SAIL_PIXEL_FORMAT_BPP64_YUVA: *result = 64; return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

enum SailPixelFormatComparisonPrivate {
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL,
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

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL:
            *result = pixel_format_bits1 <= pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL:
            *result = pixel_format_bits1 == pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL:
            *result = pixel_format_bits1 >= pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER:
            *result = pixel_format_bits1 > pixel_format_bits2;
        break;
    }

    return SAIL_OK;
}

sail_status_t sail_less_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS, result));

    return SAIL_OK;
}

sail_status_t sail_less_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL, result));

    return SAIL_OK;
}

sail_status_t sail_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL, result));

    return SAIL_OK;
}

sail_status_t sail_greater_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL, result));

    return SAIL_OK;
}

sail_status_t sail_greater_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2, bool *result) {

    SAIL_TRY(sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER, result));

    return SAIL_OK;
}

sail_status_t sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format, unsigned *result) {

    if (width == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_RESULT_PTR(result);

    unsigned bits_per_pixel;
    SAIL_TRY(sail_bits_per_pixel(pixel_format, &bits_per_pixel));

    *result = (unsigned)(((double)width * bits_per_pixel + 7) / 8);

    return SAIL_OK;
}

bool sail_is_indexed(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool sail_is_grayscale(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool sail_is_rgb_family(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP16_RGB555:
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:

        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR:

        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR:

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
            return true;
        }
        default: {
            return false;
        }
    }
}

sail_status_t sail_print_errno(const char *format) {

    SAIL_CHECK_STRING_PTR(format);

    if (strstr(format, "%s") == NULL) {
        SAIL_LOG_ERROR("Format argument must contain %%s");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

#ifdef _MSC_VER
    char buffer[80];
    strerror_s(buffer, sizeof(buffer), errno);
    SAIL_LOG_ERROR(format, buffer);
#else
    SAIL_LOG_ERROR(format, strerror(errno));
#endif

    return SAIL_OK;
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

#ifdef _MSC_VER
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

#ifdef _MSC_VER
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

#ifdef _MSC_VER
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

    SAIL_CHECK_PATH_PTR(path);

    bool is_file;

#ifdef _MSC_VER
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

sail_status_t sail_file_contents_into_data(const char *path, void *data) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_BUFFER_PTR(data);

    size_t size;
    SAIL_TRY(sail_file_size(path, &size));

#ifdef _MSC_VER
    FILE *f = _fsopen(path, "rb", _SH_DENYWR);
#else
    FILE *f = fopen(path, "rb");
#endif

    if (f == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    if (fread(data, 1, size, f) != size) {
        fclose(f);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_READ_FILE);
    }

    fclose(f);

    return SAIL_OK;
}

sail_status_t sail_file_contents_to_data(const char *path, void **data, size_t *data_size) {

    SAIL_CHECK_BUFFER_PTR(data);
    SAIL_CHECK_PTR(data_size);

    size_t size;
    SAIL_TRY(sail_file_size(path, &size));

    void *data_local;
    SAIL_TRY(sail_malloc(size, &data_local));

    SAIL_TRY_OR_CLEANUP(sail_file_contents_into_data(path, data_local),
                        /* cleanup */ sail_free(data_local));

    *data = data_local;
    *data_size = size;

    return SAIL_OK;
}

sail_status_t sail_hex_string_into_data(const char *str, void *data) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_BUFFER_PTR(data);

    size_t data_saved;
    SAIL_TRY(hex_string_into_data(str, data, &data_saved));

    return SAIL_OK;
}

sail_status_t sail_hex_string_to_data(const char *str, void **data, size_t *data_size) {

    SAIL_CHECK_STRING_PTR(str);
    SAIL_CHECK_BUFFER_PTR(data);
    SAIL_CHECK_PTR(data_size);

    const size_t str_length = strlen(str);

    void *ptr;
    SAIL_TRY(sail_malloc(str_length / 2, &ptr));
    unsigned char *data_local = ptr;

    size_t data_saved;
    SAIL_TRY_OR_CLEANUP(hex_string_into_data(str, data_local, &data_saved),
                        /* cleanup */ sail_free(data_local));

    *data = data_local;
    *data_size = data_saved;

    return SAIL_OK;
}

sail_status_t sail_data_into_hex_string(const void *data, size_t data_size, char *str) {

    SAIL_CHECK_BUFFER_PTR(data);
    SAIL_CHECK_STRING_PTR(str);

    char *str_local_copy = str;

    const unsigned char *data_local = data;
    size_t data_local_index = 0;

    for (size_t i = 0; i < data_size; i++) {
#ifdef SAIL_WIN32
        sprintf_s(str_local_copy, SIZE_MAX, "%02X", data_local[data_local_index++]);
#else
        sprintf(str_local_copy, "%02X", data_local[data_local_index++]);
#endif
        str_local_copy += 2;
    }

    return SAIL_OK;
}

sail_status_t sail_data_to_hex_string(const void *data, size_t data_size, char **str) {

    SAIL_CHECK_BUFFER_PTR(data);
    SAIL_CHECK_STRING_PTR(str);

    void *ptr;
    SAIL_TRY(sail_malloc(data_size * 2 + 1, &ptr));
    char *str_local = ptr;

    SAIL_TRY_OR_CLEANUP(sail_data_into_hex_string(data, data_size, str_local),
                        /* cleanup */ sail_free(str_local));

    *str = str_local;

    return SAIL_OK;
}
