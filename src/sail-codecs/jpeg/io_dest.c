/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include <jerror.h>

#include "sail-common.h"

#include "io_dest.h"

/*
 * Most of this file was copied from libjpeg-turbo 2.0.4 and adapted to SAIL.
 */

#define OUTPUT_BUF_SIZE  4096   /* choose an efficiently fwrite'able size */

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */
static void init_destination(j_compress_ptr cinfo)
{
    struct sail_jpeg_destination_mgr *dest = (struct sail_jpeg_destination_mgr *)cinfo->dest;

    /* Allocate the output buffer --- it will be released when done with image */
    dest->buffer = (JOCTET *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,
                                                        JPOOL_IMAGE,
                                                        OUTPUT_BUF_SIZE * sizeof(JOCTET));

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer   = OUTPUT_BUF_SIZE;
}

/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */
static boolean empty_output_buffer(j_compress_ptr cinfo)
{
    struct sail_jpeg_destination_mgr *dest = (struct sail_jpeg_destination_mgr *)cinfo->dest;

    sail_status_t err = dest->io->strict_write(dest->io->stream, dest->buffer, OUTPUT_BUF_SIZE);

    if (err != SAIL_OK)
        ERREXIT(cinfo, JERR_FILE_WRITE);

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer   = OUTPUT_BUF_SIZE;

    return TRUE;
}

/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
static void term_destination(j_compress_ptr cinfo)
{
    struct sail_jpeg_destination_mgr *dest = (struct sail_jpeg_destination_mgr *)cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

    /* Write any data remaining in the buffer */
    if (datacount > 0) {
        sail_status_t err = dest->io->strict_write(dest->io->stream, dest->buffer, datacount);

        if (err != SAIL_OK)
            ERREXIT(cinfo, JERR_FILE_WRITE);
    }

    sail_status_t err = dest->io->flush(dest->io->stream);

    /* Make sure we wrote the output file OK */
    if (err != SAIL_OK)
        ERREXIT(cinfo, JERR_FILE_WRITE);
}

/*
 * Prepare for output to a SAIL I/O stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */
void jpeg_private_sail_io_dest(j_compress_ptr cinfo, struct sail_io *io)
{
    struct sail_jpeg_destination_mgr *dest;

    /* The destination object is made permanent so that multiple JPEG images
     * can be written to the same file without re-executing jpeg_stdio_dest.
     */
    if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
        cinfo->dest = (struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,
                                                                                JPOOL_PERMANENT,
                                                                                sizeof(struct sail_jpeg_destination_mgr));
    } else if (cinfo->dest->init_destination != init_destination) {
        /* It is unsafe to reuse the existing destination manager unless it was
         * created by this function.  Otherwise, there is no guarantee that the
         * opaque structure is the right size.  Note that we could just create a
         * new structure, but the old structure would not be freed until
         * jpeg_destroy_compress() was called.
        */
        ERREXIT(cinfo, JERR_BUFFER_SIZE);
    }

    dest = (struct sail_jpeg_destination_mgr *)cinfo->dest;

    dest->pub.init_destination    = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination    = term_destination;
    dest->io                      = io;
}
