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
    #include "context.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail/context.h>
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

class SAIL_EXPORT context
{
public:
    context() = delete;
    context(const context&) = delete;
    context& operator=(const context&) = delete;

    /*
     * Initializes a new SAIL thread-local static context with the specific flags. Does nothing
     * if a thread-local static context already exists. Builds a list of available SAIL codecs.
     * See SailInitFlags.
     *
     * Use this function when you need specific features like preloading codecs. If you don't need specific
     * features, using this function is optional. All reading or writing functions allocate a thread-local
     * static context implicitly when they need it and when it doesn't exist already.
     *
     * It's recommended to destroy the implicitly or explicitly allocated SAIL thread-local static context
     * by calling sail_finish() when you're done with using SAIL functions in the current thread.
     *
     * Codecs path search algorithm (first found path wins):
     *
     * 1. VCPKG port on any platform
     *   Codecs are combined into a dynamically linked library, so no need to search them.
     *
     * 2. Standalone build or bundle, both compiled with SAIL_COMBINE_CODECS=ON
     *   Same to VCPKG port.
     *
     * 3. Windows standalone build or bundle, both compiled with SAIL_COMBINE_CODECS=OFF (the default)
     *   1. SAIL_CODECS_PATH environment variable
     *   2. <SAIL DEPLOYMENT FOLDER>\lib\sail\codecs
     *   3. Hardcoded SAIL_CODECS_PATH in config.h
     *
     * 4. Unix including macOS (standalone build), compiled with SAIL_COMBINE_CODECS=OFF (the default)
     *   1. SAIL_CODECS_PATH environment variable
     *   2. Hardcoded SAIL_CODECS_PATH in config.h
     *
     *   <FOUND PATH>/lib is added to LD_LIBRARY_PATH.
     *
     * Additionally, SAIL_MY_CODECS_PATH environment variable is always searched
     * so you can load your own codecs from there.
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t init(int flags);

    /*
     * Unloads all the loaded codecs from the cache to release memory occupied by them. Use this method
     * if you want to release some memory but do not want to deinitialize SAIL with finish().
     * Subsequent attempts to read or write images will reload necessary SAIL codecs from disk.
     *
     * Returns SAIL_OK on success.
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
