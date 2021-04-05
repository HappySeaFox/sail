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

#ifndef SAIL_IO_CPP_H
#define SAIL_IO_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
    #include "io_common.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail-common/io_common.h>
#endif

namespace sail
{

/*
 * A C++ interface to struct sail_io.
 */
class SAIL_EXPORT io
{
    // For to_sail_io().
    friend class codec_info;
    friend class image_reader;
    friend class image_writer;
public:
    io();
    io(const io &i);
    io& operator=(const io &i);
    ~io();

    sail_status_t verify_valid() const;

    bool is_valid() const;

    uint64_t id() const;

    io& with_id(uint64_t id);
    io& with_stream(void *stream);
    io& with_tolerant_read(sail_io_tolerant_read_t read);
    io& with_strict_read(sail_io_strict_read_t read);
    io& with_seek(sail_io_seek_t seek);
    io& with_tell(sail_io_tell_t tell);
    io& with_tolerant_write(sail_io_tolerant_write_t write);
    io& with_strict_write(sail_io_strict_write_t write);
    io& with_flush(sail_io_flush_t flush);
    io& with_close(sail_io_close_t close);
    io& with_eof(sail_io_eof_t eof);

private:
    sail_status_t is_valid_private() const;

    sail_status_t to_sail_io(sail_io **io) const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
