/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include <sail-c++/sail-c++.h>

namespace sail
{

io_base::io_base(struct sail_io *sail_io)
    : d(new pimpl(sail_io))
{
}

io_base::~io_base()
{
}

int io_base::features() const
{
    return d->sail_io_wrapper->features;
}

sail_status_t io_base::tolerant_read(void *buf, std::size_t size_to_read, std::size_t *read_size)
{
    SAIL_TRY(d->sail_io_wrapper->tolerant_read(d->sail_io_wrapper->stream, buf, size_to_read, read_size));

    return SAIL_OK;
}

sail_status_t io_base::strict_read(void *buf, std::size_t size_to_read)
{
    SAIL_TRY(d->sail_io_wrapper->strict_read(d->sail_io_wrapper->stream, buf, size_to_read));

    return SAIL_OK;
}

sail_status_t io_base::tolerant_write(const void *buf, std::size_t size_to_write, std::size_t *written_size)
{
    SAIL_TRY(d->sail_io_wrapper->tolerant_write(d->sail_io_wrapper->stream, buf, size_to_write, written_size));

    return SAIL_OK;
}

sail_status_t io_base::strict_write(const void *buf, std::size_t size_to_write)
{
    SAIL_TRY(d->sail_io_wrapper->strict_write(d->sail_io_wrapper->stream, buf, size_to_write));

    return SAIL_OK;
}

sail_status_t io_base::seek(long offset, int whence)
{
    SAIL_TRY(d->sail_io_wrapper->seek(d->sail_io_wrapper->stream, offset, whence));

    return SAIL_OK;
}

sail_status_t io_base::tell(std::size_t *offset)
{
    SAIL_TRY(d->sail_io_wrapper->tell(d->sail_io_wrapper->stream, offset));

    return SAIL_OK;
}

sail_status_t io_base::flush()
{
    SAIL_TRY(d->sail_io_wrapper->flush(d->sail_io_wrapper->stream));

    return SAIL_OK;
}

sail_status_t io_base::close()
{
    SAIL_TRY(d->sail_io_wrapper->close(d->sail_io_wrapper->stream));

    return SAIL_OK;
}

sail_status_t io_base::eof(bool *result)
{
    SAIL_TRY(d->sail_io_wrapper->eof(d->sail_io_wrapper->stream, result));

    return SAIL_OK;
}

}
