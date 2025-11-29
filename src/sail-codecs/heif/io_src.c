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

#include <stdio.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "io_src.h"

int64_t heif_private_reader_get_position(void* user_data)
{
    struct sail_heif_reader_context* context = user_data;

    size_t position;
    sail_status_t status = context->io->tell(context->io->stream, &position);

    return (status == SAIL_OK) ? (int64_t)position : -1;
}

int heif_private_reader_read(void* data, size_t size, void* user_data)
{
    struct sail_heif_reader_context* context = user_data;

    size_t bytes_read;
    sail_status_t status = context->io->tolerant_read(context->io->stream, data, size, &bytes_read);

    if (status != SAIL_OK && status != SAIL_ERROR_EOF)
    {
        return -1;
    }

    /* If we read all requested bytes, success. */
    if (bytes_read == size)
    {
        return 0;
    }

    /* Check EOF. */
    bool is_eof;
    SAIL_TRY_OR_EXECUTE(context->io->eof(context->io->stream, &is_eof),
                        /* on error */ return -1);

    return (is_eof) ? 0 : -1;
}

int heif_private_reader_seek(int64_t position, void* user_data)
{
    struct sail_heif_reader_context* context = user_data;

    sail_status_t status = context->io->seek(context->io->stream, (long)position, SEEK_SET);

    return (status == SAIL_OK) ? 0 : -1;
}

enum heif_reader_grow_status heif_private_reader_wait_for_file_size(int64_t target_size, void* user_data)
{
    (void)target_size;
    (void)user_data;

    /* We don't support streaming, so file size is always available. */
    return heif_reader_grow_status_size_reached;
}
