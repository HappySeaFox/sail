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

#ifndef SAIL_SAIL_DEEP_DIVER_H
#define SAIL_SAIL_DEEP_DIVER_H

#include <stddef.h>

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

struct sail_io;
struct sail_codec_info;
struct sail_read_options;
struct sail_write_options;

/*
 * Starts reading the specified image file with the specified read options. Pass codec info if you would like
 * to start reading with a specific codec. If not, just pass NULL. If you do not need specific read options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The read options are deep copied.
 *
 * Typical usage: sail_start_reading_file_with_options() ->
 *                sail_read_next_frame()                 ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_extension()       ->
 *                sail_start_reading_file_with_options() ->
 *                sail_read_next_frame()                 ->
 *                sail_stop_reading().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading(). States must be used per image. DO NOT use the same state
 * to start reading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_file_with_options(const char *path, const struct sail_codec_info *codec_info,
                                                              const struct sail_read_options *read_options, void **state);

/*
 * Starts reading the specified memory buffer with the specified read options. If you do not need specific read options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The read options are deep copied.
 *
 * Typical usage: sail_codec_info_from_extension()      ->
 *                sail_start_reading_mem_with_options() ->
 *                sail_read_next_frame()                ->
 *                sail_stop_reading().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading(). States must be used per image. DO NOT use the same state
 * to start reading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_mem_with_options(const void *buffer, size_t buffer_length,
                                                             const struct sail_codec_info *codec_info,
                                                             const struct sail_read_options *read_options, void **state);

/*
 * Starts writing the specified image file with the specified write options. Pass codec info if you would like
 * to start writing with a specific codec. If not, just pass NULL. If you do not need specific write options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The write options are deep copied.
 *
 * Typical usage: sail_start_writing_file_with_options() ->
 *                sail_write_next_frame()                ->
 *                sail_stop_writing().
 *
 * Or:            sail_codec_info_from_extension()       ->
 *                sail_start_writing_file_with_options() ->
 *                sail_write_next_frame()                ->
 *                sail_stop_writing().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing(). States must be used per image. DO NOT use the same state
 * to start writing multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_file_with_options(const char *path,
                                                              const struct sail_codec_info *codec_info,
                                                              const struct sail_write_options *write_options, void **state);

/*
 * Starts writing the specified memory buffer with the specified write options. If you do not need specific
 * write options, just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The write options are deep copied.
 *
 * Typical usage: sail_codec_info_from_extension()      ->
 *                sail_start_writing_mem_with_options() ->
 *                sail_write_next_frame()               ->
 *                sail_stop_writing().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to start writing multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_mem_with_options(void *buffer, size_t buffer_length,
                                                             const struct sail_codec_info *codec_info,
                                                             const struct sail_write_options *write_options, void **state);


/*
 * Stops writing started by sail_start_writing_file() and brothers. Closes the underlying I/O target.
 * Assigns the number of bytes written to the 'written' argument. Does nothing if the state is NULL.
 *
 * It is essential to always stop writing to free memory and I/O resources. Failure to do so
 * will lead to memory leaks.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_writing_with_written(void *state, size_t *written);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
