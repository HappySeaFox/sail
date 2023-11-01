/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#ifndef SAIL_UTILS_CPP_H
#define SAIL_UTILS_CPP_H

#include <cstdint>
#include <string>

#include <sail-common/export.h>
#include <sail-common/status.h>

#include <sail-c++/arbitrary_data.h>

namespace sail
{

class abstract_io;

/*
 * Prints the recent errno value with SAIL_LOG_ERROR(). The specified format must include '%s'.
 */
SAIL_EXPORT void print_errno(const char *format);

/*
 * Returns the current number of milliseconds since Epoch or 0 on error.
 */
SAIL_EXPORT std::uint64_t now();

/*
 * Returns true if the specified file system path exists.
 */
SAIL_EXPORT bool path_exists(const std::string &path);

/*
 * Returns true if the specified file system path is a directory.
 */
SAIL_EXPORT bool is_dir(const std::string &path);

/*
 * Returns true if the specified file system path is a regular file.
 */
SAIL_EXPORT bool is_file(const std::string &path);

/*
 * Retrieves the file size.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t file_size(const std::string &path, size_t *size);

/*
 * Reads the specified file into the memory buffer. The memory buffer is resized to fit the contents.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t read_file_contents(const std::string &path, sail::arbitrary_data *contents);

/*
 * Reads the specified I/O stream into the memory buffer. The memory buffer is resized to fit the contents.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t read_io_contents(sail::abstract_io &abstract_io, sail::arbitrary_data *contents);

/*
 * Reverses the input value byte order. Only std::uint16_t, std::uint32_t, std::uint64_t,
 * and their equivalent types are supported. Other types will fail to link.
 *
 * Returns the reversed value.
 */
template<typename T>
T reverse_bytes(T v);

}

#endif
