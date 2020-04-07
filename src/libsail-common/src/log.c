#include "config.h"

#ifdef SAIL_WIN32
    #include <io.h>
    #define SAIL_ISATTY _isatty
    #define SAIL_FILENO _fileno
#else
    #define _POSIX_SOURCE
    #include <unistd.h>
    #define SAIL_ISATTY isatty
    #define SAIL_FILENO fileno
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

void sail_log(int level, const char *format, ...) {

    FILE *fptr = stderr;
    bool is_atty = (SAIL_ISATTY(SAIL_FILENO(fptr)) == 1);

    const char *level_string = NULL;

    switch (level) {
        case SAIL_LOG_LEVEL_ERROR:   level_string = "E"; break;
        case SAIL_LOG_LEVEL_WARNING: level_string = "W"; break;
        case SAIL_LOG_LEVEL_INFO:    level_string = "I"; break;
        case SAIL_LOG_LEVEL_MESSAGE: level_string = "M"; break;
        case SAIL_LOG_LEVEL_DEBUG:   level_string = "D"; break;
    }

    if (is_atty) {
        switch (level) {
            case SAIL_LOG_LEVEL_ERROR:   fprintf(fptr, "%s", SAIL_COLOR_BOLD_RED);    break;
            case SAIL_LOG_LEVEL_WARNING: fprintf(fptr, "%s", SAIL_COLOR_BOLD_YELLOW); break;
            case SAIL_LOG_LEVEL_INFO:    fprintf(fptr, "%s", SAIL_COLOR_BOLD_CYAN);   break;
            case SAIL_LOG_LEVEL_MESSAGE:                                              break;
            case SAIL_LOG_LEVEL_DEBUG:   fprintf(fptr, "%s", SAIL_COLOR_BOLD_BLUE);   break;
        }
    }

    va_list(args);
    va_start(args, format);

    fprintf(fptr, "SAIL: [%s] ", level_string);
    vfprintf(fptr, format, args);

    if (is_atty) {
        fprintf(fptr, "%s", SAIL_COLOR_RESET);
    }
}
