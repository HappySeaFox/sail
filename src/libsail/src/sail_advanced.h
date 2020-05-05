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

#ifndef SAIL_SAIL_ADVANCED_H
#define SAIL_SAIL_ADVANCED_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_context;
struct sail_plugin_info;

/*
 * Starts reading the specified image file. Pass a particular plugin info if you'd like
 * to start reading with a specific codec. If not, just pass NULL.
 *
 * The subsequent calls to sail_read_next_frame() output pixels in BPP24-RGB pixel format for image formats
 * without transparency support and BPP32-RGBA otherwise.
 *
 * Typical usage: sail_start_reading_file() ->
 *                sail_read_next_frame()    ->
 *                sail_stop_reading().
 *
 * Or:            sail_plugin_info_from_extension() ->
 *                sail_start_reading_file()         ->
 *                sail_read_next_frame()            ->
 *                sail_stop_reading().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading_file(..., &state),
 *                     sail_stop_reading(state));
 * SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, ...),
 *                     sail_stop_reading(state));
 * SAIL_TRY(sail_stop_reading(state));
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_file(const char *path, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info, void **state);

/*
 * Continues reading the file started by sail_start_reading_file() and brothers.
 * The assigned image MUST be destroyed later with sail_image_destroy(). The assigned image bits
 * MUST be destroyed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 * Returns SAIL_NO_MORE_FRAMES when no more frames are available.
 */
SAIL_EXPORT sail_error_t sail_read_next_frame(void *state, struct sail_image **image, void **image_bits);

/*
 * Stops reading the file started by sail_start_reading_file() and brothers.
 * Does nothing if the state is NULL.
 *
 * It's essential to always stop reading to free memory resources. Avoiding doing so will lead to memory leaks.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_reading(void *state);

/*
 * Starts writing into the specified image file. Pass a particular plugin info if you'd like
 * to start writing with a specific codec. If not, just pass NULL.
 *
 * The subsequent calls to sail_write_next_frame() output pixels in pixel format as specified
 * in sail_write_features.preferred_output_pixel_format.
 *
 * Typical usage: sail_start_writing()    ->
 *                sail_write_next_frame() ->
 *                sail_stop_writing().
 *
 * Or:            sail_plugin_info_from_extension() ->
 *                sail_start_writing()              ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_writing(..., &state),
 *                     sail_stop_writing(state));
 * SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, ...),
 *                     sail_stop_writing(state));
 * SAIL_TRY(sail_stop_writing(state));
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_file(const char *path, struct sail_context *context,
                                                 const struct sail_plugin_info *plugin_info, void **state);

/*
 * Continues writing the file started by sail_start_writing_file() and brothers.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write_next_frame(void *state, const struct sail_image *image, const void *image_bits);

/*
 * Stops writing the file started by sail_start_writing_file() and brothers.
 * Does nothing if the state is NULL.
 *
 * It's essential to always stop writing to free memory resources. Avoiding doing so will lead to memory leaks.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_writing(void *state);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
