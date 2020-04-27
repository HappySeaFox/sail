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

#ifndef SAIL_SAIL_JUNIOR_H
#define SAIL_SAIL_JUNIOR_H

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
struct sail_image;
struct sail_io;
struct sail_plugin_info;

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
SAIL_EXPORT sail_error_t sail_probe(const char *path, struct sail_context *context,
                                    struct sail_image **image, const struct sail_plugin_info **plugin_info);

/*
 * Loads the specified image file and returns its properties and pixel data. The assigned image MUST be destroyed later
 * with sail_destroy_image(). The assigned pixel data MUST be destroyed later with free().
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_read(const char *path, struct sail_image **image, void **image_bits);

/*
 * Writes the specified image file its pixel data into the file.
 *
 * Typical usage: this is a standalone function that could be called at any time.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write(const char *path, const struct sail_image *image, const void *image_bits);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
