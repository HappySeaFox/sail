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

#include <stdio.h>

#include "sail-common.h"

#include "io.h"

avifResult avif_private_read_proc(struct avifIO *io, uint32_t read_flags, uint64_t offset, size_t size, avifROData *out) {

    if (read_flags != 0) {
        SAIL_LOG_ERROR("AVIF: Read flags must be #0, but got #%u", read_flags);
        return AVIF_RESULT_IO_ERROR;
    }

    SAIL_LOG_TRACE("AVIF: Read at offset %ld size %lu", (long)offset, (unsigned long)size);

    struct sail_avif_context *avif_context = (struct sail_avif_context *)io->data;
    SAIL_TRY_OR_EXECUTE(avif_context->io->seek(avif_context->io->stream, (long)offset, SEEK_SET),
                        /* on error */ return AVIF_RESULT_IO_ERROR);

    /* Realloc internal buffer if necessary. */
    if (size > avif_context->buffer_size) {
        SAIL_TRY_OR_EXECUTE(sail_realloc(size, &avif_context->buffer),
                            /* on error */ return AVIF_RESULT_IO_ERROR);
        avif_context->buffer_size = size;
    }

    size_t size_read;
    SAIL_TRY_OR_EXECUTE(avif_context->io->tolerant_read(avif_context->io->stream, avif_context->buffer, size, &size_read),
                        /* on error */ return AVIF_RESULT_IO_ERROR);
    out->data = avif_context->buffer;
    out->size = size_read;

    SAIL_LOG_TRACE("AVIF: Actually read: %lu", (unsigned long)size_read);

    return AVIF_RESULT_OK;
}
