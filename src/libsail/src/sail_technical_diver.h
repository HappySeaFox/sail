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

#ifndef SAIL_SAIL_TECHNICAL_DIVER_H
#define SAIL_SAIL_TECHNICAL_DIVER_H

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
 * Starts reading the specified I/O stream.
 *
 * Outputs pixels in BPP24-RGB pixel format for image formats without transparency support and BPP32-RGBA otherwise.
 *
 * Typical usage: sail_alloc_io()                   ->
 *                set I/O callbacks                 ->
 *                sail_plugin_info_from_extension() ->
 *                sail_start_reading_io()           ->
 *                sail_read_next_frame()            ->
 *                sail_stop_reading()               ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_io(struct sail_io *io, struct sail_context *context,
                                               const struct sail_plugin_info *plugin_info, void **state);

/*
 * Starts reading the specified I/O stream with the specified read options. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Read options are deep copied.
 *
 * If read options is NULL, the subsequent calls to sail_read_next_frame() output pixels in BPP24-RGB pixel format
 * for image formats without transparency support and BPP32-RGBA otherwise.
 *
 * Typical usage: sail_alloc_io()                      ->
 *                set I/O callbacks                    ->
 *                sail_plugin_info_from_extension()    ->
 *                sail_start_reading_io_with_options() ->
 *                sail_read_next_frame()               ->
 *                sail_stop_reading()                  ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_io_with_options(struct sail_io *io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_read_options *read_options, void **state);

/*
 * Starts writing into the specified I/O stream.
 *
 * The subsequent calls to sail_write_next_frame() output pixels in pixel format as specified
 * in sail_write_features.preferred_output_pixel_format.
 *
 * Typical usage: sail_alloc_io()                   ->
 *                set I/O callbacks                 ->
 *                sail_plugin_info_from_extension() ->
 *                sail_start_writing()              ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing()               ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_io(struct sail_io *io, struct sail_context *context,
                                               const struct sail_plugin_info *plugin_info, void **state);

/*
 * Starts writing the specified I/O stream with the specified write options. If you don't need specific write options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Write options are deep copied.
 *
 * If write options is NULL, the subsequent calls to sail_write_next_frame() output pixels in pixel format
 * as specified in sail_write_features.preferred_output_pixel_format.
 *
 * Typical usage: sail_alloc_io()                      ->
 *                set I/O callbacks                    ->
 *                sail_plugin_info_from_extension()    ->
 *                sail_start_writing_io_with_options() ->
 *                sail_write_next_frame()              ->
 *                sail_stop_writing()                  ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_io_with_options(struct sail_io *io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_write_options *write_options, void **state);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
