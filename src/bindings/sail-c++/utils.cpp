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

#include <sail-c++/sail-c++.h>

namespace sail
{

void print_errno(const char* format)
{
    sail_print_errno(format);
}

std::uint64_t now()
{
    return sail_now();
}

bool path_exists(const std::string& path)
{
    return sail_path_exists(path.c_str());
}

bool is_dir(const std::string& path)
{
    return sail_is_dir(path.c_str());
}

bool is_file(const std::string& path)
{
    return sail_is_file(path.c_str());
}

std::string temp_file_path(const std::string& prefix)
{
    char* path_c      = nullptr;
    auto* real_prefix = prefix.empty() ? "sail-tmp" : prefix.c_str();

    SAIL_TRY_OR_EXECUTE(sail_temp_file_path(real_prefix, &path_c),
                        /* on error */ throw std::runtime_error("Failed to create temporary file"));

    std::string path{path_c};
    sail_free(path_c);

    return path;
}

sail_status_t file_size(const std::string& path, size_t* size)
{

    SAIL_TRY(sail_file_size(path.c_str(), size));

    return SAIL_OK;
}

sail_status_t read_file_contents(const std::string& path, sail::arbitrary_data* contents)
{

    SAIL_CHECK_PTR(contents);

    size_t size;
    SAIL_TRY(file_size(path, &size));

    contents->resize(size);
    SAIL_TRY(sail_file_contents_into_data(path.c_str(), contents->data()));

    return SAIL_OK;
}

sail_status_t read_io_contents(sail::abstract_io& abstract_io, sail::arbitrary_data* contents)
{

    SAIL_CHECK_PTR(contents);

    sail::abstract_io_adapter abstract_io_adapter(abstract_io);

    /* Resize the buffer to the I/O size. */
    size_t data_size_local;
    SAIL_TRY(sail_io_size(&abstract_io_adapter.sail_io_c(), &data_size_local));

    contents->resize(data_size_local);

    /* Read the contents. */
    SAIL_TRY(sail_io_contents_into_data(&abstract_io_adapter.sail_io_c(), contents->data()));

    return SAIL_OK;
}

template <> SAIL_EXPORT std::uint16_t reverse_bytes<>(std::uint16_t v)
{
    return sail_reverse_uint16(v);
}

template <> SAIL_EXPORT std::uint32_t reverse_bytes<>(std::uint32_t v)
{
    return sail_reverse_uint32(v);
}

template <> SAIL_EXPORT std::uint64_t reverse_bytes<>(std::uint64_t v)
{
    return sail_reverse_uint64(v);
}

} // namespace sail
