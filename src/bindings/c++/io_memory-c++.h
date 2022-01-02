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

#ifndef SAIL_IO_MEMORY_CPP_H
#define SAIL_IO_MEMORY_CPP_H

#include <cstddef>

#ifdef SAIL_BUILD
    #include "io_base-c++.h"
#else
    #include <sail-c++/io_base-c++.h>
#endif

namespace sail
{

/*
 * Memory I/O stream.
 */
class SAIL_EXPORT io_memory : public io_base
{
public:
    /*
     * Opens the specified memory buffer for reading and writing.
     */
    io_memory(void *buffer, std::size_t buffer_length);

    /*
     * Opens the specified memory buffer for reading.
     */
    io_memory(const void *buffer, std::size_t buffer_length);

    /*
     * Destroys the memory I/O stream.
     */
    ~io_memory() override;

    /*
     * Finds and returns a first codec info object that supports the magic number read
     * from the memory buffer. The comparison algorithm is case insensitive. After reading
     * a magic number, rewinds the I/O cursor position back to the previous position.
     *
     * Not all codecs support magic numbers. That's why it's not guaranteed that this method
     * returns a valid codec info object.
     *
     * Returns an invalid codec info object on error.
     */
    sail::codec_info codec_info() override;
};

}

#endif
