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

#ifndef SAIL_LOG_CPP_H
#define SAIL_LOG_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
    #include "log.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail-common/log.h>
#endif

namespace sail
{

/*
 * Logging functions.
 */
namespace log
{

/*
 * Sets a maximum log level barrier. Only the messages of the specified log level or lower will be displayed.
 *
 * This function is not thread-safe. It's recommended to call it in the main thread
 * before initializing SAIL.
 */
SAIL_EXPORT void set_barrier(SailLogLevel max_level);

}

}

#endif
