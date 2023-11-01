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

#include <stdexcept>

#include <sail-c++/sail-c++.h>
#include <sail/sail.h>

namespace sail
{

template<typename BufferType>
static sail_io *construct_sail_io(BufferType buffer, std::size_t buffer_size);

template<>
sail_io *construct_sail_io<void *>(void *buffer, std::size_t buffer_size)
{
    struct sail_io *sail_io;

    SAIL_TRY_OR_EXECUTE(sail_alloc_io_read_write_memory(buffer, buffer_size, &sail_io),
                        /* on error */ throw std::bad_alloc());

    return sail_io;
}

template<>
sail_io *construct_sail_io<const void *>(const void *buffer, std::size_t buffer_size)
{
    struct sail_io *sail_io;

    SAIL_TRY_OR_EXECUTE(sail_alloc_io_read_memory(buffer, buffer_size, &sail_io),
                        /* on error */ throw std::bad_alloc());

    return sail_io;
}

io_memory::io_memory(void *buffer, std::size_t buffer_size)
    : io_base(construct_sail_io(buffer, buffer_size))
{
}

io_memory::io_memory(const void *buffer, std::size_t buffer_size)
    : io_base(construct_sail_io(buffer, buffer_size))
{
}

io_memory::io_memory(void *buffer, std::size_t buffer_size, Operation operation)
    : io_base(construct_sail_io(operation == Operation::Read ? const_cast<const void *>(buffer) : buffer, buffer_size))
{
}

io_memory::io_memory(sail::arbitrary_data &arbitrary_data)
    : io_memory(arbitrary_data.data(), arbitrary_data.size())
{
}

io_memory::io_memory(const sail::arbitrary_data &arbitrary_data)
    : io_memory(arbitrary_data.data(), arbitrary_data.size())
{
}

io_memory::io_memory(sail::arbitrary_data &arbitrary_data, Operation operation)
    : io_memory(operation == Operation::Read ? const_cast<const sail::arbitrary_data &>(arbitrary_data).data() : arbitrary_data.data(),
                arbitrary_data.size())
{
}

io_memory::~io_memory()
{
}

codec_info io_memory::codec_info()
{
    return sail::codec_info::from_magic_number(*this);
}

}
