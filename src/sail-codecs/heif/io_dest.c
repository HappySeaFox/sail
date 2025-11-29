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
#include <stdio.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "io_dest.h"

struct heif_error heif_private_writer_write(struct heif_context* ctx, const void* data, size_t size, void* user_data)
{
    (void)ctx;

    struct sail_heif_writer_context* context = user_data;

    sail_status_t status = context->io->strict_write(context->io->stream, data, size);

    struct heif_error error;

    if (status == SAIL_OK)
    {
        error.code    = heif_error_Ok;
        error.subcode = heif_suberror_Unspecified;
        error.message = "Success";
    }
    else
    {
        error.code    = heif_error_Encoding_error;
        error.subcode = heif_suberror_Cannot_write_output_data;
        error.message = "Failed to write data";
    }

    return error;
}
