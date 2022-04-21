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

#ifndef SAIL_IO_FILE_CPP_H
#define SAIL_IO_FILE_CPP_H

#include <memory>
#include <string>

#ifdef SAIL_BUILD
    #include "io_base-c++.h"
#else
    #include <sail-c++/io_base-c++.h>
#endif

namespace sail
{

/*
 * File I/O stream.
 */
class SAIL_EXPORT io_file : public io_base
{
public:
    /*
     * Opens the specified file for reading.
     */
    explicit io_file(const std::string &path);

    /*
     * Opens the specified memory buffer for the specified I/O operations.
     */
    io_file(const std::string &path, Operation operation);

    /*
     * Destroys the file I/O stream.
     */
    ~io_file() override;

    /*
     * Finds and returns a first codec info object that supports the file extension of the path.
     * The comparison algorithm is case insensitive.
     *
     * Returns an invalid codec info object on error.
     */
    sail::codec_info codec_info() override;

private:
    class io_file_pimpl;
    const std::unique_ptr<io_file_pimpl> file_d;
};

}

#endif
