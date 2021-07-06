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
        SAIL_LOG_ERROR("AVIF: Unsupported read flags #%u", read_flags);
        return AVIF_RESULT_IO_ERROR;
    }

#if 0
    // * If offset exceeds the size of the content (past EOF), return AVIF_RESULT_IO_ERROR.
    if (offset > reader->rodata.size) {
        return AVIF_RESULT_IO_ERROR;
    }

    // * If offset is *exactly* at EOF, provide any 0-byte buffer and return AVIF_RESULT_OK.
    if (offset == reader->rodata.size) {
        out->data = reader->rodata.data;
        out->size = 0;
        return AVIF_RESULT_OK;
    }

    // * If (offset+size) exceeds the contents' size, it must provide a truncated buffer that provides
    //   all bytes from the offset to EOF, and return AVIF_RESULT_OK.
    uint64_t availableSize = reader->rodata.size - offset;
    if (size > availableSize) {
        size = (size_t)availableSize;
    }

    // * If (offset+size) does not exceed the contents' size but the *entire range* is unavailable yet
    //   (due to network conditions or any other reason), return AVIF_RESULT_WAITING_ON_IO.
    if (offset > reader->downloadedBytes) {
        return AVIF_RESULT_WAITING_ON_IO;
    }
    if (size > (reader->downloadedBytes - offset)) {
        return AVIF_RESULT_WAITING_ON_IO;
    }

    // * If (offset+size) does not exceed the contents' size, it must provide the *entire range* and
    //   return AVIF_RESULT_OK.
    out->data = reader->rodata.data + offset;
    out->size = size;
#endif

    SAIL_LOG_TRACE("AVIF: Read at offset %ld size %lu", (long)offset, (unsigned long)size);

    struct sail_io *sail_io = (struct sail_io *)io->data;
    SAIL_TRY_OR_EXECUTE(sail_io->seek(sail_io->stream, (long)offset, SEEK_SET),
                        /* on error */ return AVIF_RESULT_IO_ERROR);

    // FIXME
    uint8_t *buf;
    SAIL_TRY_OR_EXECUTE(sail_malloc(size, &buf),
                        /* on error */ return AVIF_RESULT_IO_ERROR);

    size_t size_read;
    SAIL_TRY_OR_EXECUTE(sail_io->tolerant_read(sail_io->stream, buf, size, &size_read),
                        /* on error */ return AVIF_RESULT_IO_ERROR);
    out->data = buf;
    out->size = size_read;

    SAIL_LOG_TRACE("AVIF: Actually read: %lu", (unsigned long)size_read);

    return AVIF_RESULT_OK;
}
