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

#include "sail-common.h"

#include "helpers.h"

/* Public domain Quake2 palette. See https://quakewiki.org/wiki/Quake_palette */
static const unsigned WAL_PALETTE_ELEMENTS = 256;

static const unsigned char WAL_PALETTE[256 * 3] =
{
	0, 0, 0,
	15, 15, 15,
	31, 31, 31,
	47, 47, 47,
	63, 63, 63,
	75, 75, 75,
	91, 91, 91,
	107, 107, 107,
	123, 123, 123,
	139, 139, 139,
	155, 155, 155,
	171, 171, 171,
	187, 187, 187,
	203, 203, 203,
	219, 219, 219,
	235, 235, 235,
	99, 75, 35,
	91, 67, 31,
	83, 63, 31,
	79, 59, 27,
	71, 55, 27,
	63, 47, 23,
	59, 43, 23,
	51, 39, 19,
	47, 35, 19,
	43, 31, 19,
	39, 27, 15,
	35, 23, 15,
	27, 19, 11,
	23, 15, 11,
	19, 15, 7,
	15, 11, 7,
	95, 95, 111,
	91, 91, 103,
	91, 83, 95,
	87, 79, 91,
	83, 75, 83,
	79, 71, 75,
	71, 63, 67,
	63, 59, 59,
	59, 55, 55,
	51, 47, 47,
	47, 43, 43,
	39, 39, 39,
	35, 35, 35,
	27, 27, 27,
	23, 23, 23,
	19, 19, 19,
	143, 119, 83,
	123, 99, 67,
	115, 91, 59,
	103, 79, 47,
	207, 151, 75,
	167, 123, 59,
	139, 103, 47,
	111, 83, 39,
	235, 159, 39,
	203, 139, 35,
	175, 119, 31,
	147, 99, 27,
	119, 79, 23,
	91, 59, 15,
	63, 39, 11,
	35, 23, 7,
	167, 59, 43,
	159, 47, 35,
	151, 43, 27,
	139, 39, 19,
	127, 31, 15,
	115, 23, 11,
	103, 23, 7,
	87, 19, 0,
	75, 15, 0,
	67, 15, 0,
	59, 15, 0,
	51, 11, 0,
	43, 11, 0,
	35, 11, 0,
	27, 7, 0,
	19, 7, 0,
	123, 95, 75,
	115, 87, 67,
	107, 83, 63,
	103, 79, 59,
	95, 71, 55,
	87, 67, 51,
	83, 63, 47,
	75, 55, 43,
	67, 51, 39,
	63, 47, 35,
	55, 39, 27,
	47, 35, 23,
	39, 27, 19,
	31, 23, 15,
	23, 15, 11,
	15, 11, 7,
	111, 59, 23,
	95, 55, 23,
	83, 47, 23,
	67, 43, 23,
	55, 35, 19,
	39, 27, 15,
	27, 19, 11,
	15, 11, 7,
	179, 91, 79,
	191, 123, 111,
	203, 155, 147,
	215, 187, 183,
	203, 215, 223,
	179, 199, 211,
	159, 183, 195,
	135, 167, 183,
	115, 151, 167,
	91, 135, 155,
	71, 119, 139,
	47, 103, 127,
	23, 83, 111,
	19, 75, 103,
	15, 67, 91,
	11, 63, 83,
	7, 55, 75,
	7, 47, 63,
	7, 39, 51,
	0, 31, 43,
	0, 23, 31,
	0, 15, 19,
	0, 7, 11,
	0, 0, 0,
	139, 87, 87,
	131, 79, 79,
	123, 71, 71,
	115, 67, 67,
	107, 59, 59,
	99, 51, 51,
	91, 47, 47,
	87, 43, 43,
	75, 35, 35,
	63, 31, 31,
	51, 27, 27,
	43, 19, 19,
	31, 15, 15,
	19, 11, 11,
	11, 7, 7,
	0, 0, 0,
	151, 159, 123,
	143, 151, 115,
	135, 139, 107,
	127, 131, 99,
	119, 123, 95,
	115, 115, 87,
	107, 107, 79,
	99, 99, 71,
	91, 91, 67,
	79, 79, 59,
	67, 67, 51,
	55, 55, 43,
	47, 47, 35,
	35, 35, 27,
	23, 23, 19,
	15, 15, 11,
	159, 75, 63,
	147, 67, 55,
	139, 59, 47,
	127, 55, 39,
	119, 47, 35,
	107, 43, 27,
	99, 35, 23,
	87, 31, 19,
	79, 27, 15,
	67, 23, 11,
	55, 19, 11,
	43, 15, 7,
	31, 11, 7,
	23, 7, 0,
	11, 0, 0,
	0, 0, 0,
	119, 123, 207,
	111, 115, 195,
	103, 107, 183,
	99, 99, 167,
	91, 91, 155,
	83, 87, 143,
	75, 79, 127,
	71, 71, 115,
	63, 63, 103,
	55, 55, 87,
	47, 47, 75,
	39, 39, 63,
	35, 31, 47,
	27, 23, 35,
	19, 15, 23,
	11, 7, 7,
	155, 171, 123,
	143, 159, 111,
	135, 151, 99,
	123, 139, 87,
	115, 131, 75,
	103, 119, 67,
	95, 111, 59,
	87, 103, 51,
	75, 91, 39,
	63, 79, 27,
	55, 67, 19,
	47, 59, 11,
	35, 47, 7,
	27, 35, 0,
	19, 23, 0,
	11, 15, 0,
	0, 255, 0,
	35, 231, 15,
	63, 211, 27,
	83, 187, 39,
	95, 167, 47,
	95, 143, 51,
	95, 123, 51,
	255, 255, 255,
	255, 255, 211,
	255, 255, 167,
	255, 255, 127,
	255, 255, 83,
	255, 255, 39,
	255, 235, 31,
	255, 215, 23,
	255, 191, 15,
	255, 171, 7,
	255, 147, 0,
	239, 127, 0,
	227, 107, 0,
	211, 87, 0,
	199, 71, 0,
	183, 59, 0,
	171, 43, 0,
	155, 31, 0,
	143, 23, 0,
	127, 15, 0,
	115, 7, 0,
	95, 0, 0,
	71, 0, 0,
	47, 0, 0,
	27, 0, 0,
	239, 0, 0,
	55, 55, 255,
	255, 0, 0,
	0, 0, 255,
	43, 43, 35,
	27, 27, 23,
	19, 19, 15,
	235, 151, 127,
	195, 115, 83,
	159, 87, 51,
	123, 63, 27,
	235, 211, 199,
	199, 171, 155,
	167, 139, 119,
	135, 107, 87,
	159, 91, 83
};

sail_status_t wal_private_read_file_header(struct sail_io *io, struct WalFileHeader *wal_header) {

    SAIL_TRY(io->strict_read(io->stream, &wal_header->name,      sizeof(wal_header->name)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->width,     sizeof(wal_header->width)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->height,    sizeof(wal_header->height)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->offset,    sizeof(wal_header->offset)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->next_name, sizeof(wal_header->next_name)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->flags,     sizeof(wal_header->flags)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->contents,  sizeof(wal_header->contents)));
    SAIL_TRY(io->strict_read(io->stream, &wal_header->value,     sizeof(wal_header->value)));

    return SAIL_OK;
}

sail_status_t wal_private_assign_palette(struct sail_image *image) {

    SAIL_TRY(sail_alloc_palette_from_data(SAIL_PIXEL_FORMAT_BPP24_RGB, WAL_PALETTE, WAL_PALETTE_ELEMENTS, &image->palette));

    return SAIL_OK;
}


sail_status_t wal_private_assign_meta_data(const struct WalFileHeader *wal_header, struct sail_meta_data_node **meta_data_node) {

    struct sail_meta_data_node *meta_data_node_local;
    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node_local));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_NAME, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node_local->meta_data->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node_local->meta_data->value, wal_header->name),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}
