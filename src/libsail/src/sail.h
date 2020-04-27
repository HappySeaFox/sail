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

#ifndef SAIL_SAIL_H
#define SAIL_SAIL_H

/* Universal libsail include. */

#ifdef SAIL_BUILD
    #include "sail-common.h"

    #include "context.h"
    #include "ini.h"
    #include "plugin.h"
    #include "plugin_info.h"
    #include "plugin_info_private.h"
    #include "sail_junior.h"
    #include "sail_advanced.h"
    #include "sail_private.h"
    #include "string_node.h"
#else
    #include <sail/sail-common.h>

    #include <sail/sail_junior.h>
    #include <sail/sail_advanced.h>
    #include <sail/plugin_info.h>
    #include <sail/string_node.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_context;
struct sail_io;
struct sail_plugin_info_node;
struct sail_plugin_info;

/*
 * Initializes SAIL. This is the main entry point to start working with SAIL.
 * Builds a list of available SAIL plugins.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_init(struct sail_context **context);

/*
 * Finilizes working with the specified SAIL context. Frees the context and all its internal
 * memory buffers. The context MUST NOT be used anymore after calling this function.
 * Does nothing if context is NULL.
 */
SAIL_EXPORT void sail_finish(struct sail_context *context);

/*
 * Returns a linked list of found plugin info nodes. Use it to determine the list of possible
 * image formats, file extensions, and mime types that could be hypothetically read or written by SAIL.
 *
 * Returns a pointer to the first plugin info node or NULL when no SAIL plugins were found.
 * Use sail_plugin_info_node.next to iterate.
 */
SAIL_EXPORT const struct sail_plugin_info_node* sail_plugin_info_list(const struct sail_context *context);

/*
 * Finds a first plugin info object that supports reading or writing the specified file path by its file extension.
 * For example: "/test.jpg". The path might not exist.
 *
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Typical usage: sail_plugin_info_from_path() ->
 *                sail_start_reading_file()    ->
 *                sail_read_next_frame()       ->
 *                sail_stop_reading().
 *
 * Or:            sail_plugin_info_from_path() ->
 *                sail_start_writing()         ->
 *                sail_read_next_frame()       ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_from_path(const char *path, const struct sail_context *context,
                                                    const struct sail_plugin_info **plugin_info);

/*
 * Finds a first plugin info object that supports the specified file extension. For example: "jpg".
 *
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Typical usage: sail_plugin_info_from_extension() ->
 *                sail_start_reading_file()         ->
 *                sail_read_next_frame()            ->
 *                sail_stop_reading().
 *
 * Or:            sail_plugin_info_from_extension() ->
 *                sail_start_writing()              ->
 *                sail_read_next_frame()            ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_from_extension(const char *extension, const struct sail_context *context,
                                                         const struct sail_plugin_info **plugin_info);

/*
 * Finds a first plugin info object that supports the specified mime type. For example: "image/jpeg".
 *
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Typical usage: sail_plugin_info_from_mime_type() ->
 *                sail_start_reading_file()         ->
 *                sail_read_next_frame()            ->
 *                sail_stop_reading().
 *
 * Or:            sail_plugin_info_from_mime_type() ->
 *                sail_start_writing()              ->
 *                sail_read_next_frame()            ->
 *                sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_from_mime_type(const struct sail_context *context, const char *mime_type,
                                                         const struct sail_plugin_info **plugin_info);

/*
 * Unloads all loaded plugins from the cache to release memory occupied by them. Use it if you don't want
 * to de-initialize SAIL with sail_finish(), but want to just release some memory. Subsequent attempts
 * to read or write images will reload SAIL plugins from disk.
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_unload_plugins(struct sail_context *context);

/*
 * Starts reading the specified I/O stream with the specified read options. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Read options are deep copied.
 *
 * Typical usage: sail_alloc_io_read_file()            ->
 *                sail_plugin_info_from_extension()    ->
 *                sail_start_reading_io_with_options() ->
 *                sail_read_next_frame()               ->
 *                sail_stop_reading().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options(..., &state),
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
SAIL_EXPORT sail_error_t sail_start_reading_io_with_options(struct sail_io *io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_read_options *read_options, void **state);

/*
 * Starts reading the specified image file with the specified read options. Pass a particular plugin info if you'd like
 * to start reading with a specific codec. If not, just pass NULL. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Read options are deep copied.
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
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading_file_with_options(..., &state),
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
SAIL_EXPORT sail_error_t sail_start_reading_file_with_options(const char *path, struct sail_context *context,
                                                              const struct sail_plugin_info *plugin_info,
                                                              const struct sail_read_options *read_options, void **state);

/*
 * Starts reading the specified I/O stream.
 *
 * Typical usage: sail_alloc_io_read_file()         ->
 *                sail_plugin_info_from_extension() ->
 *                sail_start_reading_io()           ->
 *                sail_read_next_frame()            ->
 *                sail_stop_reading().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading_io(..., &state),
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
SAIL_EXPORT sail_error_t sail_start_reading_io(struct sail_io *io, struct sail_context *context,
                                               const struct sail_plugin_info *plugin_info, void **state);

/*
 * Starts writing the specified I/O stream with the specified write options. If you don't need specific write options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Write options are deep copied.
 *
 * Typical usage: sail_start_writing_with_options() ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing().
 *
 * Or:            sail_plugin_info_from_extension() ->
 *                sail_start_writing_with_options() ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_writing_with_options(..., &state),
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
SAIL_EXPORT sail_error_t sail_start_writing_io_with_options(struct sail_io *io, struct sail_context *context,
                                                            const struct sail_plugin_info *plugin_info,
                                                            const struct sail_write_options *write_options, void **state);

/*
 * Starts writing the specified image file with the specified write options. Pass a particular plugin info if you'd like
 * to start writing with a specific codec. If not, just pass NULL. If you don't need specific write options,
 * just pass NULL. Plugin-specific defaults will be used in this case. Write options are deep copied.
 *
 * Typical usage: sail_start_writing_with_options() ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing().
 *
 * Or:            sail_plugin_info_from_extension() ->
 *                sail_start_writing_with_options() ->
 *                sail_write_next_frame()           ->
 *                sail_stop_writing().
 *
 * For example:
 *
 * void *state = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_writing_with_options(..., &state),
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
SAIL_EXPORT sail_error_t sail_start_writing_file_with_options(const char *path, struct sail_context *context,
                                                              const struct sail_plugin_info *plugin_info,
                                                              const struct sail_write_options *write_options, void **state);

/*
 * Starts writing into the specified I/O stream.
 *
 * Typical usage: sail_alloc_io_write_file()        ->
 *                sail_plugin_info_from_extension() ->
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
SAIL_EXPORT sail_error_t sail_start_writing_io(struct sail_io *io, struct sail_context *context,
                                               const struct sail_plugin_info *plugin_info, void **state);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
