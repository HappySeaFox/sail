/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <cstring>
#include <stdexcept>

#include <sail/sail.h>

#include <sail-c++/sail-c++.h>

namespace sail
{

io_expanding_buffer::io_expanding_buffer(std::size_t initial_capacity)
    : io_base(nullptr)
{
    struct sail_io *sail_io_local;

    SAIL_TRY_OR_EXECUTE(sail_alloc_io_write_expanding_buffer(initial_capacity, &sail_io_local),
                        /* on error */ throw std::bad_alloc());

    d->sail_io_wrapper.reset(sail_io_local);
}

io_expanding_buffer::~io_expanding_buffer()
{
}

std::size_t io_expanding_buffer::size() const
{
    std::size_t result_size;

    SAIL_TRY_OR_EXECUTE(sail_io_expanding_buffer_size(d->sail_io_wrapper.get(), &result_size),
                        /* on error */ throw std::runtime_error("Failed to get expanding buffer size"));

    return result_size;
}

codec_info io_expanding_buffer::codec_info()
{
    return sail::codec_info::from_magic_number(*this);
}

} // namespace sail
