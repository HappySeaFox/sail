#ifndef SAIL_LOG_H
#define SAIL_LOG_H

#include <limits.h>

#ifdef SAIL_BUILD
    #include "export.h"
#else
    #include <sail/export.h>
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

    /* Not to be used. Resize the enum for future elements. */
    SAIL_LOG_LEVEL_RESIZE_ENUM_TO_INT = INT_MAX
};

SAIL_EXPORT void sail_log(int level, const char *format, ...);

/*
 * Log an error message.
 */
#define SAIL_LOG_ERROR(...) sail_log(SAIL_LOG_LEVEL_ERROR, __VA_ARGS__)

/*
 * Log a warning message.
 */
#define SAIL_LOG_WARNING(...) sail_log(SAIL_LOG_LEVEL_WARNING, __VA_ARGS__)

/*
 * Log an important information message.
 */
#define SAIL_LOG_INFO(...) sail_log(SAIL_LOG_LEVEL_INFO, __VA_ARGS__)

/*
 * Log a regular message.
 */
#define SAIL_LOG_MESSAGE(...) sail_log(SAIL_LOG_LEVEL_MESSAGE, __VA_ARGS__)

/*
 * Log a debug message.
 */
#define SAIL_LOG_DEBUG(...) sail_log(SAIL_LOG_LEVEL_DEBUG, __VA_ARGS__)

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
