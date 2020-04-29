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

#ifndef SAIL_IO_NOOP_H
#define SAIL_IO_NOOP_H

#include <stdbool.h>
#include <stdio.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

/*
 * No-op callbacks that just return SAIL_NOT_IMPLEMENTED.
 */
SAIL_HIDDEN sail_error_t io_noop_read(void *stream, void *buf, size_t object_size, size_t objects_count, size_t *read_objects_count);
SAIL_HIDDEN sail_error_t io_noop_seek(void *stream, long offset, int whence);
SAIL_HIDDEN sail_error_t io_noop_tell(void *stream, long *offset);
SAIL_HIDDEN sail_error_t io_noop_write(void *stream, const void *buf, size_t object_size, size_t objects_count, size_t *written_objects_count);
SAIL_HIDDEN sail_error_t io_noop_flush(void *stream);
SAIL_HIDDEN sail_error_t io_noop_close(void *stream);
SAIL_HIDDEN sail_error_t io_noop_eof(void *stream, bool *result);

#endif
