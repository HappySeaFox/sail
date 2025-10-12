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

#pragma once

#include <cstddef> /* std::size_t */

#include <sail-c++/io_base.h>

namespace sail
{

/*
 * Expanding buffer I/O stream. A writable memory buffer that automatically
 * grows as data is written to it. Useful for encoding images to memory
 * without pre-allocating a fixed-size buffer. The growth factor is 1.5x.
 */
class SAIL_EXPORT io_expanding_buffer : public io_base
{
public:
    /*
     * Creates a new expanding buffer with the specified initial capacity.
     * The buffer will automatically grow using realloc as needed.
     */
    explicit io_expanding_buffer(std::size_t initial_capacity);

    /*
     * Destroys the expanding buffer I/O stream.
     */
    ~io_expanding_buffer() override;

    /*
     * Returns the current size of data written to the buffer.
     * This is different from the buffer capacity.
     */
    std::size_t size() const;

    /*
     * Finds and returns the first codec info object that supports the magic number read
     * from the memory buffer. The comparison algorithm is case insensitive. After reading
     * the magic number, rewinds the I/O cursor position back to the previous position.
     *
     * Not all codecs support magic numbers. That's why it's not guaranteed that this method
     * returns a valid codec info object.
     *
     * Returns an invalid codec info object on error.
     */
    sail::codec_info codec_info() override;
};

} // namespace sail
