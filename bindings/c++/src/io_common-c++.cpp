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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN io::pimpl
{
public:
    sail_io sail_io;
};

io::io()
    : d(new pimpl)
{
}

io::io(const io &i)
    : io()
{
    *this = i;
}

io& io::operator=(const io &i)
{
    d->sail_io = i.d->sail_io;
    return *this;
}

io::~io()
{
    delete d;
}

bool io::is_valid() const
{
    return is_valid_private() == 0;
}

sail_error_t io::to_sail_io(sail_io *io) const
{
    SAIL_CHECK_IO_PTR(io);

    *io = d->sail_io;

    return 0;
}

io& io::with_stream(void *stream)
{
    d->sail_io.stream = stream;
    return *this;
}

io& io::with_read(sail_io_read_t read)
{
    d->sail_io.read = read;
    return *this;
}

io& io::with_seek(sail_io_seek_t seek)
{
    d->sail_io.seek = seek;
    return *this;
}

io& io::with_tell(sail_io_tell_t tell)
{
    d->sail_io.tell = tell;
    return *this;
}

io& io::with_write(sail_io_write_t write)
{
    d->sail_io.write = write;
    return *this;
}

io& io::with_flush(sail_io_flush_t flush)
{
    d->sail_io.flush = flush;
    return *this;
}

io& io::with_close(sail_io_close_t close)
{
    d->sail_io.close = close;
    return *this;
}

io& io::with_eof(sail_io_eof_t eof)
{
    d->sail_io.eof = eof;
    return *this;
}

sail_error_t io::is_valid_private() const
{
    sail_io *sail_io = &d->sail_io;

    SAIL_CHECK_IO(sail_io);

    return 0;
}

}
