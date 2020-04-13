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

#ifndef SAIL_PLUGIN_LAYOUT_V2_H
#define SAIL_PLUGIN_LAYOUT_V2_H

/*
 * This is a plugin layout definition file (version 2).
 *
 * It's intedened to be used as a reference how plugins V2 are organized. It's also could
 * be used by plugins' developers to compile their plugins directly into a testing application
 * to simplify debugging.
 */

#include <sail/error.h>
#include <sail/export.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Decoding functions.
 */

/*
 * Assigns possible read features for this plugin. The assigned read features MUST be destroyed later
 * with sail_destroy_read_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_features_v2(struct sail_read_features **read_features);

/*
 * Starts decoding the specified file using the specified options (or NULL to use defaults).
 * The specified read options will be copied into an internal buffer.
 *
 * If the specified read options is NULL, plugin-specific defaults will be used.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_init_v2(struct sail_file *file, struct sail_read_options *read_options);

/*
 * Seeks to the next frame. The frame is NOT immediately read or decoded by most SAIL plugins. One could
 * use this method to quickly detect the image dimensions without parsing the whole file or frame.
 *
 * Use sail_plugin_read_seek_next_pass() + sail_plugin_read_scan_line() to actually read the frame.
 * The assigned image MUST be destroyed later with sail_destroy_image().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_frame_v2(struct sail_file *file, struct sail_image **image);

/*
 * Seeks to the next pass if the specified image has multiple passes. Does nothing otherwise.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_seek_next_pass_v2(struct sail_file *file, struct sail_image *image);

/*
 * Reads a scan line of the current image in the current pass. The specified scan line must be
 * allocated by the caller and must be be large enough. Use bytes_per_line field to calculate
 * the necessary length of a scan line.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_scan_line_v2(struct sail_file *file, struct sail_image *image, void *scanline);

/*
 * Reads a scan line of the current image in the current pass. Allocates a new scan line. The assigned scan line
 * MUST be freed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_alloc_scan_line_v2(struct sail_file *file, struct sail_image *image, void **scanline);

/*
 * Finilizes reading operation. No more readings are possible after calling this function.
 * This function doesn't close the file. It just stops decoding. Use sail_destroy_file()
 * to actually close the file.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_finish_v2(struct sail_file *file);

/*
 * Encoding functions.
 */

/*
 * Assigns possible write features for this plugin. The assigned write features MUST be destroyed later
 * with sail_destroy_write_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_features_v2(struct sail_write_features **write_features);

/*
 * Starts encoding the specified file using the specified options (or NULL to use defaults).
 * The specified write options will be copied into an internal buffer.
 *
 * If the specified write options is NULL, plugin-specific defaults will be used.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_init_v2(struct sail_file *file, struct sail_write_options *write_options);

/*
 * Seeks to a next frame before writing it. The frame is NOT immediately written. Use sail_plugin_write_seek_next_pass()
 * and sail_plugin_write_scan_line() to actually write a frame.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_frame_v2(struct sail_file *file, struct sail_image *image);

/*
 * Seeks to a next pass before writing it if the specified image is interlaced. Does nothing otherwise.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_seek_next_pass_v2(struct sail_file *file, struct sail_image *image);

/*
 * Writes a scan line of the current image in the current pass.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_scan_line_v2(struct sail_file *file, struct sail_image *image, void *scanline);

/*
 * Finilizes writing operation. No more writings are possible after calling this function.
 * This function doesn't close the file. Use sail_destroy_file() for that.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_write_finish_v2(struct sail_file *file);


/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
