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

#ifndef SAIL_SAIL_H
#define SAIL_SAIL_H

/* Universal libsail include. */

#ifdef SAIL_BUILD
    #include "sail-common.h"

    #include "context.h"
    #include "ini.h"
    #include "io_file.h"
    #include "io_mem.h"
    #include "io_noop.h"
    #include "codec.h"
    #include "codec_info.h"
    #include "codec_info_node.h"
    #include "codec_info_private.h"
    #include "sail_advanced.h"
    #include "sail_deep_diver.h"
    #include "sail_junior.h"
    #include "sail_private.h"
    #include "sail_technical_diver.h"
    #include "sail_technical_diver_private.h"
    #include "string_node.h"
#else
    #include <sail-common/sail-common.h>

    #include <sail/codec_info.h>
    #include <sail/codec_info_node.h>
    #include <sail/sail_advanced.h>
    #include <sail/sail_deep_diver.h>
    #include <sail/sail_junior.h>
    #include <sail/sail_technical_diver.h>
    #include <sail/string_node.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_codec_info_node;
struct sail_codec_info;

/*
 * SAIL contexts.
 *
 * All SAIL functions allocate a thread-local static context per thread when necessary. The context enumerates and holds
 * a list of available codec info objects.
 *
 * If you call SAIL functions from three different threads, three different contexts are allocated.
 * You can destroy them with calling sail_finish() in each thread.
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
 * Initializes a new SAIL thread-local static context with the specific flags. Does nothing if the thread-local static context
 * already exists. Builds a list of available SAIL codecs. See SailInitFlags.
 *
 * If you don't need specific features like preloading codecs, just don't use this function at all.
 * All reading or writing functions allocate a thread-local static context implicitly when they need it
 * and when it doesn't exist already.
 *
 * It's always recommended to destroy the implicitly or explicitly allocated SAIL thread-local static context
 * with sail_finish() when you're done with calling SAIL functions in the current thread.
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
SAIL_EXPORT sail_status_t sail_init_with_flags(int flags);

/*
 * Finalizes working with the thread-local static context that was implicitly or explicitly allocated by
 * reading or writing functions.
 *
 * Unloads all codecs. All pointers to codec info objects, read and write features get invalidated. 
 * Using them after calling sail_finish() will lead to a crash.
 *
 * It's possible to initialize a new SAIL thread-local static context afterwards, implicitly or explicitly.
 */
SAIL_EXPORT void sail_finish(void);

/*
 * Returns a linked list of found codec info nodes. Use it to determine the list of possible image formats,
 * file extensions, and mime types that could be hypothetically read or written by SAIL.
 *
 * Returns a pointer to the first codec info node or NULL when no SAIL codecs were found.
 * Use sail_codec_info_node.next to iterate.
 */
SAIL_EXPORT const struct sail_codec_info_node* sail_codec_info_list(void);

/*
 * Finds a first codec info object that supports reading or writing the specified file path by its file extension.
 * For example: "/test.jpg". The path might not exist.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_path() ->
 *                sail_start_reading_file()   ->
 *                sail_read_next_frame()      ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_path() ->
 *                sail_start_writing()        ->
 *                sail_read_next_frame()      ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_path(const char *path, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified file.
 * The comparison algorithm is case insensitive.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_path() ->
 *                sail_start_reading_file()                   ->
 *                sail_read_next_frame()                      ->
 *                sail_stop_reading().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_path(const char *path, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified memory buffer.
 * The comparison algorithm is case insensitive.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_mem() ->
 *                sail_start_reading_file()                  ->
 *                sail_read_next_frame()                     ->
 *                sail_stop_reading().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_mem(const void *buffer, size_t buffer_length,
                                                                   const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified I/O data source.
 * The comparison algorithm is case insensitive. After reading a magic number, this function rewinds the I/O
 * cursor position back to the beginning. That's why the I/O source must be seekable.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_io() ->
 *                sail_start_reading_file()                 ->
 *                sail_read_next_frame()                    ->
 *                sail_stop_reading().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_io(struct sail_io *io, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the specified file extension.
 * The comparison algorithm is case insensitive. For example: "jpg".
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_reading_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_writing()             ->
 *                sail_read_next_frame()           ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_extension(const char *extension, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the specified mime type.
 * The comparison algorithm is case insensitive. For example: "image/jpeg".
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_mime_type() ->
 *                sail_start_reading_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_mime_type() ->
 *                sail_start_writing()             ->
 *                sail_read_next_frame()           ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_mime_type(const char *mime_type, const struct sail_codec_info **codec_info);

/*
 * Unloads all the loaded codecs from the cache to release memory occupied by them. Use it if you want
 * to release some memory but do not want to deinitialize SAIL with sail_finish(). Subsequent attempts
 * to read or write images will reload necessary SAIL codecs from disk.
 *
 * Typical usage: This is a standalone function that can be called at any time.
 *
 * Returns 0 on success or sail_status_t on error.
 */
SAIL_EXPORT sail_status_t sail_unload_codecs(void);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
