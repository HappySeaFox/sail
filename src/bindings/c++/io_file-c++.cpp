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

#include <stdexcept>

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN io_file::io_file_pimpl
{
public:
    io_file_pimpl(const std::string &path)
        : codec_info(sail::codec_info::from_path(path))
    {
    }

    const sail::codec_info codec_info;
};

static struct sail_io *construct_sail_io(const std::string &path, io_file::Operation operation)
{
    struct sail_io *sail_io;

    switch (operation) {
        case io_file::Operation::Read:
            SAIL_TRY_OR_EXECUTE(sail_alloc_io_read_file(path.c_str(), &sail_io),
                                /* on error */ throw std::bad_alloc());
        break;
        case io_file::Operation::ReadWrite:
            SAIL_TRY_OR_EXECUTE(sail_alloc_io_read_write_file(path.c_str(), &sail_io),
                                /* on error */ throw std::bad_alloc());
        break;
        default: {
            throw std::runtime_error("Unknown file operation");
        }
    }

    return sail_io;
}

io_file::io_file(const std::string &path)
    : io_file(path, Operation::Read)
{
}

io_file::io_file(const std::string &path, io_file::Operation operation)
    : io_base(construct_sail_io(path, operation))
    , file_d(new io_file_pimpl(path))
{
}

io_file::~io_file()
{
}

codec_info io_file::codec_info()
{
    return file_d->codec_info;
}

}
