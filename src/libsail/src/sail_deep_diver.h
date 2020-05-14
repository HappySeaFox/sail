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

struct sail_context;
struct sail_io;
struct sail_plugin_info;
struct sail_read_options;
struct sail_write_options;

/*
 * Starts reading the specified image file with the specified read options. Pass a particular plugin info if you'd like
 * to start reading with a specific codec. If not, just pass NULL. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Read options are deep copied.
 *
 * If read options is NULL, the subsequent calls to sail_read_next_frame() output pixels in BPP24-RGB pixel format
 * for image formats without transparency support and BPP32-RGBA otherwise.
 *
 * Typical usage: sail_start_reading_file_with_options() ->
 *                sail_read_next_frame()                 ->
 *                sail_stop_reading().
 *
 * Or:            sail_plugin_info_from_extension()      ->
 *                sail_start_reading_file_with_options() ->
 *                sail_read_next_frame()                 ->
 *                sail_stop_reading().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_file_with_options(const char *path, struct sail_context *context,
                                                              const struct sail_plugin_info *plugin_info,
                                                              const struct sail_read_options *read_options, void **state);

/*
 * Starts reading the specified memory buffer with the specified read options. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Read options are deep copied.
 *
 * If read options is NULL, the subsequent calls to sail_read_next_frame() output pixels in BPP24-RGB pixel format
 * for image formats without transparency support and BPP32-RGBA otherwise.
 *
 * Typical usage: sail_plugin_info_from_extension()     ->
 *                sail_start_reading_mem_with_options() ->
 *                sail_read_next_frame()                ->
 *                sail_stop_reading().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_mem_with_options(const void *buffer, size_t buffer_length, struct sail_context *context,
                                                             const struct sail_plugin_info *plugin_info,
                                                             const struct sail_read_options *read_options, void **state);

/*
 * Starts writing the specified image file with the specified write options. Pass a particular plugin info if you'd like
 * to start writing with a specific codec. If not, just pass NULL. If you don't need specific write options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Write options are deep copied.
 *
 * If write options is NULL, the subsequent calls to sail_write_next_frame() output pixels in pixel format
 * as specified in sail_write_features.preferred_output_pixel_format.
 *
 * Typical usage: sail_start_writing_file_with_options() ->
 *                sail_write_next_frame()                ->
 *                sail_stop_writing().
 *
 * Or:            sail_plugin_info_from_extension()      ->
 *                sail_start_writing_file_with_options() ->
 *                sail_write_next_frame()                ->
 *                sail_stop_writing().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_file_with_options(const char *path, struct sail_context *context,
                                                              const struct sail_plugin_info *plugin_info,
                                                              const struct sail_write_options *write_options, void **state);

/*
 * Starts writing the specified memory buffer with the specified write options. If you don't need specific write options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Write options are deep copied.
 *
 * If write options is NULL, the subsequent calls to sail_write_next_frame() output pixels in pixel format
 * as specified in sail_write_features.preferred_output_pixel_format.
 *
 * Typical usage: sail_plugin_info_from_extension()     ->
 *                sail_start_writing_mem_with_options() ->
 *                sail_write_next_frame()               ->
 *                sail_stop_writing().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_mem_with_options(void *buffer, size_t buffer_length, struct sail_context *context,
                                                             const struct sail_plugin_info *plugin_info,
                                                             const struct sail_write_options *write_options, void **state);


/*
 * Stops writing the file started by sail_start_writing_file() and brothers. Assings the number of
 * bytes written. Does nothing if the state is NULL.
 *
 * It's essential to always stop writing to free memory resources. Avoiding doing so will lead to memory leaks.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_writing_with_written(void *state, size_t *written);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
