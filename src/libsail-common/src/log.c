#include "config.h"

#ifdef SAIL_COLORED_OUTPUT
    #ifdef SAIL_WIN32
        #include <io.h>
        #include <windows.h>
        #include <versionhelpers.h>
        #define SAIL_ISATTY _isatty
        #define SAIL_FILENO _fileno
    #else
        #define _POSIX_SOURCE
        #include <unistd.h>
        #define SAIL_ISATTY isatty
        #define SAIL_FILENO fileno
    #endif
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

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
#define SAIL_COLOR_RESET        "\033[0m"

#define SAIL_LOG_FPTR       stderr
#define SAIL_LOG_STD_HANDLE STD_ERROR_HANDLE

static bool check_ansi_colors_supported(void) {

    bool result = false;

#ifdef SAIL_COLORED_OUTPUT
    bool is_atty = (SAIL_ISATTY(SAIL_FILENO(SAIL_LOG_FPTR)) != 0);

    if (is_atty) {
        /*
         * This requires the application to target Windows 8.1 or later. Otherwise it always returns false.
         *
         * See https://docs.microsoft.com/ru-ru/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
         */
        #ifdef SAIL_WIN32
            if (IsWindows10OrGreater()) {
                HANDLE stderrHandle = GetStdHandle(SAIL_LOG_STD_HANDLE);
                DWORD consoleMode;

                if (GetConsoleMode(stderrHandle, &consoleMode)) {
                    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

                    if (SetConsoleMode(stderrHandle, consoleMode)) {
                        result = true;
                    }
                }
            }
        #else
            result = true;
        #endif
    }
#endif

    return result;
}

void sail_log(int level, const char *format, ...) {

    const char *level_string = NULL;

    switch (level) {
        case SAIL_LOG_LEVEL_ERROR:   level_string = "E"; break;
        case SAIL_LOG_LEVEL_WARNING: level_string = "W"; break;
        case SAIL_LOG_LEVEL_INFO:    level_string = "I"; break;
        case SAIL_LOG_LEVEL_MESSAGE: level_string = "M"; break;
        case SAIL_LOG_LEVEL_DEBUG:   level_string = "D"; break;
    }

    bool ansi_colors_supported = check_ansi_colors_supported();

    if (ansi_colors_supported) {
        switch (level) {
            case SAIL_LOG_LEVEL_ERROR:   fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_RED);    break;
            case SAIL_LOG_LEVEL_WARNING: fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_YELLOW); break;
            case SAIL_LOG_LEVEL_INFO:    fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_CYAN);   break;
            case SAIL_LOG_LEVEL_MESSAGE:                                                       break;
            case SAIL_LOG_LEVEL_DEBUG:   fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_BOLD_BLUE);   break;
        }
    }

    va_list(args);
    va_start(args, format);

    fprintf(SAIL_LOG_FPTR, "SAIL: [%s] ", level_string);
    vfprintf(SAIL_LOG_FPTR, format, args);

    if (ansi_colors_supported) {
        fprintf(SAIL_LOG_FPTR, "%s", SAIL_COLOR_RESET);
    }

    fprintf(SAIL_LOG_FPTR, "\n");
}
