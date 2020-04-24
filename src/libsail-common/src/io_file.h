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

#ifndef SAIL_IO_FILE_H
#define SAIL_IO_FILE_H

#include <stdio.h>

#include "error.h"
#include "export.h"

/*
 * File I/O implementation.
 */

SAIL_HIDDEN sail_error_t sail_io_file_read(void *stream, void *buf, size_t object_size, size_t objects_count);

SAIL_HIDDEN sail_error_t sail_io_file_seek(void *stream, long offset, int whence);

SAIL_HIDDEN sail_error_t sail_io_file_tell(void *stream, long *offset);

SAIL_HIDDEN sail_error_t sail_io_file_write(void *stream, const void *buf, size_t object_size, size_t objects_count);

SAIL_HIDDEN sail_error_t sail_io_file_close(void *stream);

#endif
