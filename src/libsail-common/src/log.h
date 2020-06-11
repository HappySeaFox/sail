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

#ifndef SAIL_LOG_H
#define SAIL_LOG_H

#ifdef SAIL_BUILD
    #include "export.h"
#else
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum SailLogLevel {

    SAIL_LOG_LEVEL_ERROR,
    SAIL_LOG_LEVEL_WARNING,
    SAIL_LOG_LEVEL_INFO,
    SAIL_LOG_LEVEL_MESSAGE,
    SAIL_LOG_LEVEL_DEBUG,
};

SAIL_EXPORT void sail_log(int level, const char *file, int line, const char *format, ...);

/*
 * Log an error message.
 */
#define SAIL_LOG_ERROR(...) sail_log(SAIL_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

/*
 * Log a warning message.
 */
#define SAIL_LOG_WARNING(...) sail_log(SAIL_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)

/*
 * Log an important information message.
 */
#define SAIL_LOG_INFO(...) sail_log(SAIL_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)

/*
 * Log a regular message.
 */
#define SAIL_LOG_MESSAGE(...) sail_log(SAIL_LOG_LEVEL_MESSAGE, __FILE__, __LINE__, __VA_ARGS__)

/*
 * Log a debug message.
 */
#define SAIL_LOG_DEBUG(...) sail_log(SAIL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
