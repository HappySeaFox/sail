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
    #include <sail/plugin.h>
    #include <sail/string_node.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_plugin_info_node;
struct sail_plugin_info;

struct sail_context {

    struct sail_plugin_info_node *plugin_info_node;
};

typedef struct sail_context sail_context_t;

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
 */
SAIL_EXPORT void sail_finish(struct sail_context *context);

/*
 * Finds a first plugin info object that supports the specified file extension. For example: "jpg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_extension(const struct sail_context *context, const char *extension, const struct sail_plugin_info **plugin_info);

/*
 * Finds a first plugin info object that supports the specified mime type. For example: "image/jpeg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_mime_type(const struct sail_context *context, const char *mime_type, const struct sail_plugin_info **plugin_info);

/*
 * Loads the plugin addressed by the specified plugin info. The assigned plugin object MUST NOT be destroyed.
 * It's a pointer to an internal data structure. Caches the loaded plugin in the internal storage, so a subsequent
 * call with the same plugin info just returns the cached plugin pointer.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_load_plugin(struct sail_context *context, const struct sail_plugin_info *plugin_info, const struct sail_plugin **plugin);

/*
 * Unloads all loaded plugins from the cache to release memory occupied by them. Use it if you don't want
 * to de-initialize SAIL with sail_finish(), but want just to release some memory.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_unload_plugins(struct sail_context *context);

/*
 * Loads the specified image and returns its properties. The assigned image MUST be destroyed later
 * with sail_destroy_image(). The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal
 * data structure. If you don't need it, just pass NULL.
 *
 * This function is pretty fast as it doesn't decode whole image data for most image formats.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_probe_image(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, struct sail_image **image);

/*
 * Starts reading the specified image. The assigned plugin info MUST NOT be destroyed.
 * It's a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. For example:
 *
 * void *pimpl;
 * sail_start_reading(..., &pimpl);
 * sail_read_next_frame(pimpl, ...);
 * sail_stop_reading(pimpl);
 *
 * Typical usage: sail_start_reading -> sail_read_next_frame -> sail_stop_reading
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_reading(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, void **pimpl);

/*
 * Continues reading the image started by sail_start_reading(). The assigned image MUST be destroyed later
 * with sail_image_destroy(). The assigned image bits MUST be destroyed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_read_next_frame(void *pimpl, struct sail_image **image, void **image_bits);

/*
 * Stops reading started by sail_start_reading().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_reading(void *pimpl);

/*
 * Starts writing into the specified image file. The assigned plugin info MUST NOT be destroyed.
 * It's a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * PIMPL explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. For example:
 *
 * void *pimpl;
 * sail_start_writing(..., &pimpl);
 * sail_write_next_frame(pimpl, ...);
 * sail_stop_writing(pimpl);
 *
 * Typical usage: sail_start_writing -> sail_write_next_frame -> sail_stop_writing
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_start_writing(const char *path, struct sail_context *context,
                                            const struct sail_plugin_info **plugin_info, void **pimpl);

/*
 * Continues writing the image started by sail_start_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write_next_frame(void *pimpl, struct sail_image *image, void *image_bits);

/*
 * Stops writing started by sail_start_writing().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_stop_writing(void *pimpl);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
