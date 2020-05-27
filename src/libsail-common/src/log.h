/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
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
