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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

void my_error_fn(const char *module, const char *format, va_list ap) {

    char buffer[80];

#ifdef SAIL_WIN32
    vsprintf_s(buffer, sizeof(buffer), format, ap);
#else
    vsprintf(buffer, format, ap);
#endif

	if (module != NULL) {
        SAIL_LOG_ERROR("%s: %s", module, buffer);
    } else {
        SAIL_LOG_ERROR("%s", buffer);
    }
}

void my_warning_fn(const char *module, const char *format, va_list ap) {

    char buffer[80];

#ifdef SAIL_WIN32
    vsprintf_s(buffer, sizeof(buffer), format, ap);
#else
    vsprintf(buffer, format, ap);
#endif

	if (module != NULL) {
        SAIL_LOG_WARNING("%s: %s", module, buffer);
    } else {
        SAIL_LOG_WARNING("%s", buffer);
    }
}

sail_status_t supported_read_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
            return SAIL_OK;
        }

        default: {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }
    }
}
