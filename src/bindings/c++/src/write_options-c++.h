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
 * A C++ interface to struct sail_write_options.
 */
class SAIL_EXPORT write_options
{
    friend class image_writer;

public:
    write_options();
    /*
     * Makes a deep copy of the specified write options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    write_options(const sail_write_options *wo);
    write_options(const write_options &wo);
    write_options& operator=(const write_options &wo);
    write_options(write_options &&wo);
    write_options& operator=(write_options &&wo);
    ~write_options();

    SailPixelFormat output_pixel_format() const;
    int io_options() const;
    SailCompression compression() const;
    double compression_level() const;

    write_options& with_output_pixel_format(SailPixelFormat output_pixel_format);
    write_options& with_io_options(int io_options);
    write_options& with_compression(SailCompression compression);
    write_options& with_compression_level(double compression_level);

private:
    sail_status_t to_sail_write_options(sail_write_options *write_options) const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
