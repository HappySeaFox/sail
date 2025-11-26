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

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sail-common/io_common.h>

#include "datastream.h"

namespace sail::raw
{

SailRawDatastream::SailRawDatastream(struct sail_io* io)
    : mIo(io)
    , mPosition(0)
    , mSize(0)
{
    if (sail_io_size(mIo, &mSize) != SAIL_OK)
    {
        mSize = SIZE_MAX;
    }
}

SailRawDatastream::~SailRawDatastream() = default;

int SailRawDatastream::valid()
{
    return (mIo->stream != NULL) ? 1 : 0;
}

int SailRawDatastream::read(void* buffer, size_t size, size_t nmemb)
{
    if (buffer == NULL)
    {
        return -1;
    }

    size_t total_bytes = size * nmemb;
    size_t bytes_read;

    sail_status_t err = mIo->tolerant_read(mIo->stream, buffer, total_bytes, &bytes_read);

    if (err != SAIL_OK)
    {
        return -1;
    }

    mPosition += bytes_read;

    return static_cast<int>(bytes_read / size);
}

int SailRawDatastream::seek(INT64 offset, int whence)
{
    long seek_offset;

    switch (whence)
    {
    case SEEK_SET: seek_offset = static_cast<long>(offset); break;
    case SEEK_CUR: seek_offset = static_cast<long>(mPosition + offset); break;
    case SEEK_END:
        {
            if (mSize == SIZE_MAX)
            {
                return -1;
            }
            seek_offset = static_cast<long>(mSize + offset);
            break;
        }
    default: return -1;
    }

    sail_status_t err = mIo->seek(mIo->stream, seek_offset, SEEK_SET);

    if (err != SAIL_OK)
    {
        return -1;
    }

    mPosition = static_cast<size_t>(seek_offset);

    return 0;
}

INT64 SailRawDatastream::tell()
{
    size_t current_pos;
    sail_status_t err = mIo->tell(mIo->stream, &current_pos);

    if (err != SAIL_OK)
    {
        return -1;
    }

    mPosition = current_pos;

    return static_cast<INT64>(current_pos);
}

INT64 SailRawDatastream::size()
{
    if (mSize == SIZE_MAX)
    {
        return INT64_MAX;
    }

    return static_cast<INT64>(mSize);
}

int SailRawDatastream::get_char()
{
    unsigned char c;
    size_t bytes_read;

    sail_status_t err = mIo->tolerant_read(mIo->stream, &c, 1, &bytes_read);

    if (err != SAIL_OK || bytes_read == 0)
    {
        return -1;
    }

    mPosition++;

    return static_cast<int>(c);
}

char* SailRawDatastream::gets(char* str, int size)
{
    if (str == NULL || size <= 0)
    {
        return NULL;
    }

    for (int i = 0; i < size - 1; i++)
    {
        int c = get_char();

        if (c == -1)
        {
            if (i == 0)
            {
                return NULL;
            }
            break;
        }

        str[i] = static_cast<char>(c);

        if (c == '\n')
        {
            i++;
            break;
        }
    }

    str[size - 1] = '\0';

    return str;
}

int SailRawDatastream::scanf_one(const char* fmt, void* val)
{
    if (fmt == NULL || val == NULL)
    {
        return 0;
    }

    // Simple implementation - read one value based on format.
    if (strcmp(fmt, "%d") == 0)
    {
        int* int_val = static_cast<int*>(val);
        char buffer[32];
        int i = 0;

        while (i < 31)
        {
            int c = get_char();

            if (c == -1 || (c == ' ' || c == '\t' || c == '\n'))
            {
                break;
            }

            buffer[i++] = static_cast<char>(c);
        }

        buffer[i] = '\0';
        *int_val  = atoi(buffer);

        return 1;
    }

    return 0;
}

int SailRawDatastream::eof()
{
    bool is_eof;
    sail_status_t err = mIo->eof(mIo->stream, &is_eof);

    if (err != SAIL_OK)
    {
        return 1;
    }

    return is_eof ? 1 : 0;
}

} // namespace sail::raw
