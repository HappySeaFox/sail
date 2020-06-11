/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
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
    pimpl()
    {
        empty_sail_io();
    }

    void empty_sail_io();

    struct sail_io sail_io;
};

void io::pimpl::empty_sail_io()
{
    sail_io.stream = nullptr;
    sail_io.read   = nullptr;
    sail_io.seek   = nullptr;
    sail_io.tell   = nullptr;
    sail_io.write  = nullptr;
    sail_io.flush  = nullptr;
    sail_io.close  = nullptr;
    sail_io.eof    = nullptr;
}

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
