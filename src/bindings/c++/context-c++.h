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
 * All SAIL functions allocate a global static context when necessary. The context enumerates
 * and holds a list of available codec info objects. The context is guarded with a mutex to avoid
 * unpredictable errors in a multi-threaded environment.
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
     * Initializes a new SAIL context with the specific flags. Does nothing if a global context
     * already exists. Builds a list of available SAIL codecs. See SailInitFlags.
     *
     * Use this method when you need specific features like preloading codecs. If you don't need specific
     * features, using this function is optional. All reading or writing functions allocate a global
     * static context implicitly when they need it and when it doesn't exist yet.
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
     * Additionally, SAIL_THIRD_PARTY_CODECS_PATH environment variable is searched if SAIL_THIRD_PARTY_CODECS is ON,
     * (the default) so you can load your own codecs from there.
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t init(int flags);

    /*
     * Unloads all the loaded codecs from the global static context to release memory occupied by them.
     * Use this function if you want to release some memory but do not want to deinitialize SAIL
     * with finish(). Subsequent attempts to read or write images will reload necessary SAIL codecs
     * from disk.
     *
     * Make sure no reading or writing operations are in progress before calling unload_codecs().
     *
     * Typical usage: This is a standalone function that can be called at any time.
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t unload_codecs();

    /*
     * Destroys the global static context that was implicitly or explicitly allocated by
     * reading or writing functions.
     *
     * Unloads all codecs. All pointers to codec info objects, read and write features, and codecs
     * get invalidated. Using them after calling finish() will lead to a crash.
     *
     * Make sure no reading or writing operations are in progress before calling finish().
     *
     * It's possible to initialize a new global static context afterwards, implicitly or explicitly.
     */
    static void finish();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
