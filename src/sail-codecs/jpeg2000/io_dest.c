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

#include <sail-common/sail-common.h>

#include "io_dest.h"

static OPJ_SIZE_T jpeg2000_private_stream_write(void* buffer, OPJ_SIZE_T bytes, void* user_data)
{

    struct sail_io* io = user_data;

    const sail_status_t err = io->strict_write(io->stream, buffer, bytes);

    if (err != SAIL_OK)
    {
        return (OPJ_SIZE_T)-1;
    }

    return bytes;
}

static OPJ_OFF_T jpeg2000_private_stream_skip_write(OPJ_OFF_T bytes, void* user_data)
{

    struct sail_io* io = user_data;

    const sail_status_t err = io->seek(io->stream, (long)bytes, SEEK_CUR);

    if (err != SAIL_OK)
    {
        return -1;
    }

    return bytes;
}

static OPJ_BOOL jpeg2000_private_stream_seek_write(OPJ_OFF_T bytes, void* user_data)
{

    struct sail_io* io = user_data;

    const sail_status_t err = io->seek(io->stream, (long)bytes, SEEK_SET);

    if (err != SAIL_OK)
    {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

opj_stream_t* jpeg2000_private_sail_io_dest(struct sail_io* io)
{

    opj_stream_t* stream = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_FALSE);

    if (stream == NULL)
    {
        return NULL;
    }

    opj_stream_set_user_data(stream, io, NULL);
    opj_stream_set_write_function(stream, jpeg2000_private_stream_write);
    opj_stream_set_skip_function(stream, jpeg2000_private_stream_skip_write);
    opj_stream_set_seek_function(stream, jpeg2000_private_stream_seek_write);

    return stream;
}
