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
 * IO represents an input/output abstraction.
 */
class SAIL_EXPORT io
{
    friend class codec_info;
    friend class image_input;
    friend class image_output;

public:
    /*
     * Constructs a new I/O stream.
     */
    io();

    /*
     * Copies the I/O stream.
     */
    io(const io &i);

    /*
     * Copies the I/O stream.
     */
    io& operator=(const io &i);

    /*
     * Destroys the I/O stream.
     */
    ~io();

    /*
     * Returns SAIL_OK if the I/O stream has valid callbacks and a non-zero id.
     */
    sail_status_t verify_valid() const;

    /*
     * Returns true if the I/O stream has valid callbacks and a non-zero id.
     */
    bool is_valid() const;

    /*
     * Returns the I/O stream id.
     *
     * The same I/O classes (file, memory, etc.) share the same ids. This way
     * a client can known the exact type of the I/O object. For example, a client can distinguish between
     * file and memory I/O streams.
     *
     * You MUST use your own unique id for custom I/O classes. For example, you can use sail_hash()
     * to generate a unique id and assign it with with_id().
     *
     * Well-known I/O ids for file and memory I/O classes: SAIL_FILE_IO_ID and SAIL_MEMORY_IO_ID.
     */
    uint64_t id() const;

    /*
     * Returns the I/O stream features. See SailIoFeature.
     */
    int features() const;

    /*
     * Sets a new I/O stream id.
     */
    io& with_id(uint64_t id);

    /*
     * Sets new I/O stream features. See SailIoFeature.
     */
    io& with_features(int features);

    /*
     * Sets a new I/O-specific data object. For example, a pointer to a FILE.
     */
    io& with_stream(void *stream);

    /*
     * Sets a new tolerant read callback.
     */
    io& with_tolerant_read(sail_io_tolerant_read_t read);

    /*
     * Sets a new strict read callback.
     */
    io& with_strict_read(sail_io_strict_read_t read);

    /*
     * Sets a new seek callback.
     */
    io& with_seek(sail_io_seek_t seek);

    /*
     * Sets a new tell callback.
     */
    io& with_tell(sail_io_tell_t tell);

    /*
     * Sets a new tolerant write callback.
     */
    io& with_tolerant_write(sail_io_tolerant_write_t write);

    /*
     * Sets a new strict write callback.
     */
    io& with_strict_write(sail_io_strict_write_t write);

    /*
     * Sets a new flush callback.
     */
    io& with_flush(sail_io_flush_t flush);

    /*
     * Sets a new close callback.
     */
    io& with_close(sail_io_close_t close);

    /*
     * Sets a new EOF callback.
     */
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
