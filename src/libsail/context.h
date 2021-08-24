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

#ifndef SAIL_CONTEXT_H
#define SAIL_CONTEXT_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SAIL context.
 *
 * SAIL context enumerates and holds a list of available codec info objects and a list of loaded codecs.
 * It's a global static object being created on demand by all SAIL reading, writing, and probing functions.
 * If you want to allocate SAIL context explicitly, use sail_init() or sail_init_with_flags().
 * All SAIL reading, writing, and probing functions will re-use it then.
 *
 * SAIL context modification (creating, destroying, loading and unloading codecs) is guarded with a mutex
 * to avoid unpredictable errors in a multi-threaded environment.
 */

/*
 * Flags to control SAIL initialization behavior.
 */
enum SailInitFlags {

    /*
     * Preload all codecs in sail_init_with_flags(). Codecs are lazy-loaded by default.
     */
    SAIL_FLAG_PRELOAD_CODECS = 1 << 0,
};

/*
 * Initializes a new SAIL global static context with default flags. Does nothing
 * if a global context already exists. See also sail_init_with_flags().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_init(void);

/*
 * Initializes a new SAIL global static context with the specific flags. Does nothing
 * if a global context already exists. Builds a list of available SAIL codecs. See SailInitFlags.
 *
 * Use this method when you need specific features like preloading codecs. If you don't need
 * specific features, using this method is optional. All reading or writing functions allocate
 * a global static context implicitly when they need it and when it doesn't exist yet.
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
 * Additionally, SAIL_THIRD_PARTY_CODECS_PATH environment variable with a list of ';'-separated paths
 * is searched if SAIL_THIRD_PARTY_CODECS_PATH is enabled in CMake, (the default) so you can load
 * your own codecs from there.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_init_with_flags(int flags);

/*
 * Unloads all the loaded codecs from the global static context to release memory occupied by them.
 * Use this function if you want to release some memory but do not want to deinitialize SAIL
 * with sail_finish(). Subsequent attempts to read or write images will reload necessary SAIL codecs
 * from disk.
 *
 * Warning: Make sure no reading or writing operations are in progress before calling sail_unload_codecs().
 *          Failure to do so may lead to a crash.
 *
 * Typical usage: This is a standalone function that can be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_unload_codecs(void);

/*
 * Destroys the global static context that was implicitly or explicitly allocated by
 * reading or writing functions.
 *
 * Unloads all codecs. All pointers to codec info objects, read and write features, and codecs
 * get invalidated. Using them after calling sail_finish() will lead to a crash.
 *
 * It's possible to initialize a new global static context afterwards, implicitly or explicitly.
 *
 * Warning: Make sure no reading or writing operations are in progress before calling sail_finish().
 *          Failure to do so may lead to a crash.
 */
SAIL_EXPORT void sail_finish(void);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
