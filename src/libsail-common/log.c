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

#ifdef SAIL_COLORED_OUTPUT
    #ifdef SAIL_WIN32
        #include <io.h>
        #include <windows.h>
        #include <versionhelpers.h>
        #define SAIL_ISATTY _isatty
        #define SAIL_FILENO _fileno
    #else
        #include <unistd.h>
        #define SAIL_ISATTY isatty
        #define SAIL_FILENO fileno
    #endif
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sail-common.h"

/* ANSI terminal color escapes. */
#define SAIL_COLOR_RED          "\033[0;31m"
#define SAIL_COLOR_BOLD_RED     "\033[1;31m"
#define SAIL_COLOR_GREEN        "\033[0;32m"
#define SAIL_COLOR_BOLD_GREEN   "\033[1;32m"
#define SAIL_COLOR_YELLOW       "\033[0;33m"
#define SAIL_COLOR_BOLD_YELLOW  "\033[1;33m"
#define SAIL_COLOR_BLUE         "\033[0;34m"
#define SAIL_COLOR_BOLD_BLUE    "\033[1;34m"
#define SAIL_COLOR_MAGENTA      "\033[0;35m"
#define SAIL_COLOR_BOLD_MAGENTA "\033[1;35m"
#define SAIL_COLOR_CYAN         "\033[0;36m"
#define SAIL_COLOR_BOLD_CYAN    "\033[1;36m"
#define SAIL_COLOR_WHITE        "\033[0;37m"
#define SAIL_COLOR_BOLD_WHITE   "\033[1;37m"
#define SAIL_COLOR_RESET        "\033[0m"

#define SAIL_LOG_FPTR       stderr
#define SAIL_LOG_STD_HANDLE STD_ERROR_HANDLE /* for Windows */

static enum SailLogLevel sail_max_log_level = SAIL_LOG_LEVEL_DEBUG;

static sail_logger sail_external_logger = NULL;

static bool check_ansi_colors_supported(void) {

    static SAIL_THREAD_LOCAL bool ansi_colors_supported_called = false;
    static SAIL_THREAD_LOCAL bool ansi_colors_supported = false;

    if (ansi_colors_supported_called) {
        return ansi_colors_supported;
    }

    ansi_colors_supported_called = true;

#ifdef SAIL_COLORED_OUTPUT
    bool is_atty = (SAIL_ISATTY(SAIL_FILENO(SAIL_LOG_FPTR)) != 0);

    if (is_atty) {
        /*
         * This requires the application to target Windows 8.1 or later. Otherwise it always returns false.
         *
         * See https://docs.microsoft.com/ru-ru/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
         */
        #ifdef SAIL_WIN32
            /* MinGW 8.1 in particular doesn't define this flag. */
            #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
            #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
            #endif

            if (IsWindows10OrGreater()) {
                HANDLE stderrHandle = GetStdHandle(SAIL_LOG_STD_HANDLE);
                DWORD consoleMode;

                if (GetConsoleMode(stderrHandle, &consoleMode)) {
                    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

                    if (SetConsoleMode(stderrHandle, consoleMode)) {
                        ansi_colors_supported = true;
                    }
                }
            }
        #else
            ansi_colors_supported = true;
        #endif
    }
#endif

    return ansi_colors_supported;
}

void sail_log(enum SailLogLevel level, const char *file, int line, const char *format, ...) {

    /* Filter out. */
    if (level > sail_max_log_level) {
        return;
    }

    if (sail_external_logger != NULL) {
        va_list args;
        va_start(args, format);

        sail_external_logger(level, file, line, format, args);

        va_end(args);
        return;
    }

    const char *level_string = NULL;

    switch (level) {
        /* Something weird. */
        case SAIL_LOG_LEVEL_SILENCE: break;

        /* Normal log levels. */
        case SAIL_LOG_LEVEL_ERROR:   level_string = "E"; break;
        case SAIL_LOG_LEVEL_WARNING: level_string = "W"; break;
        case SAIL_LOG_LEVEL_INFO:    level_string = "I"; break;
        case SAIL_LOG_LEVEL_MESSAGE: level_string = "M"; break;
        case SAIL_LOG_LEVEL_DEBUG:   level_string = "D"; break;
        case SAIL_LOG_LEVEL_TRACE:   level_string = "T"; break;
    }

    const bool ansi_colors_supported = check_ansi_colors_supported();

    if (ansi_colors_supported) {
        switch (level) {
            case SAIL_LOG_LEVEL_SILENCE: break;

            case SAIL_LOG_LEVEL_ERROR:   fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_RED);    break;
            case SAIL_LOG_LEVEL_WARNING: fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_YELLOW); break;
            case SAIL_LOG_LEVEL_INFO:    fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_CYAN);   break;
            case SAIL_LOG_LEVEL_MESSAGE:                                                       break;
            case SAIL_LOG_LEVEL_DEBUG:   fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_BLUE);   break;
            case SAIL_LOG_LEVEL_TRACE:   fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_WHITE);  break;
        }
    }

    /* Print log level. */
    va_list args;
    va_start(args, format);

    fprintf(SAIL_LOG_FPTR, "SAIL: [%s] ", level_string);

    /* Print file and line. */
#ifdef SAIL_WIN32
    const char *name = strrchr(file, '\\');
#else
    const char *name = strrchr(file, '/');
#endif

    fprintf(SAIL_LOG_FPTR, "[%s:%d] ", name == NULL ? file : name+1, line);

    /* Print the rest of arguments. */
    vfprintf(SAIL_LOG_FPTR, format, args);

    if (ansi_colors_supported) {
        fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_RESET);
    }

    fprintf(SAIL_LOG_FPTR, "\n");

    va_end(args);
}

void sail_set_log_barrier(enum SailLogLevel max_level) {

    sail_max_log_level = max_level;
}

void sail_set_logger(sail_logger logger) {

    sail_external_logger = logger;
}
