/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

static sail_status_t hex_string_into_data(const char *str, size_t str_length, void *data, size_t *data_saved) {

    unsigned char *data_local = data;
    *data_saved = 0;
    unsigned byte;
    int bytes_consumed;

#ifdef _MSC_VER
    while (str_length > 1U && sscanf_s(str, "%02x%n", &byte, &bytes_consumed) == 1) {
#else
    while (str_length > 1U && sscanf(str, "%02x%n", &byte, &bytes_consumed) == 1) {
#endif
        str += bytes_consumed;
        str_length -= bytes_consumed;
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

    SAIL_CHECK_PTR(input);
    SAIL_CHECK_PTR(output);

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

uint64_t sail_string_hash(const char *str) {

    if (str == NULL || *str == '\0') {
        return 0;
    }

    const unsigned char *ustr = (const unsigned char *)str;

    uint64_t hash = 5381; /* Magic number, never explained. */
    unsigned c;

    while ((c = *ustr++) != 0) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

unsigned sail_bits_per_pixel(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN: return 0;

        case SAIL_PIXEL_FORMAT_BPP1:   return 1;
        case SAIL_PIXEL_FORMAT_BPP2:   return 2;
        case SAIL_PIXEL_FORMAT_BPP4:   return 4;
        case SAIL_PIXEL_FORMAT_BPP8:   return 8;
        case SAIL_PIXEL_FORMAT_BPP16:  return 16;
        case SAIL_PIXEL_FORMAT_BPP24:  return 24;
        case SAIL_PIXEL_FORMAT_BPP32:  return 32;
        case SAIL_PIXEL_FORMAT_BPP48:  return 48;
        case SAIL_PIXEL_FORMAT_BPP64:  return 64;
        case SAIL_PIXEL_FORMAT_BPP72:  return 72;
        case SAIL_PIXEL_FORMAT_BPP96:  return 96;
        case SAIL_PIXEL_FORMAT_BPP128: return 128;

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:  return 1;
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:  return 2;
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:  return 4;
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:  return 8;
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED: return 16;

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:  return 1;
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:  return 2;
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:  return 4;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:  return 8;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: return 16;

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  return 4;
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  return 8;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: return 16;
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: return 32;

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:
        case SAIL_PIXEL_FORMAT_BPP16_BGR565: return 16;

        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR: return 24;

        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_BGR: return 48;

        case SAIL_PIXEL_FORMAT_BPP16_RGBX:
        case SAIL_PIXEL_FORMAT_BPP16_BGRX:
        case SAIL_PIXEL_FORMAT_BPP16_XRGB:
        case SAIL_PIXEL_FORMAT_BPP16_XBGR:
        case SAIL_PIXEL_FORMAT_BPP16_RGBA:
        case SAIL_PIXEL_FORMAT_BPP16_BGRA:
        case SAIL_PIXEL_FORMAT_BPP16_ARGB:
        case SAIL_PIXEL_FORMAT_BPP16_ABGR: return 16;

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: return 32;

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: return 64;

        case SAIL_PIXEL_FORMAT_BPP16_FLOAT: return 16;
        case SAIL_PIXEL_FORMAT_BPP32_FLOAT: return 32;

        case SAIL_PIXEL_FORMAT_BPP32_CMYK: return 32;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK: return 64;

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: return 24;

        case SAIL_PIXEL_FORMAT_BPP32_YCCK: return 32;

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LAB: return 24;
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LAB: return 40;

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LUV: return 24;
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LUV: return 40;

        case SAIL_PIXEL_FORMAT_BPP24_YUV: return 24;
        case SAIL_PIXEL_FORMAT_BPP30_YUV: return 30;
        case SAIL_PIXEL_FORMAT_BPP36_YUV: return 36;
        case SAIL_PIXEL_FORMAT_BPP48_YUV: return 48;

        case SAIL_PIXEL_FORMAT_BPP32_YUVA: return 32;
        case SAIL_PIXEL_FORMAT_BPP40_YUVA: return 40;
        case SAIL_PIXEL_FORMAT_BPP48_YUVA: return 48;
        case SAIL_PIXEL_FORMAT_BPP64_YUVA: return 64;
    }

    return 0;
}

enum SailPixelFormatComparisonPrivate {
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL,
    SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER,
};

static sail_status_t sail_compare_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2,
                                                    enum SailPixelFormatComparisonPrivate op) {

    if (pixel_format1 == SAIL_PIXEL_FORMAT_UNKNOWN || pixel_format2 == SAIL_PIXEL_FORMAT_UNKNOWN) {
        return false;
    }

    const unsigned pixel_format_bits1 = sail_bits_per_pixel(pixel_format1);
    const unsigned pixel_format_bits2 = sail_bits_per_pixel(pixel_format2);

    switch(op) {
        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS:
            return pixel_format_bits1 < pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL:
            return pixel_format_bits1 <= pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL:
            return pixel_format_bits1 == pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL:
            return pixel_format_bits1 >= pixel_format_bits2;
        break;

        case SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER:
            return pixel_format_bits1 > pixel_format_bits2;
        break;
    }

    return false;
}

bool sail_less_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2) {

    return sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS);
}

bool sail_less_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2) {

    return sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_LESS_EQUAL);
}

bool sail_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2) {

    return sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_EQUAL);
}

bool sail_greater_equal_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2) {

    return sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER_EQUAL);
}

bool sail_greater_bits_per_pixel(enum SailPixelFormat pixel_format1, enum SailPixelFormat pixel_format2) {

    return sail_compare_bits_per_pixel(pixel_format1, pixel_format2, SAIL_PIXEL_FORMAT_COMPARISON_PRIVATE_GREATER);
}

unsigned sail_bytes_per_line(unsigned width, enum SailPixelFormat pixel_format) {

    const unsigned bits_per_pixel = sail_bits_per_pixel(pixel_format);
    return (unsigned)(((double)width * bits_per_pixel + 7) / 8);
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

void sail_print_errno(const char *format) {

    if (strstr(format, "%s") == NULL) {
        SAIL_LOG_ERROR("Format argument must contain %%s");
        return;
    }

#ifdef _MSC_VER
    char buffer[80];
    strerror_s(buffer, sizeof(buffer), errno);
    SAIL_LOG_ERROR(format, buffer);
#else
    SAIL_LOG_ERROR(format, strerror(errno));
#endif
}

uint64_t sail_now(void) {

#ifdef SAIL_WIN32
    static SAIL_THREAD_LOCAL bool initialized = false;
    static SAIL_THREAD_LOCAL double frequency = 0;

    LARGE_INTEGER li;

    if (!initialized) {
        initialized = true;

        if (!QueryPerformanceFrequency(&li)) {
            SAIL_LOG_ERROR("Failed to get the current time. Error: 0x%X", GetLastError());
            return 0;
        }

        frequency = (double)li.QuadPart / 1000;
    }

    if (!QueryPerformanceCounter(&li)) {
        SAIL_LOG_ERROR("Failed to get the current time. Error: 0x%X", GetLastError());
        return 0;
    }

    return (uint64_t)((double)li.QuadPart / frequency);
#else
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        sail_print_errno("Failed to get the current time: %s");
        return 0;
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

    SAIL_CHECK_PTR(path);

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

    SAIL_CHECK_PTR(path);
    SAIL_CHECK_PTR(data);

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

sail_status_t sail_alloc_data_from_file_contents(const char *path, void **data, size_t *data_size) {

    SAIL_CHECK_PTR(data);
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

    SAIL_CHECK_PTR(str);
    SAIL_CHECK_PTR(data);

    size_t data_saved;
    SAIL_TRY(hex_string_into_data(str, strlen(str), data, &data_saved));

    return SAIL_OK;
}

sail_status_t sail_hex_string_to_data(const char *str, void **data, size_t *data_size) {

    SAIL_CHECK_PTR(str);
    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(data_size);

    const size_t str_length = strlen(str);

    void *ptr;
    SAIL_TRY(sail_malloc(str_length / 2, &ptr));
    unsigned char *data_local = ptr;

    size_t data_saved;
    SAIL_TRY_OR_CLEANUP(hex_string_into_data(str, str_length, data_local, &data_saved),
                        /* cleanup */ sail_free(data_local));

    *data = data_local;
    *data_size = data_saved;

    return SAIL_OK;
}

sail_status_t sail_data_into_hex_string(const void *data, size_t data_size, char *str) {

    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(str);

    char *str_local_copy = str;

    const unsigned char *data_local = data;
    size_t data_local_index = 0;

    for (size_t i = 0; i < data_size; i++) {
#ifdef _MSC_VER
        sprintf_s(str_local_copy, SIZE_MAX, "%02X", data_local[data_local_index++]);
#else
        sprintf(str_local_copy, "%02X", data_local[data_local_index++]);
#endif
        str_local_copy += 2;
    }

    return SAIL_OK;
}

sail_status_t sail_data_to_hex_string(const void *data, size_t data_size, char **str) {

    SAIL_CHECK_PTR(data);
    SAIL_CHECK_PTR(str);

    void *ptr;
    SAIL_TRY(sail_malloc(data_size * 2 + 1, &ptr));
    char *str_local = ptr;

    SAIL_TRY_OR_CLEANUP(sail_data_into_hex_string(data, data_size, str_local),
                        /* cleanup */ sail_free(str_local));

    *str = str_local;

    return SAIL_OK;
}

uint16_t sail_reverse_uint16(uint16_t v)
{
#if defined(SAIL_HAVE_BUILTIN_BSWAP16)
    return __builtin_bswap16(v);
#elif defined(_MSC_VER)
    return _byteswap_ushort(v);
#else
    const uint8_t *view = (uint8_t *)&v;

    return (view[0] << 8) | view[1];
#endif
}

uint32_t sail_reverse_uint32(uint32_t v)
{
#if defined(SAIL_HAVE_BUILTIN_BSWAP32)
    return __builtin_bswap32(v);
#elif defined(_MSC_VER)
    return (uint32_t)_byteswap_ulong(v);
#else
    const uint8_t *view = (uint8_t *)&v;

    return (view[0] << 24) |
           (view[1] << 16) |
           (view[2] << 8)  |
           (view[3] << 0);
#endif
}

uint64_t sail_reverse_uint64(uint64_t v)
{
#if defined(SAIL_HAVE_BUILTIN_BSWAP64)
    return __builtin_bswap64(v);
#elif defined(_MSC_VER)
    return (uint64_t)_byteswap_uint64(v);
#else
    const uint8_t *view = (uint8_t *)&v;

    return ((uint64_t)view[0] << 56) |
           ((uint64_t)view[1] << 48) |
           ((uint64_t)view[2] << 40) |
           ((uint64_t)view[3] << 32) |
                     (view[4] << 24) |
                     (view[5] << 16) |
                     (view[6] << 8)  |
                     (view[7] << 0);
#endif
}
