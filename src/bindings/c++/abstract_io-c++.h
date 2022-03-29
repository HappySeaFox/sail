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

#ifndef SAIL_ABSTRACT_IO_CPP_H
#define SAIL_ABSTRACT_IO_CPP_H

#include <cstddef>
#include <cstdint>
#include <cstdio> /* seek whence */

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "codec_info-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/codec_info-c++.h>
#endif

namespace sail
{

/*
 * Abstract I/O stream represents an input/output abstraction.
 */
class SAIL_EXPORT abstract_io
{
public:
    /*
     * Destroys the I/O stream.
     */
    virtual ~abstract_io() = default;

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
    virtual std::uint64_t id() const = 0;

    /*
     * Returns the I/O stream features. See SailIoFeature.
     */
    virtual int features() const = 0;

    /*
     * Reads from the underlying I/O object into the specified buffer. In contrast to strict_read(),
     * doesn't fail when the actual number of bytes read is smaller than requested.
     * Assigns the number of bytes actually read to the 'read_size' argument.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t tolerant_read(void *buf, std::size_t size_to_read, std::size_t *read_size) = 0;

    /*
     * Reads from the underlying I/O object into the specified buffer. In contrast to tolerant_read(),
     * fails when the actual number of bytes read is smaller than requested.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t strict_read(void *buf, std::size_t size_to_read) = 0;

    /*
     * Writes the specified buffer to the underlying I/O object. In contrast to strict_write(),
     * doesn't fail when the actual number of bytes written is smaller than requested.
     * Assigns the number of bytes actually written to the 'written_size' argument.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t tolerant_write(const void *buf, std::size_t size_to_write, std::size_t *written_size) = 0;

    /*
     * Writes the specified buffer to the underlying I/O object. In contrast to tolerant_write(),
     * fails when the actual number of bytes written is smaller than requested.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t strict_write(const void *buf, std::size_t size_to_write) = 0;

    /*
     * Sets the I/O position in the underlying I/O object.
     *
     * Possible 'whence' values: SEEK_SET, SEEK_CUR, or SEEK_END declared in <cstdio>.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t seek(long offset, int whence) = 0;

    /*
     * Assigns the current I/O position in the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t tell(std::size_t *offset) = 0;

    /*
     * Flushes buffers of the underlying I/O object. Has no effect if the underlying I/O object
     * is opened for reading.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t flush() = 0;

    /*
     * Closes the underlying I/O object.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t close() = 0;

    /*
     * Assigns true to the specified result if the underlying I/O object reached the end-of-file indicator.
     *
     * Returns SAIL_OK on success.
     */
    virtual sail_status_t eof(bool *result) = 0;

    /*
     * Finds and returns a first codec info object that can theoretically read the underlying
     * I/O stream into a valid image.
     *
     * Returns an invalid codec info object if no suitable codecs was found.
     */
    virtual sail::codec_info codec_info() = 0;
};

}

#endif
