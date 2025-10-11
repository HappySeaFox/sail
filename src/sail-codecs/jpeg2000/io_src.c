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

#include <stddef.h> /* size_t */
#include <stdio.h>  /* SEEK_CUR */

#include <sail-common/sail-common.h>

#include "io_src.h"

static OPJ_SIZE_T jpeg2000_private_stream_read(void* buffer, OPJ_SIZE_T bytes, void* user_data)
{
    struct sail_io* io = user_data;
    size_t bytes_read;

    const sail_status_t err = io->tolerant_read(io->stream, buffer, bytes, &bytes_read);

    if (err != SAIL_OK)
    {
        SAIL_LOG_ERROR("JPEG2000: Read failed for %zu bytes", (size_t)bytes);
        return (OPJ_SIZE_T)-1;
    }

    /* Return -1 to signal error, otherwise OpenJPEG loops infinitely. */
    if (bytes_read == 0 && bytes > 0)
    {
        return (OPJ_SIZE_T)-1;
    }

    return (OPJ_SIZE_T)bytes_read;
}

static OPJ_OFF_T jpeg2000_private_stream_skip(OPJ_OFF_T bytes, void* user_data)
{
    struct sail_io* io = user_data;

    const sail_status_t err = io->seek(io->stream, (long)bytes, SEEK_CUR);

    if (err != SAIL_OK)
    {
        return -1;
    }

    return bytes;
}

static OPJ_BOOL jpeg2000_private_stream_seek(OPJ_OFF_T bytes, void* user_data)
{
    struct sail_io* io = user_data;

    const sail_status_t err = io->seek(io->stream, (long)bytes, SEEK_SET);

    if (err != SAIL_OK)
    {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

opj_stream_t* jpeg2000_private_sail_io_src(struct sail_io* io)
{
    opj_stream_t* stream = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_TRUE);

    if (stream == NULL)
    {
        return NULL;
    }

    /* Get file size for OpenJPEG. */
    size_t file_size;
    if (io->seek(io->stream, 0, SEEK_END) != SAIL_OK || io->tell(io->stream, &file_size) != SAIL_OK
        || io->seek(io->stream, 0, SEEK_SET) != SAIL_OK)
    {
        opj_stream_destroy(stream);
        return NULL;
    }

    opj_stream_set_user_data(stream, io, NULL);
    opj_stream_set_user_data_length(stream, (OPJ_UINT64)file_size);
    opj_stream_set_read_function(stream, jpeg2000_private_stream_read);
    opj_stream_set_skip_function(stream, jpeg2000_private_stream_skip);
    opj_stream_set_seek_function(stream, jpeg2000_private_stream_seek);

    return stream;
}
