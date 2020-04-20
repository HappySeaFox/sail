/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    /* Universal include. */

    /* libsail-common. */
    #include <sail/common.h>
    #include <sail/config.h>
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/meta_entry_node.h>
    #include <sail/log.h>
    #include <sail/utils.h>

    /* libsail. */
    #include <sail/plugin_info.h>
    #include <sail/string_node.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_context;
struct sail_plugin_info_node;
struct sail_plugin_info;
struct sail_plugin;

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
 * Finds a first plugin info object that supports the specified file extension. For example: "jpg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Typical usage: sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_reading_with_plugin() ->
 *                sail_read_next_frame() -> sail_stop_reading().
 * Or:            sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_writing_with_plugin() ->
 *                sail_write_next_frame() -> sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_extension(const struct sail_context *context, const char *extension, const struct sail_plugin_info **plugin_info);

/*
 * Finds a first plugin info object that supports the specified mime type. For example: "image/jpeg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Typical usage: sail_plugin_info_by_mime_type() -> sail_load_plugin() -> sail_start_reading_with_plugin() ->
 *                sail_read_next_frame() -> sail_stop_reading().
 * Or:            sail_plugin_info_by_mime_type() -> sail_load_plugin() -> sail_start_writing_with_plugin() ->
 *                sail_write_next_frame() -> sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_mime_type(const struct sail_context *context, const char *mime_type, const struct sail_plugin_info **plugin_info);

/*
 * Loads the plugin addressed by the specified plugin info. The assigned plugin object MUST NOT be destroyed.
 * It's a pointer to an internal data structure. Caches the loaded plugin in the internal storage, so a subsequent
 * call with the same plugin info just returns the cached plugin pointer.
 *
 * Typical usage: sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_reading_with_plugin() ->
 *                sail_read_next_frame() -> sail_stop_reading().
 * Or:            sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_writing_with_plugin() ->
 *                sail_write_next_frame() -> sail_stop_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_load_plugin(struct sail_context *context, const struct sail_plugin_info *plugin_info, const struct sail_plugin **plugin);

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
 * Loads the specified image and returns its properties without pixel data. The assigned image MUST be destroyed later
 * with sail_destroy_image(). The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal
 * data structure. If you don't need it, just pass NULL.
 *
 * This function is pretty fast as it doesn't decode whole image data for most image formats.
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_probe_image(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, struct sail_image **image);

/*
 * Loads the specified image file and returns its properties and pixel data. The assigned image MUST be destroyed later
 * with sail_destroy_image(). The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal
 * data structure. If you don't need it, just pass NULL. The assigned pixel data MUST be destroyed later free().
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_read(const char *path, struct sail_context *context, struct sail_image **image, void **image_bits,
                                    const struct sail_plugin_info **plugin_info);

/*
 * Writes the specified image file its pixel data into the fle. The assigned plugin info MUST NOT be destroyed.
 * It's a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write(const char *path, struct sail_context *context, const struct sail_image *image, const void *image_bits,
                                    const struct sail_plugin_info **plugin_info);

/*
 * Starts reading the specified image with the specified plugin and read options. If you don't need specific read options,
 * just pass NULL. Plugin-specific defaults will be used in this case.
 *
 * Typical usage: sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_reading_with_plugin() ->
 *                sail_read_next_frame() -> sail_stop_reading().
 *
 * For example:
 *
 * SAIL_TRY(sail_plugin_info_by_extension(...));
 * SAIL_TRY(sail_load_plugin(...));
 *
 * void *pimpl = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading_with_plugin(..., &pimpl),
 *                     sail_stop_reading(pimpl));
 * SAIL_TRY_OR_CLEANUP(sail_read_next_frame(pimpl, ...),
 *                     sail_stop_reading(pimpl));
 * SAIL_TRY(sail_stop_reading(pimpl));
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. Pimpls must be used per image. DO NOT use the same pimpl
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading_with_plugin(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                        const struct sail_plugin *plugin, const struct sail_read_options *read_options,
                                                        void **pimpl);

/*
 * Starts reading the specified image. The assigned plugin info MUST NOT be destroyed.
 * It's a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * Typical usage: sail_start_reading() -> sail_read_next_frame() -> sail_stop_reading().
 *
 * For example:
 *
 * void *pimpl = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_reading(..., &pimpl),
 *                     sail_stop_reading(pimpl));
 * SAIL_TRY_OR_CLEANUP(sail_read_next_frame(pimpl, ...),
 *                     sail_stop_reading(pimpl));
 * SAIL_TRY(sail_stop_reading(pimpl));
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. Pimpls must be used per image. DO NOT use the same pimpl
 * to read multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, void **pimpl);

/*
 * Continues reading the file started by sail_start_reading() or sail_start_reading_with_plugin().
 * The assigned image MUST be destroyed later with sail_image_destroy(). The assigned image bits
 * MUST be destroyed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 * Returns SAIL_NO_MORE_FRAMES when no more frames are available.
 */
SAIL_EXPORT sail_error_t sail_read_next_frame(void *pimpl, struct sail_image **image, void **image_bits);

/*
 * Stops reading the file started by sail_start_reading() or sail_start_reading_with_plugin().
 * Does nothing if the pimpl is NULL.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_reading(void *pimpl);

/*
 * Starts writing the specified image file with the specified plugin and write options. If you don't need
 * specific write options, just pass NULL. Plugin-specific defaults will be used in this case.
 *
 * Typical usage: sail_plugin_info_by_extension() -> sail_load_plugin() -> sail_start_writing_with_plugin() ->
 *                sail_write_next_frame() -> sail_stop_writing().
 *
 * For example:
 *
 * SAIL_TRY(sail_plugin_info_by_extension(...));
 * SAIL_TRY(sail_load_plugin(...));
 *
 * void *pimpl = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_writing_with_plugin(..., &pimpl),
 *                     sail_stop_writing(pimpl));
 * SAIL_TRY_OR_CLEANUP(sail_write_next_frame(pimpl, ...),
 *                     sail_stop_writing(pimpl));
 * SAIL_TRY(sail_stop_writing(pimpl));
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. Pimpls must be used per image. DO NOT use the same pimpl
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing_with_plugin(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info,
                                                        const struct sail_plugin *plugin, const struct sail_write_options *write_options,
                                                        void **pimpl);

/*
 * Starts writing into the specified image file. The assigned plugin info MUST NOT be destroyed.
 * It's a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * Typical usage: sail_start_writing() -> sail_write_next_frame() -> sail_stop_writing().
 *
 * For example:
 *
 * void *pimpl = NULL;
 *
 * SAIL_TRY_OR_CLEANUP(sail_start_writing(..., &pimpl),
 *                     sail_stop_writing(pimpl));
 * SAIL_TRY_OR_CLEANUP(sail_write_next_frame(pimpl, ...),
 *                     sail_stop_writing(pimpl));
 * SAIL_TRY(sail_stop_writing(pimpl));
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. Pimpls must be used per image. DO NOT use the same pimpl
 * to write multiple images in the same time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, void **pimpl);

/*
 * Continues writing the file started by sail_start_writing() or sail_start_writing_with_plugin().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write_next_frame(void *pimpl, const struct sail_image *image, const void *image_bits);

/*
 * Stops writing the file started by sail_start_writing() or sail_start_writing_with_plugin().
 * Does nothing if the pimpl is NULL.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_writing(void *pimpl);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
