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

#ifndef SAIL_IO_BASE_CPP_H
#define SAIL_IO_BASE_CPP_H

#include <memory>

#ifdef SAIL_BUILD
    #include "abstract_io-c++.h"
#else
    #include <sail-c++/abstract_io-c++.h>
#endif

namespace sail
{

/*
 * Base I/O stream.
 */
class SAIL_EXPORT io_base : public abstract_io
{
public:
    /*
     * Operations on I/O streams.
     */
    enum class Operation
    {
        /*
         * Reading only.
         */
        Read,

        /*
         * Reading and writing.
         */
        ReadWrite,
    };

    /*
     * Construct a new base I/O stream.
     */
    explicit io_base(struct sail_io *sail_io);

    /*
     * Returns the I/O stream id.
     *
     * The same I/O classes (file, memory, etc.) share the same ids. This way
     * a client can known the exact type of the I/O stream. For example, a client can distinguish between
     * file and memory I/O streams.
     *
     * You MUST use your own unique id for custom I/O classes. For example, you can use sail_hash()
     * to generate and return a unique id.
     *
     * Well-known I/O ids for file and memory I/O classes: SAIL_FILE_IO_ID and SAIL_MEMORY_IO_ID.
     */
    std::uint64_t id() const override;

    /*
     * Returns the I/O stream features. See SailIoFeature.
     */
    int features() const override;

    /*
     * Reads from the underlying I/O object into the specified buffer. In contrast to strict_read(),
     * doesn't fail when the actual number of bytes read is smaller than requested.
     * Assigns the number of bytes actually read to the 'read_size' argument.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t tolerant_read(void *buf, std::size_t size_to_read, std::size_t *read_size) override;

    /*
     * Reads from the underlying I/O object into the specified buffer. In contrast to tolerant_read(),
     * fails when the actual number of bytes read is smaller than requested.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t strict_read(void *buf, std::size_t size_to_read) override;

    /*
     * Writes the specified buffer to the underlying I/O object. In contrast to strict_write(),
     * doesn't fail when the actual number of bytes written is smaller than requested.
     * Assigns the number of bytes actually written to the 'written_size' argument.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t tolerant_write(const void *buf, std::size_t size_to_write, std::size_t *written_size) override;

    /*
     * Writes the specified buffer to the underlying I/O object. In contrast to tolerant_write(),
     * fails when the actual number of bytes written is smaller than requested.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t strict_write(const void *buf, std::size_t size_to_write) override;

    /*
     * Sets the I/O position in the underlying I/O object.
     *
     * Possible 'whence' values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <cstdio>.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t seek(long offset, int whence) override;

    /*
     * Assigns the current I/O position in the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t tell(std::size_t *offset) override;

    /*
     * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
     * is opened for reading.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t flush() override;

    /*
     * Closes the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t close() override;

    /*
     * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t eof(bool *result) override;

protected:
    class pimpl;
    const std::unique_ptr<pimpl> d;
};

}

#endif
