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

#include "helpers.h"

sail_status_t tga_private_read_file_header(struct sail_io *io, struct TgaFileHeader *file_header) {

    SAIL_TRY(io->strict_read(io->stream, &file_header->id_length,                   sizeof(file_header->id_length)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_type,              sizeof(file_header->color_map_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->image_type,                  sizeof(file_header->image_type)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->first_color_map_entry_index, sizeof(file_header->first_color_map_entry_index)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_elements,          sizeof(file_header->color_map_elements)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->color_map_entry_size,        sizeof(file_header->color_map_entry_size)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->x,                           sizeof(file_header->x)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->y,                           sizeof(file_header->y)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->width,                       sizeof(file_header->width)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->height,                      sizeof(file_header->height)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->bpp,                         sizeof(file_header->bpp)));
    SAIL_TRY(io->strict_read(io->stream, &file_header->descriptor,                  sizeof(file_header->descriptor)));

    return SAIL_OK;
}

enum SailPixelFormat tga_private_sail_pixel_format(int image_type, int bpp) {

    switch (image_type) {
        case TGA_INDEXED:
        case TGA_INDEXED_RLE: {
            return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        }

        case TGA_TRUE_COLOR:
        case TGA_TRUE_COLOR_RLE: {
            switch (bpp) {
                case 16: return SAIL_PIXEL_FORMAT_BPP16_BGR555;
                case 24: return SAIL_PIXEL_FORMAT_BPP24_BGR;
                case 32: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        case TGA_MONO:
        case TGA_MONO_RLE: {
            switch (bpp) {
                case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

enum SailPixelFormat tga_private_palette_bpp_to_sail_pixel_format(int bpp) {

    switch (bpp) {
        case 15:
        case 16:
        case 24: return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case 32: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        default: {
            SAIL_LOG_ERROR("TGA: Palette bit depth %d is not supported", bpp);
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

sail_status_t tga_private_fetch_id(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_meta_data_node **meta_data_node) {

    struct sail_meta_data_node *meta_data_node_local;

    SAIL_TRY(sail_alloc_meta_data_node_and_value(&meta_data_node_local));

    struct sail_meta_data *meta_data = meta_data_node_local->meta_data;

    meta_data->key          = SAIL_META_DATA_ID;
    meta_data->value_type   = SAIL_META_DATA_TYPE_STRING;
    meta_data->value_length = file_header->id_length + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(meta_data->value_length, &meta_data->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, meta_data->value, meta_data->value_length - 1),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    ((char *)meta_data->value)[meta_data->value_length - 1] = '\0';

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t tga_private_fetch_palette(struct sail_io *io, const struct TgaFileHeader *file_header, struct sail_palette **palette) {

    size_t element_size_in_bytes = ((size_t)file_header->color_map_entry_size + 7) / 8;
    size_t bytes_to_skip = file_header->first_color_map_entry_index * element_size_in_bytes;

    if (bytes_to_skip > 0) {
        SAIL_TRY(io->seek(io->stream, (long)bytes_to_skip, SEEK_CUR));
    }

    enum SailPixelFormat palette_pixel_format = tga_private_palette_bpp_to_sail_pixel_format(file_header->color_map_entry_size);

    if (palette_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    unsigned palette_elements = (unsigned)file_header->color_map_elements - file_header->first_color_map_entry_index;
    struct sail_palette *palette_local;

    SAIL_TRY(sail_alloc_palette_for_data(palette_pixel_format, palette_elements, &palette_local));

    unsigned char *palette_data = palette_local->data;

    for (int i = file_header->first_color_map_entry_index; i < file_header->color_map_elements; i++) {
        unsigned char data[4];

        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, data, element_size_in_bytes),
                            /* cleanup */ sail_destroy_palette(palette_local));

        switch (file_header->color_map_entry_size) {
            case 15:
            case 16: {
                const uint16_t *data_as_word = (const uint16_t *)data;

                *palette_data++ = (unsigned char)(((*data_as_word & 0x1f)   <<  3) << 0);
                *palette_data++ = (unsigned char)(((*data_as_word & 0x3e0)  >>  5) << 3);
                *palette_data++ = (unsigned char)(((*data_as_word & 0x7c00) >> 10) << 3);
                break;
            }

            case 24: {
                *palette_data++ = data[2];
                *palette_data++ = data[1];
                *palette_data++ = data[0];
                break;
            }

            case 32: {
                *palette_data++ = data[2];
                *palette_data++ = data[1];
                *palette_data++ = data[0];
                *palette_data++ = data[3];
                break;
            }

            default: {
                SAIL_LOG_ERROR("TGA: Internal error: Unhandled palette pixel format");
                sail_destroy_palette(palette_local);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }
    }

    *palette = palette_local;

    return SAIL_OK;
}
