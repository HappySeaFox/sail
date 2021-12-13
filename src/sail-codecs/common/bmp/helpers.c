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

#include <stdio.h>

#include "sail-common.h"

#include "helpers.h"

sail_status_t bmp_private_read_ddb_file_header(struct sail_io *io, struct SailBmpDdbFileHeader *ddb_file_header) {

    SAIL_TRY(io->strict_read(io->stream, &ddb_file_header->type, sizeof(ddb_file_header->type)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_v1(struct sail_io *io, struct SailBmpDdbBitmap *v1) {

    SAIL_TRY(io->strict_read(io->stream, &v1->type,       sizeof(v1->type)));
    SAIL_TRY(io->strict_read(io->stream, &v1->width,      sizeof(v1->width)));
    SAIL_TRY(io->strict_read(io->stream, &v1->height,     sizeof(v1->height)));
    SAIL_TRY(io->strict_read(io->stream, &v1->byte_width, sizeof(v1->byte_width)));
    SAIL_TRY(io->strict_read(io->stream, &v1->planes,     sizeof(v1->planes)));
    SAIL_TRY(io->strict_read(io->stream, &v1->bit_count,  sizeof(v1->bit_count)));
    SAIL_TRY(io->strict_read(io->stream, &v1->pixels,     sizeof(v1->pixels)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_dib_file_header(struct sail_io *io, struct SailBmpDibFileHeader *fh) {

    SAIL_TRY(io->strict_read(io->stream, &fh->type,      sizeof(fh->type)));
    SAIL_TRY(io->strict_read(io->stream, &fh->size,      sizeof(fh->size)));
    SAIL_TRY(io->strict_read(io->stream, &fh->reserved1, sizeof(fh->reserved1)));
    SAIL_TRY(io->strict_read(io->stream, &fh->reserved2, sizeof(fh->reserved2)));
    SAIL_TRY(io->strict_read(io->stream, &fh->offset,    sizeof(fh->offset)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_v2(struct sail_io *io, struct SailBmpDibHeaderV2 *v2) {

    SAIL_TRY(io->strict_read(io->stream, &v2->size,      sizeof(v2->size)));
    SAIL_TRY(io->strict_read(io->stream, &v2->width,     sizeof(v2->width)));
    SAIL_TRY(io->strict_read(io->stream, &v2->height,    sizeof(v2->height)));
    SAIL_TRY(io->strict_read(io->stream, &v2->planes,    sizeof(v2->planes)));
    SAIL_TRY(io->strict_read(io->stream, &v2->bit_count, sizeof(v2->bit_count)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_v3(struct sail_io *io, struct SailBmpDibHeaderV3 *v3) {

    SAIL_TRY(io->strict_read(io->stream, &v3->compression,        sizeof(v3->compression)));
    SAIL_TRY(io->strict_read(io->stream, &v3->bitmap_size,        sizeof(v3->bitmap_size)));
    SAIL_TRY(io->strict_read(io->stream, &v3->x_pixels_per_meter, sizeof(v3->x_pixels_per_meter)));
    SAIL_TRY(io->strict_read(io->stream, &v3->y_pixels_per_meter, sizeof(v3->y_pixels_per_meter)));
    SAIL_TRY(io->strict_read(io->stream, &v3->colors_used,        sizeof(v3->colors_used)));
    SAIL_TRY(io->strict_read(io->stream, &v3->colors_important,   sizeof(v3->colors_important)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_v4(struct sail_io *io, struct SailBmpDibHeaderV4 *v4) {

    SAIL_TRY(io->strict_read(io->stream, &v4->red_mask,         sizeof(v4->red_mask)));
    SAIL_TRY(io->strict_read(io->stream, &v4->green_mask,       sizeof(v4->green_mask)));
    SAIL_TRY(io->strict_read(io->stream, &v4->blue_mask,        sizeof(v4->blue_mask)));
    SAIL_TRY(io->strict_read(io->stream, &v4->alpha_mask,       sizeof(v4->alpha_mask)));
    SAIL_TRY(io->strict_read(io->stream, &v4->color_space_type, sizeof(v4->color_space_type)));
    SAIL_TRY(io->strict_read(io->stream, &v4->red_x,            sizeof(v4->red_x)));
    SAIL_TRY(io->strict_read(io->stream, &v4->red_y,            sizeof(v4->red_y)));
    SAIL_TRY(io->strict_read(io->stream, &v4->red_z,            sizeof(v4->red_z)));
    SAIL_TRY(io->strict_read(io->stream, &v4->green_x,          sizeof(v4->green_x)));
    SAIL_TRY(io->strict_read(io->stream, &v4->green_y,          sizeof(v4->green_y)));
    SAIL_TRY(io->strict_read(io->stream, &v4->green_z,          sizeof(v4->green_z)));
    SAIL_TRY(io->strict_read(io->stream, &v4->blue_x,           sizeof(v4->blue_x)));
    SAIL_TRY(io->strict_read(io->stream, &v4->blue_y,           sizeof(v4->blue_y)));
    SAIL_TRY(io->strict_read(io->stream, &v4->blue_z,           sizeof(v4->blue_z)));
    SAIL_TRY(io->strict_read(io->stream, &v4->gamma_red,        sizeof(v4->gamma_red)));
    SAIL_TRY(io->strict_read(io->stream, &v4->gamma_green,      sizeof(v4->gamma_green)));
    SAIL_TRY(io->strict_read(io->stream, &v4->gamma_blue,       sizeof(v4->gamma_blue)));

    return SAIL_OK;
}

sail_status_t bmp_private_read_v5(struct sail_io *io, struct SailBmpDibHeaderV5 *v5) {

    SAIL_TRY(io->strict_read(io->stream, &v5->intent,       sizeof(v5->intent)));
    SAIL_TRY(io->strict_read(io->stream, &v5->profile_data, sizeof(v5->profile_data)));
    SAIL_TRY(io->strict_read(io->stream, &v5->profile_size, sizeof(v5->profile_size)));
    SAIL_TRY(io->strict_read(io->stream, &v5->reserved,     sizeof(v5->reserved)));

    return SAIL_OK;
}

sail_status_t bmp_private_bit_count_to_pixel_format(uint16_t bit_count, enum SailPixelFormat *pixel_format) {

    switch (bit_count) {
        case 1:  *pixel_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED; return SAIL_OK;
        case 4:  *pixel_format = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
        case 8:  *pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED; return SAIL_OK;
        case 16: *pixel_format = SAIL_PIXEL_FORMAT_BPP16_BGR555; return SAIL_OK;
        case 24: *pixel_format = SAIL_PIXEL_FORMAT_BPP24_BGR;    return SAIL_OK;
        case 32: *pixel_format = SAIL_PIXEL_FORMAT_BPP32_BGRA;   return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
}

sail_status_t bmp_private_fetch_iccp(struct sail_io *io, long offset_of_data, uint32_t profile_size, struct sail_iccp **iccp) {

    SAIL_TRY(io->seek(io->stream, offset_of_data, SEEK_SET));

    void *profile_data;
    SAIL_TRY(sail_malloc(profile_size, &profile_data));

    SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, profile_data, profile_size),
                        /* cleanup */ sail_free(profile_data));

    SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_move_data(profile_data, profile_size, iccp),
                        /* cleanup */ sail_free(profile_data));

    return SAIL_OK;
}

sail_status_t bmp_private_skip_end_of_scan_line(struct sail_io *io) {

    uint8_t marker;
    SAIL_TRY(io->strict_read(io->stream, &marker, sizeof(marker)));

    if (marker == SAIL_UNENCODED_RUN_MARKER) {
        SAIL_TRY(io->strict_read(io->stream, &marker, sizeof(marker)));

        if (marker != SAIL_END_OF_SCAN_LINE_MARKER) {
            SAIL_TRY(io->seek(io->stream, -2, SEEK_CUR));
        }
    } else {
        SAIL_TRY(io->seek(io->stream, -1, SEEK_CUR));
    }

    return SAIL_OK;
}

sail_status_t bmp_private_bytes_in_row(unsigned width, unsigned bit_count, unsigned *bytes_in_row) {

    switch (bit_count) {
        case 1:  *bytes_in_row = (width + 7) / 8; return SAIL_OK;
        case 4:  *bytes_in_row = (width + 1) / 2; return SAIL_OK;
        case 8:  *bytes_in_row = width;           return SAIL_OK;
        case 16: *bytes_in_row = width * 2;       return SAIL_OK;
        case 24: *bytes_in_row = width * 3;       return SAIL_OK;
        case 32: *bytes_in_row = width * 4;       return SAIL_OK;
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
}

unsigned bmp_private_pad_bytes(unsigned bytes_in_row) {

    const unsigned remainder = bytes_in_row % 4;
    return (remainder == 0) ? 0 : (4 - remainder);
}

sail_status_t bmp_private_fill_system_palette(unsigned bit_count, sail_rgb24_t **palette, unsigned *palette_count) {

    switch (bit_count) {
        case 1: {
            *palette_count = 2;

            void *ptr;
            SAIL_TRY(sail_malloc(sizeof(sail_rgb24_t) * (*palette_count), &ptr));
            *palette = ptr;

            (*palette)[0] = (sail_rgb24_t) { 0,   0,   0   };
            (*palette)[1] = (sail_rgb24_t) { 255, 255, 255 };

            return SAIL_OK;
        }
        case 4: {
            *palette_count = 16;

            void *ptr;
            SAIL_TRY(sail_malloc(sizeof(sail_rgb24_t) * (*palette_count), &ptr));
            *palette = ptr;

            (*palette)[0]  = (sail_rgb24_t) { 0,   0,   0   };
            (*palette)[1]  = (sail_rgb24_t) { 128, 0,   0   };
            (*palette)[2]  = (sail_rgb24_t) { 0,   128, 0   };
            (*palette)[3]  = (sail_rgb24_t) { 128, 128, 0   };
            (*palette)[4]  = (sail_rgb24_t) { 0,   0,   128 };
            (*palette)[5]  = (sail_rgb24_t) { 128, 0,   128 };
            (*palette)[6]  = (sail_rgb24_t) { 0,   128, 128 };
            (*palette)[7]  = (sail_rgb24_t) { 192, 192, 192 };
            (*palette)[8]  = (sail_rgb24_t) { 128, 128, 128 };
            (*palette)[9]  = (sail_rgb24_t) { 255, 0,   0   };
            (*palette)[10] = (sail_rgb24_t) { 0,   255, 0   };
            (*palette)[11] = (sail_rgb24_t) { 255, 255, 0   };
            (*palette)[12] = (sail_rgb24_t) { 0,   0,   255 };
            (*palette)[13] = (sail_rgb24_t) { 255, 0,   255 };
            (*palette)[14] = (sail_rgb24_t) { 0,   255, 255 };
            (*palette)[15] = (sail_rgb24_t) { 255, 255, 255 };

            return SAIL_OK;
        }
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
}
