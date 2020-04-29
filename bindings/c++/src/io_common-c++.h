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

#ifndef SAIL_IO_CPP_H
#define SAIL_IO_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
    #include "io_common.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/io_common.h>
#endif

namespace sail
{

/*
 * A C++ interface to struct sail_io.
 */
class SAIL_EXPORT io
{
public:
    io();
    io(const io &io);
    io& operator=(const io &io);
    ~io();

    bool is_valid() const;

    sail_error_t to_sail_io(sail_io *io) const;

    io& with_stream(void *stream);
    io& with_read(sail_io_read_t read);
    io& with_seek(sail_io_seek_t seek);
    io& with_tell(sail_io_tell_t tell);
    io& with_write(sail_io_write_t write);
    io& with_flush(sail_io_flush_t flush);
    io& with_close(sail_io_close_t close);
    io& with_eof(sail_io_eof_t eof);

private:
    sail_error_t is_valid_private() const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
