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

#ifndef SAIL_WRITE_OPTIONS_CPP_H
#define SAIL_WRITE_OPTIONS_CPP_H

#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_write_options;

namespace sail
{

/*
 * Options to modify writing operations.
 */
class SAIL_EXPORT write_options
{
    friend class image_output;
    friend class write_features;

public:
    /*
     * Constructs empty write options.
     */
    write_options();

    /*
     * Copies the write options.
     */
    write_options(const write_options &wo);

    /*
     * Copies the write options.
     */
    write_options& operator=(const sail::write_options &write_options);

    /*
     * Moves the write options.
     */
    write_options(sail::write_options &&write_options) noexcept;

    /*
     * Moves the write options.
     */
    write_options& operator=(sail::write_options &&write_options) noexcept;

    /*
     * Destroys the write options.
     */
    ~write_options();

    /*
     * Returns the or-ed I/O manipulation options for writing operations. See SailIoOption.
     */
    int io_options() const;

    /*
     * Returns the compression type. For example: SAIL_COMPRESSION_RLE. See SailCompression.
     * Use write_features to determine what compression types or values are supported by a particular codec.
     *
     * If a codec supports more than two compression types, compression levels are ignored in this case.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. compression_level_min, compression_level_max,
     *        compression_level_default can be used to select a compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.). Compression levels
     *        are ignored.
     */
    SailCompression compression() const;

    /*
     * Returns the requested compression level. Must be in the range specified by compression_level_min()
     * and compression_level_max() in write_features. If compression_level() < compression_level_min() or
     * compression_level() > compression_level_max(), compression_level_default() will be used.
     */
    double compression_level() const;

    /*
     * Sets new or-ed I/O manipulation options for writing operations. See SailIoOption.
     */
    write_options& with_io_options(int io_options);

    /*
     * Sets a new compression type.
     */
    write_options& with_compression(SailCompression compression);

    /*
     * Sets a new compression level.
     */
    write_options& with_compression_level(double compression_level);

private:
    /*
     * Makes a deep copy of the specified write options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit write_options(const sail_write_options *wo);

    sail_status_t to_sail_write_options(sail_write_options *write_options) const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
