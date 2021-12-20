/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

/*
 * Private functions.
 */

static sail_status_t wrapped_tolerant_read(void *stream, void *buf, size_t size_to_read, size_t *read_size) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.tolerant_read(buf, size_to_read, read_size));

    return SAIL_OK;
}

static sail_status_t wrapped_strict_read(void *stream, void *buf, size_t size_to_read) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.strict_read(buf, size_to_read));

    return SAIL_OK;
}

static sail_status_t wrapped_tolerant_write(void *stream, const void *buf, size_t size_to_write, size_t *written_size) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.tolerant_write(buf, size_to_write, written_size));

    return SAIL_OK;
}

static sail_status_t wrapped_strict_write(void *stream, const void *buf, size_t size_to_write) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.strict_write(buf, size_to_write));

    return SAIL_OK;
}

static sail_status_t wrapped_seek(void *stream, long offset, int whence) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.seek(offset, whence));

    return SAIL_OK;
}

static sail_status_t wrapped_tell(void *stream, size_t *offset) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.tell(offset));

    return SAIL_OK;
}

static sail_status_t wrapped_flush(void *stream) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.flush());

    return SAIL_OK;
}

static sail_status_t wrapped_close(void *stream) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.close());

    return SAIL_OK;
}

static sail_status_t wrapped_eof(void *stream, bool *result) {

    sail::abstract_io &abstract_io = *reinterpret_cast<sail::abstract_io *&>(stream);

    SAIL_TRY(abstract_io.eof(result));

    return SAIL_OK;
}

class SAIL_HIDDEN abstract_io_adapter::pimpl
{
public:
    explicit pimpl(sail::abstract_io &other_abstract_io)
        : abstract_io(other_abstract_io)
    {
        sail_io.id             = abstract_io.id();
        sail_io.features       = abstract_io.features();
        sail_io.stream         = &abstract_io;
        sail_io.tolerant_read  = wrapped_tolerant_read;
        sail_io.strict_read    = wrapped_strict_read;
        sail_io.tolerant_write = wrapped_tolerant_write;
        sail_io.strict_write   = wrapped_strict_write;
        sail_io.seek           = wrapped_seek;
        sail_io.tell           = wrapped_tell;
        sail_io.flush          = wrapped_flush;
        sail_io.close          = wrapped_close;
        sail_io.eof            = wrapped_eof;
    }

    sail::abstract_io &abstract_io;
    struct sail_io sail_io;
};

abstract_io_adapter::abstract_io_adapter(sail::abstract_io &abstract_io)
    : d(new pimpl(abstract_io))
{
}

abstract_io_adapter::~abstract_io_adapter()
{
}

struct sail_io& abstract_io_adapter::sail_io_c() const
{
    return d->sail_io;
}

}
