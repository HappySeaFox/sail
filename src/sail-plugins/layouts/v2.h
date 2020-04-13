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
 *
 * V2 inherits all the functions from V1.
 *
 * Layout files must be included in reverse order like that:
 *
 * #include "v2.h"
 * #include "v1.h"
 */

#include <sail/error.h>
#include <sail/export.h>

#ifdef __cplusplus
extern "C" {
#endif

/* V2 inherits all the functions from V1. */

/*
 * Decoding functions.
 */

/*
 * Reads a scan line of the current image in the current pass. Allocates a new scan line. The assigned scan line
 * MUST be freed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_scan_line_v2(struct sail_file *file, struct sail_image *image, void **scanline);

/*
 * Encoding functions.
 */

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
