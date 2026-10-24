/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/* 256-color palette signature. */
static const uint8_t SAIL_PCX_PALETTE_SIGNATURE = 0x0C;

static const unsigned char SAIL_PCX_MONO_PALETTE[] = {0, 0, 0, 255, 255, 255};

sail_status_t pcx_private_read_header(struct sail_io* io, struct SailPcxHeader* header)
{
    SAIL_TRY(io->strict_read(io->stream, &header->id, sizeof(header->id)));
    SAIL_TRY(io->strict_read(io->stream, &header->version, sizeof(header->version)));
    SAIL_TRY(io->strict_read(io->stream, &header->encoding, sizeof(header->encoding)));
    SAIL_TRY(io->strict_read(io->stream, &header->bits_per_plane, sizeof(header->bits_per_plane)));
    SAIL_TRY(io->strict_read(io->stream, &header->xmin, sizeof(header->xmin)));
    SAIL_TRY(io->strict_read(io->stream, &header->ymin, sizeof(header->ymin)));
    SAIL_TRY(io->strict_read(io->stream, &header->xmax, sizeof(header->xmax)));
    SAIL_TRY(io->strict_read(io->stream, &header->ymax, sizeof(header->ymax)));
    SAIL_TRY(io->strict_read(io->stream, &header->hdpi, sizeof(header->hdpi)));
    SAIL_TRY(io->strict_read(io->stream, &header->vdpi, sizeof(header->vdpi)));
    SAIL_TRY(io->strict_read(io->stream, header->palette, sizeof(header->palette)));
    SAIL_TRY(io->strict_read(io->stream, &header->reserved, sizeof(header->reserved)));
    SAIL_TRY(io->strict_read(io->stream, &header->planes, sizeof(header->planes)));
    SAIL_TRY(io->strict_read(io->stream, &header->bytes_per_line, sizeof(header->bytes_per_line)));
    SAIL_TRY(io->strict_read(io->stream, &header->palette_info, sizeof(header->palette_info)));
    SAIL_TRY(io->strict_read(io->stream, &header->hscreen_size, sizeof(header->hscreen_size)));
    SAIL_TRY(io->strict_read(io->stream, &header->vscreen_size, sizeof(header->vscreen_size)));
    SAIL_TRY(io->strict_read(io->stream, header->filler, sizeof(header->filler)));

    return SAIL_OK;
}

sail_status_t pcx_private_sail_pixel_format(unsigned bits_per_plane, unsigned planes, enum SailPixelFormat* result)
{
    switch (planes)
    {
    case 1:
    {
        switch (bits_per_plane)
        {
        case 1: *result = SAIL_PIXEL_FORMAT_BPP1_INDEXED; return SAIL_OK;
        case 4: *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
        case 8: *result = SAIL_PIXEL_FORMAT_BPP8_INDEXED; return SAIL_OK;
        }
        break;
    }
    case 3:
    {
        switch (bits_per_plane)
        {
        case 8: *result = SAIL_PIXEL_FORMAT_BPP24_RGB; return SAIL_OK;
        }
        break;
    }
    case 4:
    {
        switch (bits_per_plane)
        {
        case 1: *result = SAIL_PIXEL_FORMAT_BPP4_INDEXED; return SAIL_OK;
        case 8: *result = SAIL_PIXEL_FORMAT_BPP32_RGBA; return SAIL_OK;
        }
        break;
    }
    }

    SAIL_LOG_ERROR("PCX: Unsuppored combination of bits per plane(%u) and planes(%u)", bits_per_plane, planes);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

sail_status_t pcx_private_build_palette(enum SailPixelFormat pixel_format,
                                        struct sail_io* io,
                                        uint8_t palette16[48],
                                        struct sail_palette** palette)
{
    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
    {
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 2, palette));
        memcpy((*palette)->data, SAIL_PCX_MONO_PALETTE, 6);
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
    {
        const int palette_colors = 16;     /* 256 RGB entries. */
        const int palette_size   = 16 * 3; /* 256 RGB entries. */

        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, palette_colors, palette));
        memcpy((*palette)->data, palette16, palette_size);
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
    {
        const int palette_colors = 256;     /* 256 RGB entries. */
        const int palette_size   = 256 * 3; /* 256 RGB entries. */

        struct sail_palette* palette_local;
        SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, palette_colors, &palette_local));

        size_t saved_offset;
        SAIL_TRY_OR_CLEANUP(io->tell(io->stream, &saved_offset), /* cleanup */ sail_destroy_palette(palette_local));

        /*
         * The 256-color palette is stored at the end of the file after a signature byte.
         * Files without it are not broken: they are just older than the palette extension.
         */
        bool have_palette = false;
        size_t stream_size;

        if (io->size(io->stream, &stream_size) == SAIL_OK && stream_size > (size_t)palette_size
            && io->seek(io->stream, (long)-(palette_size + 1), SEEK_END) == SAIL_OK)
        {
            uint8_t signature;

            if (io->strict_read(io->stream, &signature, sizeof(signature)) == SAIL_OK
                && signature == SAIL_PCX_PALETTE_SIGNATURE
                && io->strict_read(io->stream, palette_local->data, palette_size) == SAIL_OK)
            {
                have_palette = true;
            }
        }

        SAIL_TRY_OR_CLEANUP(io->seek(io->stream, (long)saved_offset, SEEK_SET),
                            /* cleanup */ sail_destroy_palette(palette_local));

        if (!have_palette)
        {
            /* Fall back to the 16 colors from the header, the rest of the palette is black. */
            SAIL_LOG_WARNING("PCX: No 256-color palette found, using the 16 colors from the header");
            memcpy(palette_local->data, palette16, 16 * 3);
            memset((unsigned char*)palette_local->data + 16 * 3, 0, palette_size - 16 * 3);
        }

        *palette = palette_local;

        break;
    }
    default:
    {
        break;
    }
    }

    return SAIL_OK;
}

void pcx_private_merge_planes(const struct SailPcxHeader* header,
                              unsigned width,
                              const unsigned char* buffer,
                              unsigned char* target_scan)
{
    if (header->bits_per_plane == 1 && header->planes > 1)
    {
        /*
         * Every pixel takes a single bit from every plane. Bit 0 of the pixel comes from
         * the first plane, bit 1 from the second one and so on.
         */
        const unsigned columns = (width < header->bytes_per_line * 8) ? width : header->bytes_per_line * 8u;

        memset(target_scan, 0, (width * header->planes + 7) / 8);

        for (unsigned column = 0; column < columns; column++)
        {
            unsigned value = 0;

            for (unsigned plane = 0; plane < header->planes; plane++)
            {
                const unsigned char plane_byte = buffer[plane * header->bytes_per_line + column / 8];
                value |= ((plane_byte >> (7 - column % 8)) & 1u) << plane;
            }

            /* Two 4-bit pixels per byte, the leftmost pixel occupies the high bits. */
            if (column % 2 == 0)
            {
                target_scan[column / 2] = (unsigned char)(value << 4);
            }
            else
            {
                target_scan[column / 2] |= (unsigned char)value;
            }
        }
    }
    else
    {
        /* Every pixel takes a whole byte from every plane. */
        for (unsigned plane = 0; plane < header->planes; plane++)
        {
            for (unsigned column = 0; column < header->bytes_per_line; column++)
            {
                target_scan[column * header->planes + plane] = buffer[plane * header->bytes_per_line + column];
            }
        }
    }
}

void pcx_private_split_planes(const struct SailPcxHeader* header,
                              const struct sail_image* image,
                              const unsigned char* scan,
                              unsigned char* buffer)
{
    if (header->bits_per_plane == 1 && header->planes > 1)
    {
        /* Split every 4-bit pixel into four 1-bit planes. */
        memset(buffer, 0, (size_t)header->bytes_per_line * header->planes);

        for (unsigned column = 0; column < image->width; column++)
        {
            if (column / 2 >= image->bytes_per_line || column / 8 >= header->bytes_per_line)
            {
                break;
            }

            const unsigned char pixel =
                (column % 2 == 0) ? (unsigned char)(scan[column / 2] >> 4) : (unsigned char)(scan[column / 2] & 0x0F);

            for (unsigned plane = 0; plane < header->planes; plane++)
            {
                if ((pixel >> plane) & 1u)
                {
                    buffer[plane * header->bytes_per_line + column / 8] |= (unsigned char)(0x80 >> (column % 8));
                }
            }
        }
    }
    else
    {
        for (unsigned plane = 0; plane < header->planes; plane++)
        {
            for (unsigned column = 0; column < header->bytes_per_line; column++)
            {
                const unsigned index = column * header->planes + plane;

                buffer[plane * header->bytes_per_line + column] = (index < image->bytes_per_line) ? scan[index] : 0;
            }
        }
    }
}

sail_status_t pcx_private_read_uncompressed(struct sail_io* io,
                                            const struct SailPcxHeader* header,
                                            unsigned char* buffer,
                                            struct sail_image* image)
{
    for (unsigned row = 0; row < image->height; row++)
    {
        SAIL_TRY(io->strict_read(io->stream, buffer, (size_t)header->bytes_per_line * header->planes));

        pcx_private_merge_planes(header, image->width, buffer, sail_scan_line(image, row));
    }

    return SAIL_OK;
}

sail_status_t pcx_private_write_header(struct sail_io* io, const struct SailPcxHeader* header)
{
    SAIL_TRY(io->strict_write(io->stream, &header->id, sizeof(header->id)));
    SAIL_TRY(io->strict_write(io->stream, &header->version, sizeof(header->version)));
    SAIL_TRY(io->strict_write(io->stream, &header->encoding, sizeof(header->encoding)));
    SAIL_TRY(io->strict_write(io->stream, &header->bits_per_plane, sizeof(header->bits_per_plane)));
    SAIL_TRY(io->strict_write(io->stream, &header->xmin, sizeof(header->xmin)));
    SAIL_TRY(io->strict_write(io->stream, &header->ymin, sizeof(header->ymin)));
    SAIL_TRY(io->strict_write(io->stream, &header->xmax, sizeof(header->xmax)));
    SAIL_TRY(io->strict_write(io->stream, &header->ymax, sizeof(header->ymax)));
    SAIL_TRY(io->strict_write(io->stream, &header->hdpi, sizeof(header->hdpi)));
    SAIL_TRY(io->strict_write(io->stream, &header->vdpi, sizeof(header->vdpi)));
    SAIL_TRY(io->strict_write(io->stream, header->palette, sizeof(header->palette)));
    SAIL_TRY(io->strict_write(io->stream, &header->reserved, sizeof(header->reserved)));
    SAIL_TRY(io->strict_write(io->stream, &header->planes, sizeof(header->planes)));
    SAIL_TRY(io->strict_write(io->stream, &header->bytes_per_line, sizeof(header->bytes_per_line)));
    SAIL_TRY(io->strict_write(io->stream, &header->palette_info, sizeof(header->palette_info)));
    SAIL_TRY(io->strict_write(io->stream, &header->hscreen_size, sizeof(header->hscreen_size)));
    SAIL_TRY(io->strict_write(io->stream, &header->vscreen_size, sizeof(header->vscreen_size)));
    SAIL_TRY(io->strict_write(io->stream, header->filler, sizeof(header->filler)));

    return SAIL_OK;
}

sail_status_t pcx_private_pixel_format_to_pcx_format(enum SailPixelFormat pixel_format,
                                                     uint8_t* bits_per_plane,
                                                     uint8_t* planes)
{
    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
    {
        *bits_per_plane = 1;
        *planes         = 1;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
    {
        *bits_per_plane = 1;
        *planes         = 4;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
    {
        *bits_per_plane = 8;
        *planes         = 1;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        *bits_per_plane = 8;
        *planes         = 3;
        return SAIL_OK;
    }
    /*
     * Four 8-bit planes are not a part of the format. Such files are read for compatibility
     * with the writers that produce them, but writing them would produce images that most
     * decoders reject.
     */
    default:
    {
        SAIL_LOG_ERROR("PCX: Pixel format %s is not supported for saving", sail_pixel_format_to_string(pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }
}

void pcx_private_palette_to_rgb(const struct sail_palette* palette, unsigned char* rgb_output, unsigned color_count)
{
    /* Use the actual number of colors in the palette, not the requested count. */
    unsigned actual_color_count = palette->color_count < color_count ? palette->color_count : color_count;

    if (palette->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
    {
        memcpy(rgb_output, palette->data, actual_color_count * 3);
        /* Fill remaining colors with black. */
        if (actual_color_count < color_count)
        {
            memset(rgb_output + actual_color_count * 3, 0, (color_count - actual_color_count) * 3);
        }
    }
    else
    {
        /* Convert RGBA to RGB. */
        const unsigned char* src = (const unsigned char*)palette->data;
        for (unsigned i = 0; i < actual_color_count; i++)
        {
            rgb_output[i * 3 + 0] = src[i * 4 + 0];
            rgb_output[i * 3 + 1] = src[i * 4 + 1];
            rgb_output[i * 3 + 2] = src[i * 4 + 2];
        }
        /* Fill remaining colors with black. */
        if (actual_color_count < color_count)
        {
            memset(rgb_output + actual_color_count * 3, 0, (color_count - actual_color_count) * 3);
        }
    }
}

sail_status_t pcx_private_write_palette(struct sail_io* io, const struct sail_palette* palette)
{
    /* Write 256-color palette signature. */
    uint8_t signature = SAIL_PCX_PALETTE_SIGNATURE;
    SAIL_TRY(io->strict_write(io->stream, &signature, sizeof(signature)));

    /* Write palette data (256 RGB entries). PCX always requires exactly 256 entries.
     * If palette has fewer colors, pad with zeros. */
    unsigned char rgb[256 * 3];
    memset(rgb, 0, sizeof(rgb));
    pcx_private_palette_to_rgb(palette, rgb, sizeof(rgb) / 3);
    SAIL_TRY(io->strict_write(io->stream, rgb, sizeof(rgb)));

    return SAIL_OK;
}
