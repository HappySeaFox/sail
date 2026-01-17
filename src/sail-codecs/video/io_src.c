/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libavformat/avio.h>
#include <libavutil/error.h>

#include <sail-common/sail-common.h>

#include "io_src.h"

int video_private_avio_read_packet(void* opaque, uint8_t* buf, int buf_size)
{
    if (opaque == NULL || buf == NULL || buf_size <= 0)
    {
        return AVERROR(EINVAL);
    }

    struct sail_io* io = (struct sail_io*)opaque;
    size_t read_size;

    sail_status_t err = io->tolerant_read(io->stream, buf, (size_t)buf_size, &read_size);

    if (err != SAIL_OK)
    {
        if (read_size == 0)
        {
            return AVERROR_EOF;
        }
        return AVERROR(EIO);
    }

    return (int)read_size;
}

int64_t video_private_avio_seek(void* opaque, int64_t offset, int whence)
{
    if (opaque == NULL)
    {
        return AVERROR(EINVAL);
    }

    struct sail_io* io = opaque;

    /* Handle special FFmpeg seek. */
    if (whence == AVSEEK_SIZE)
    {
        size_t size;
        sail_status_t err = io->size(io->stream, &size);

        if (err != SAIL_OK)
        {
            return AVERROR(EIO);
        }

        return (int64_t)size;
    }

    if ((io->features & SAIL_IO_FEATURE_SEEKABLE) == 0)
    {
        return AVERROR(ESPIPE);
    }
    if (offset > (int64_t)LONG_MAX || offset < (int64_t)LONG_MIN)
    {
        return AVERROR(EINVAL);
    }

    int base_whence = whence & ~(AVSEEK_SIZE | AVSEEK_FORCE);
    int std_whence;

    switch (base_whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
    case SEEK_END:
        std_whence = base_whence;
        break;
    default:
        SAIL_LOG_ERROR("VIDEO: Unsupported seek whence: #%d", base_whence);
        return AVERROR(EINVAL);
    }

    sail_status_t err = io->seek(io->stream, (long)offset, std_whence);
    if (err != SAIL_OK)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to seek with offset: %ld, whence: #%d", offset, std_whence);
        return AVERROR(EIO);
    }

    size_t position;
    err = io->tell(io->stream, &position);
    if (err != SAIL_OK)
    {
        SAIL_LOG_ERROR("VIDEO: Failed to get current position");
        return AVERROR(EIO);
    }

    return (int64_t)position;
}
