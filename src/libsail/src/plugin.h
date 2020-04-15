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

#ifndef SAIL_PLUGIN_H
#define SAIL_PLUGIN_H

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

/*
 * Currently supported plugin layout version.
 */
#define SAIL_PLUGIN_LAYOUT_V2 2

struct sail_plugin_info;

struct sail_read_features;
struct sail_read_options;
struct sail_write_features;
struct sail_write_options;
struct sail_file;
struct sail_image;

/* V2 declarations. */
typedef sail_error_t (*sail_plugin_read_features_v2_t)(struct sail_read_features **read_features);
typedef sail_error_t (*sail_plugin_read_init_v2_t)(struct sail_file *file, const struct sail_read_options *read_options);
typedef sail_error_t (*sail_plugin_read_seek_next_frame_v2_t)(struct sail_file *file, struct sail_image **image);
typedef sail_error_t (*sail_plugin_read_seek_next_pass_v2_t)(struct sail_file *file, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_read_scan_line_v2_t)(struct sail_file *file, const struct sail_image *image, void *scanline);
typedef sail_error_t (*sail_plugin_read_alloc_scan_line_v2_t)(struct sail_file *file, const struct sail_image *image, void **scanline);
typedef sail_error_t (*sail_plugin_read_finish_v2_t)(struct sail_file *file);

typedef sail_error_t (*sail_plugin_write_features_v2_t)(struct sail_write_features **write_features);
typedef sail_error_t (*sail_plugin_write_init_v2_t)(struct sail_file *file, const struct sail_write_options *write_options);
typedef sail_error_t (*sail_plugin_write_seek_next_frame_v2_t)(struct sail_file *file, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_write_seek_next_pass_v2_t)(struct sail_file *file, const struct sail_image *image);
typedef sail_error_t (*sail_plugin_write_scan_line_v2_t)(struct sail_file *file, const struct sail_image *image, const void *scanline);
typedef sail_error_t (*sail_plugin_write_finish_v2_t)(struct sail_file *file);

struct sail_plugin_layout_v2 {
    sail_plugin_read_features_v2_t        read_features_v2;
    sail_plugin_read_init_v2_t            read_init_v2;
    sail_plugin_read_seek_next_frame_v2_t read_seek_next_frame_v2;
    sail_plugin_read_seek_next_pass_v2_t  read_seek_next_pass_v2;
    sail_plugin_read_scan_line_v2_t       read_scan_line_v2;
    sail_plugin_read_alloc_scan_line_v2_t read_alloc_scan_line_v2;
    sail_plugin_read_finish_v2_t          read_finish_v2;

    sail_plugin_write_features_v2_t        write_features_v2;
    sail_plugin_write_init_v2_t            write_init_v2;
    sail_plugin_write_seek_next_frame_v2_t write_seek_next_frame_v2;
    sail_plugin_write_seek_next_pass_v2_t  write_seek_next_pass_v2;
    sail_plugin_write_scan_line_v2_t       write_scan_line_v2;
    sail_plugin_write_finish_v2_t          write_finish_v2;
};

/*
 * SAIL plugin.
 */
struct sail_plugin {

    /* Layout version. */
    int layout;

    /* System-specific library handle. */
    void *handle;

    /* Plugin interface. */
    struct sail_plugin_layout_v2 *v2;
};

typedef struct sail_plugin sail_plugin_t;

/*
 * Loads the specified plugin by its info and saves its handle and exported interfaces into the specified plugin
 * instance. The assigned plugin MUST be destroyed later with sail_destroy_plugin().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_plugin(const struct sail_plugin_info *plugin_info, struct sail_plugin **plugin);

/*
 * Destroys the specified plugin and all its internal memory buffers.
 * The plugin MUST NOT be used anymore after calling this function.
 */
SAIL_EXPORT void sail_destroy_plugin(struct sail_plugin *plugin);

/*
 * Reads plugin read features from the specified plugin. The assigned read features MUST be destroyed
 * later with sail_destroy_read_features().
 */
SAIL_EXPORT sail_error_t sail_plugin_read_features(const struct sail_plugin *plugin, struct sail_read_features **read_features);

/*
 * Reads plugin write features from the specified plugin. The assigned write features MUST be destroyed
 * later with sail_destroy_write_features().
 */
SAIL_EXPORT sail_error_t sail_plugin_write_features(const struct sail_plugin *plugin, struct sail_write_features **write_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
