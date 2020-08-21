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

#ifndef SAIL_CONTEXT_CPP_H
#define SAIL_CONTEXT_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

/*
 * SAIL contexts.
 *
 * All SAIL functions allocate a thread-local static context per thread when necessary. The context enumerates and holds
 * a list of available codec info objects.
 *
 * If you call SAIL functions from three different threads, three different contexts are allocated.
 * You can destroy them with calling sail_finish() in each thread.
 */

namespace sail
{

class codec_info;
class io;

class SAIL_EXPORT context
{
    friend class image_reader;
    friend class image_writer;

public:
    context() = delete;
    context(const context&) = delete;
    context& operator=(const context&) = delete;

    /*
     * Initializes a new SAIL thread-local static context with the specific flags. Does nothing if the thread-local static context
     * already exists. Builds a list of available SAIL codecs. See SailInitFlags.
     *
     * If you don't need specific features like preloading codecs, just don't use this method at all.
     * All reading or writing methods allocate a thread-local static context implicitly when they need it
     * and when it doesn't exist already.
     *
     * It's always recommended to destroy the implicitly or explicitly allocated SAIL thread-local static context
     * with finish() when you're done with calling SAIL methods in the current thread.
     *
     * Codecs paths search algorithm (first found path wins):
     *
     *   Windows:
     *     1. SAIL_CODECS_PATH environment variable
     *     2. <SAIL DEPLOYMENT FOLDER>\lib\sail\codecs
     *     3. Hardcoded SAIL_CODECS_PATH in config.h
     *
     *   Unix (including macOS):
     *     1. SAIL_CODECS_PATH environment variable
     *     2. Hardcoded SAIL_CODECS_PATH in config.h
     *
     * Returns 0 on success or sail_status_t on error.
     */
    static sail_status_t init(int flags);

    /*
     * Unloads all the loaded codecs from the cache to release memory occupied by them. Use it if you want
     * to release some memory but do not want to deinitialize SAIL with sail_finish(). Subsequent attempts
     * to read or write images will reload necessary SAIL codecs from disk.
     *
     * Returns 0 on success or sail_status_t on error.
     */
    static sail_status_t unload_codecs();

    /*
     * Finalizes working with the thread-local static context that was implicitly or explicitly allocated by
     * reading or writing functions.
     *
     * Unloads all codecs. All pointers to codec info objects, read and write features get invalidated. 
     * Using them after calling finish() will lead to a crash.
     *
     * It's possible to initialize a new SAIL thread-local static context afterwards, implicitly or explicitly.
     */
    static void finish();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
