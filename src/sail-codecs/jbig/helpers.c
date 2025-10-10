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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jbig.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

bool jbig_private_is_jbig(const void* data, size_t size)
{

    if (data == NULL || size < JBIG_MAGIC_SIZE)
    {
        return false;
    }

    const unsigned char* bytes = data;

    /* Check for JBIG BIH (BIE Header) marker.
     * JBIG files start with a specific header structure.
     * Byte 0: DL (lowest resolution layer)
     * Byte 1: D (highest resolution layer)
     * Byte 2: P (number of bit planes, 1-255)
     * Byte 3: reserved
     * Bytes 4-7: XD - Image width (big-endian)
     * Bytes 8-11: YD - Image height (big-endian)
     * Bytes 12-15: L0 (number of lines per stripe at lowest resolution)
     *
     * We check that DL <= D and some basic sanity checks.
     */

    /* Check if DL <= D (bytes 0 and 1). */
    if (bytes[0] > bytes[1])
    {
        return false;
    }

    /* Check if the number of planes is reasonable (byte 2, should be 1-255). */
    if (bytes[2] == 0)
    {
        return false;
    }

    return true;
}

sail_status_t jbig_private_read_header(struct sail_io* io, unsigned long* width, unsigned long* height, int* planes)
{

    unsigned char header[JBIG_MAGIC_SIZE];
    size_t bytes_read;

    SAIL_TRY(io->tolerant_read(io->stream, header, sizeof(header), &bytes_read));

    if (bytes_read != sizeof(header))
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if (!jbig_private_is_jbig(header, sizeof(header)))
    {
        SAIL_LOG_ERROR("JBIG: Invalid header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Parse width, height and planes from BIH header (big-endian). */
    *planes = header[2];

    uint32_t width32, height32;
    memcpy(&width32, header + 4, sizeof(uint32_t));
    memcpy(&height32, header + 8, sizeof(uint32_t));
    *width  = sail_reverse_uint32(width32);
    *height = sail_reverse_uint32(height32);

    if (*width == 0 || *height == 0)
    {
        SAIL_LOG_ERROR("JBIG: Invalid image dimensions %lux%lu", *width, *height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Seek back to start for actual decoding. */
    SAIL_TRY(io->seek(io->stream, -(long)JBIG_MAGIC_SIZE, SEEK_CUR));

    return SAIL_OK;
}
