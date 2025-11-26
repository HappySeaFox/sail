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

#include <cstddef> /* size_t */

#include <libraw/libraw_datastream.h>

#include <sail-common/sail-common.h>

namespace sail::raw
{

class SailRawDatastream : public LibRaw_abstract_datastream
{
    struct sail_io* mIo;
    size_t mPosition;
    size_t mSize;

public:
    explicit SailRawDatastream(struct sail_io* io);
    ~SailRawDatastream() override;

private:
    int valid() override;

    int read(void* buffer, size_t size, size_t nmemb) override;

    int seek(INT64 offset, int whence) override;

    INT64 tell() override;

    INT64 size() override;

    int get_char() override;

    char* gets(char* str, int size) override;

    int scanf_one(const char* fmt, void* val) override;

    int eof() override;
};

} // namespace sail::raw
