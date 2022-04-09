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

struct sail_codec_info;
struct sail_io;
struct sail_load_options;
struct sail_save_options;

/*
 * Starts loading the specified image file with the specified load options. Pass codec info if you would like
 * to start loading with a specific codec. If not, just pass NULL. If you do not need specific load options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The load options are deep copied.
 *
 * Typical usage: sail_start_loading_file_with_options() ->
 *                sail_load_next_frame()                 ->
 *                sail_stop_loading().
 *
 * Or:            sail_codec_info_from_extension()       ->
 *                sail_start_loading_file_with_options() ->
 *                sail_load_next_frame()                 ->
 *                sail_stop_loading().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_loading(). States must be used per image. DO NOT use the same state
 * to start loading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_loading_file_with_options(const char *path, const struct sail_codec_info *codec_info,
                                                              const struct sail_load_options *load_options, void **state);

/*
 * Starts loading the specified memory buffer with the specified load options. If you do not need specific load options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The load options are deep copied.
 *
 * Typical usage: sail_codec_info_from_extension()         ->
 *                sail_start_loading_memory_with_options() ->
 *                sail_load_next_frame()                   ->
 *                sail_stop_loading().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_loading(). States must be used per image. DO NOT use the same state
 * to start loading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_loading_memory_with_options(const void *buffer, size_t buffer_length,
                                                                 const struct sail_codec_info *codec_info,
                                                                 const struct sail_load_options *load_options, void **state);

/*
 * Starts saving the specified image file with the specified save options. Pass codec info if you would like
 * to start saving with a specific codec. If not, just pass NULL. If you do not need specific save options,
 * just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The save options are deep copied.
 *
 * Typical usage: sail_start_saving_file_with_options() ->
 *                sail_write_next_frame()               ->
 *                sail_stop_saving().
 *
 * Or:            sail_codec_info_from_extension()      ->
 *                sail_start_saving_file_with_options() ->
 *                sail_write_next_frame()               ->
 *                sail_stop_saving().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_saving(). States must be used per image. DO NOT use the same state
 * to start saving multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_saving_file_with_options(const char *path,
                                                              const struct sail_codec_info *codec_info,
                                                              const struct sail_save_options *save_options, void **state);

/*
 * Starts saving the specified memory buffer with the specified save options. If you do not need specific
 * save options, just pass NULL. Codec-specific defaults will be used in this case.
 *
 * The save options are deep copied.
 *
 * Typical usage: sail_codec_info_from_extension()        ->
 *                sail_start_saving_memory_with_options() ->
 *                sail_write_next_frame()                 ->
 *                sail_stop_saving().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_saving. States must be used per image. DO NOT use the same state
 * to start saving multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_saving_memory_with_options(void *buffer, size_t buffer_length,
                                                                 const struct sail_codec_info *codec_info,
                                                             const struct sail_save_options *save_options, void **state);


/*
 * Stops saving started by sail_start_saving_file() and brothers. Closes the underlying I/O target.
 * Assigns the number of bytes written to the 'written' argument. Does nothing if the state is NULL.
 *
 * It is essential to always stop saving to free memory and I/O resources. Failure to do so
 * will lead to memory leaks.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_saving_with_written(void *state, size_t *written);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
