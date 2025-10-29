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
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

sail_status_t fli_private_read_header(struct sail_io* io, struct SailFliHeader* header)
{
    SAIL_TRY(io->strict_read(io->stream, &header->size, sizeof(header->size)));
    SAIL_TRY(io->strict_read(io->stream, &header->magic, sizeof(header->magic)));
    SAIL_TRY(io->strict_read(io->stream, &header->frames, sizeof(header->frames)));
    SAIL_TRY(io->strict_read(io->stream, &header->width, sizeof(header->width)));
    SAIL_TRY(io->strict_read(io->stream, &header->height, sizeof(header->height)));
    SAIL_TRY(io->strict_read(io->stream, &header->depth, sizeof(header->depth)));
    SAIL_TRY(io->strict_read(io->stream, &header->flags, sizeof(header->flags)));
    SAIL_TRY(io->strict_read(io->stream, &header->speed, sizeof(header->speed)));
    SAIL_TRY(io->strict_read(io->stream, &header->reserved1, sizeof(header->reserved1)));
    SAIL_TRY(io->strict_read(io->stream, &header->created, sizeof(header->created)));
    SAIL_TRY(io->strict_read(io->stream, &header->creator, sizeof(header->creator)));
    SAIL_TRY(io->strict_read(io->stream, &header->updated, sizeof(header->updated)));
    SAIL_TRY(io->strict_read(io->stream, &header->updater, sizeof(header->updater)));
    SAIL_TRY(io->strict_read(io->stream, &header->aspect_x, sizeof(header->aspect_x)));
    SAIL_TRY(io->strict_read(io->stream, &header->aspect_y, sizeof(header->aspect_y)));
    SAIL_TRY(io->strict_read(io->stream, header->reserved2, sizeof(header->reserved2)));
    SAIL_TRY(io->strict_read(io->stream, &header->oframe1, sizeof(header->oframe1)));
    SAIL_TRY(io->strict_read(io->stream, &header->oframe2, sizeof(header->oframe2)));
    SAIL_TRY(io->strict_read(io->stream, header->reserved3, sizeof(header->reserved3)));

    return SAIL_OK;
}

sail_status_t fli_private_write_header(struct sail_io* io, const struct SailFliHeader* header)
{
    SAIL_TRY(io->strict_write(io->stream, &header->size, sizeof(header->size)));
    SAIL_TRY(io->strict_write(io->stream, &header->magic, sizeof(header->magic)));
    SAIL_TRY(io->strict_write(io->stream, &header->frames, sizeof(header->frames)));
    SAIL_TRY(io->strict_write(io->stream, &header->width, sizeof(header->width)));
    SAIL_TRY(io->strict_write(io->stream, &header->height, sizeof(header->height)));
    SAIL_TRY(io->strict_write(io->stream, &header->depth, sizeof(header->depth)));
    SAIL_TRY(io->strict_write(io->stream, &header->flags, sizeof(header->flags)));
    SAIL_TRY(io->strict_write(io->stream, &header->speed, sizeof(header->speed)));
    SAIL_TRY(io->strict_write(io->stream, &header->reserved1, sizeof(header->reserved1)));
    SAIL_TRY(io->strict_write(io->stream, &header->created, sizeof(header->created)));
    SAIL_TRY(io->strict_write(io->stream, &header->creator, sizeof(header->creator)));
    SAIL_TRY(io->strict_write(io->stream, &header->updated, sizeof(header->updated)));
    SAIL_TRY(io->strict_write(io->stream, &header->updater, sizeof(header->updater)));
    SAIL_TRY(io->strict_write(io->stream, &header->aspect_x, sizeof(header->aspect_x)));
    SAIL_TRY(io->strict_write(io->stream, &header->aspect_y, sizeof(header->aspect_y)));
    SAIL_TRY(io->strict_write(io->stream, header->reserved2, sizeof(header->reserved2)));
    SAIL_TRY(io->strict_write(io->stream, &header->oframe1, sizeof(header->oframe1)));
    SAIL_TRY(io->strict_write(io->stream, &header->oframe2, sizeof(header->oframe2)));
    SAIL_TRY(io->strict_write(io->stream, header->reserved3, sizeof(header->reserved3)));

    return SAIL_OK;
}

sail_status_t fli_private_read_frame_header(struct sail_io* io, struct SailFliFrameHeader* frame_header)
{
    SAIL_TRY(io->strict_read(io->stream, &frame_header->size, sizeof(frame_header->size)));
    SAIL_TRY(io->strict_read(io->stream, &frame_header->magic, sizeof(frame_header->magic)));
    SAIL_TRY(io->strict_read(io->stream, &frame_header->chunks, sizeof(frame_header->chunks)));
    SAIL_TRY(io->strict_read(io->stream, &frame_header->delay, sizeof(frame_header->delay)));
    SAIL_TRY(io->strict_read(io->stream, frame_header->reserved, sizeof(frame_header->reserved)));

    return SAIL_OK;
}

sail_status_t fli_private_write_frame_header(struct sail_io* io, const struct SailFliFrameHeader* frame_header)
{
    SAIL_TRY(io->strict_write(io->stream, &frame_header->size, sizeof(frame_header->size)));
    SAIL_TRY(io->strict_write(io->stream, &frame_header->magic, sizeof(frame_header->magic)));
    SAIL_TRY(io->strict_write(io->stream, &frame_header->chunks, sizeof(frame_header->chunks)));
    SAIL_TRY(io->strict_write(io->stream, &frame_header->delay, sizeof(frame_header->delay)));
    SAIL_TRY(io->strict_write(io->stream, frame_header->reserved, sizeof(frame_header->reserved)));

    return SAIL_OK;
}

sail_status_t fli_private_read_chunk_header(struct sail_io* io, struct SailFliChunkHeader* chunk_header)
{
    SAIL_TRY(io->strict_read(io->stream, &chunk_header->size, sizeof(chunk_header->size)));
    SAIL_TRY(io->strict_read(io->stream, &chunk_header->type, sizeof(chunk_header->type)));

    return SAIL_OK;
}

sail_status_t fli_private_write_chunk_header(struct sail_io* io, const struct SailFliChunkHeader* chunk_header)
{
    SAIL_TRY(io->strict_write(io->stream, &chunk_header->size, sizeof(chunk_header->size)));
    SAIL_TRY(io->strict_write(io->stream, &chunk_header->type, sizeof(chunk_header->type)));

    return SAIL_OK;
}

sail_status_t fli_private_decode_color64(struct sail_io* io, uint32_t chunk_size, struct sail_palette* palette)
{
    (void)chunk_size;

    uint16_t packets;
    SAIL_TRY(io->strict_read(io->stream, &packets, sizeof(packets)));

    unsigned index = 0;

    for (uint16_t i = 0; i < packets; i++)
    {
        uint8_t skip, count;
        SAIL_TRY(io->strict_read(io->stream, &skip, sizeof(skip)));
        SAIL_TRY(io->strict_read(io->stream, &count, sizeof(count)));

        index += skip;

        /* Count of 0 means 256. */
        unsigned colors_to_read = (count == 0) ? 256 : count;

        for (unsigned j = 0; j < colors_to_read && index < 256; j++, index++)
        {
            uint8_t rgb[3];
            SAIL_TRY(io->strict_read(io->stream, rgb, 3));

            /* FLI COLOR64 uses 6-bit color values (0-63), scale to 8-bit (0-255). */
            unsigned char* pal_entry = (unsigned char*)palette->data + index * 3;
            pal_entry[0]             = (unsigned char)((rgb[0] << 2) | (rgb[0] >> 4));
            pal_entry[1]             = (unsigned char)((rgb[1] << 2) | (rgb[1] >> 4));
            pal_entry[2]             = (unsigned char)((rgb[2] << 2) | (rgb[2] >> 4));
        }
    }

    return SAIL_OK;
}

sail_status_t fli_private_decode_color256(struct sail_io* io, uint32_t chunk_size, struct sail_palette* palette)
{
    (void)chunk_size;

    uint16_t packets;
    SAIL_TRY(io->strict_read(io->stream, &packets, sizeof(packets)));

    unsigned index = 0;

    for (uint16_t i = 0; i < packets; i++)
    {
        uint8_t skip, count;
        SAIL_TRY(io->strict_read(io->stream, &skip, sizeof(skip)));
        SAIL_TRY(io->strict_read(io->stream, &count, sizeof(count)));

        index += skip;

        /* Count of 0 means 256. */
        unsigned colors_to_read = (count == 0) ? 256 : count;

        for (unsigned j = 0; j < colors_to_read && index < 256; j++, index++)
        {
            uint8_t rgb[3];
            SAIL_TRY(io->strict_read(io->stream, rgb, 3));

            /* FLI COLOR256 uses 8-bit color values. */
            unsigned char* pal_entry = (unsigned char*)palette->data + index * 3;
            pal_entry[0]             = rgb[0];
            pal_entry[1]             = rgb[1];
            pal_entry[2]             = rgb[2];
        }
    }

    return SAIL_OK;
}

sail_status_t fli_private_encode_color256(struct sail_io* io, const struct sail_palette* palette)
{
    /* Write single packet with all 256 colors. */
    uint16_t packets = 1;
    SAIL_TRY(io->strict_write(io->stream, &packets, sizeof(packets)));

    uint8_t skip  = 0;
    uint8_t count = 0; /* 0 means 256. */
    SAIL_TRY(io->strict_write(io->stream, &skip, sizeof(skip)));
    SAIL_TRY(io->strict_write(io->stream, &count, sizeof(count)));

    /* Write all 256 colors. */
    for (unsigned i = 0; i < 256; i++)
    {
        const unsigned char* pal_entry = (const unsigned char*)palette->data + i * 3;
        SAIL_TRY(io->strict_write(io->stream, pal_entry, 3));
    }

    return SAIL_OK;
}

sail_status_t fli_private_decode_brun(struct sail_io* io, unsigned char* pixels, unsigned width, unsigned height)
{
    /* BRUN format: each line starts with packet count byte, followed by packets. */
    for (unsigned y = 0; y < height; y++)
    {
        unsigned char* line = pixels + y * width;
        unsigned x          = 0;

        /* Read packet count for this line. */
        uint8_t packet_count;
        SAIL_TRY(io->strict_read(io->stream, &packet_count, sizeof(packet_count)));

        for (uint8_t pkt = 0; pkt < packet_count && x < width; pkt++)
        {
            int8_t packet_type;
            SAIL_TRY(io->strict_read(io->stream, &packet_type, sizeof(packet_type)));

            if (packet_type > 0)
            {
                /* Run packet: repeat next byte (packet_type) times. */
                unsigned count = packet_type;
                if (x + count > width)
                {
                    count = width - x;
                }
                uint8_t value;
                SAIL_TRY(io->strict_read(io->stream, &value, sizeof(value)));
                memset(line + x, value, count);
                x += count;
            }
            else if (packet_type < 0)
            {
                /* Copy packet: read (-packet_type) bytes. */
                unsigned count = -packet_type;
                if (x + count > width)
                {
                    count = width - x;
                }
                SAIL_TRY(io->strict_read(io->stream, line + x, count));
                x += count;
            }
        }
    }

    return SAIL_OK;
}

sail_status_t fli_private_encode_brun(struct sail_io* io, const unsigned char* pixels, unsigned width, unsigned height)
{
    /* BRUN format: each line starts with packet count byte, followed by packets. */
    /* First pass: build packets for all lines. */
    for (unsigned y = 0; y < height; y++)
    {
        const unsigned char* line = pixels + y * width;

        /* Count packets for this line. */
        unsigned packet_count = 0;
        unsigned x_count      = 0;

        while (x_count < width)
        {
            unsigned run_len = 1;
            while (x_count + run_len < width && run_len < 127 && line[x_count] == line[x_count + run_len])
            {
                run_len++;
            }

            if (run_len >= 3)
            {
                packet_count++;
                x_count += run_len;
            }
            else
            {
                unsigned copy_len = 1;
                while (x_count + copy_len < width && copy_len < 127)
                {
                    unsigned next_run = 1;
                    if (x_count + copy_len + 2 < width)
                    {
                        while (x_count + copy_len + next_run < width && next_run < 3
                               && line[x_count + copy_len] == line[x_count + copy_len + next_run])
                        {
                            next_run++;
                        }
                    }
                    if (next_run >= 3)
                    {
                        break;
                    }
                    copy_len++;
                }
                packet_count++;
                x_count += copy_len;
            }
        }

        /* Write packet count for line. */
        uint8_t pkt_count = (uint8_t)(packet_count > 255 ? 255 : packet_count);
        SAIL_TRY(io->strict_write(io->stream, &pkt_count, sizeof(pkt_count)));

        /* Write packets for line. */
        unsigned x = 0;

        while (x < width)
        {
            /* Check for run. */
            unsigned run_len = 1;
            while (x + run_len < width && run_len < 127 && line[x] == line[x + run_len])
            {
                run_len++;
            }

            if (run_len >= 3)
            {
                /* Run packet: positive value = RUN count. */
                int8_t packet = (int8_t)run_len;
                SAIL_TRY(io->strict_write(io->stream, &packet, sizeof(packet)));
                SAIL_TRY(io->strict_write(io->stream, &line[x], 1));
                x += run_len;
            }
            else
            {
                /* Copy packet: negative value = COPY count. */
                unsigned copy_len = 1;
                while (x + copy_len < width && copy_len < 127)
                {
                    /* Check if next bytes form a run. */
                    unsigned next_run = 1;
                    if (x + copy_len + 2 < width)
                    {
                        while (x + copy_len + next_run < width && next_run < 3
                               && line[x + copy_len] == line[x + copy_len + next_run])
                        {
                            next_run++;
                        }
                    }

                    if (next_run >= 3)
                    {
                        break;
                    }
                    copy_len++;
                }

                int8_t packet = -(int8_t)copy_len;
                SAIL_TRY(io->strict_write(io->stream, &packet, sizeof(packet)));
                SAIL_TRY(io->strict_write(io->stream, line + x, copy_len));
                x += copy_len;
            }
        }
    }

    return SAIL_OK;
}

sail_status_t fli_private_decode_copy(struct sail_io* io, unsigned char* pixels, unsigned width, unsigned height)
{
    SAIL_TRY(io->strict_read(io->stream, pixels, width * height));
    return SAIL_OK;
}

sail_status_t fli_private_encode_copy(struct sail_io* io, const unsigned char* pixels, unsigned width, unsigned height)
{
    SAIL_TRY(io->strict_write(io->stream, pixels, width * height));
    return SAIL_OK;
}

sail_status_t fli_private_decode_lc(struct sail_io* io, unsigned char* pixels, unsigned width, unsigned height)
{
    uint16_t lines_start_y, lines_count;
    SAIL_TRY(io->strict_read(io->stream, &lines_start_y, sizeof(lines_start_y)));
    SAIL_TRY(io->strict_read(io->stream, &lines_count, sizeof(lines_count)));

    for (uint16_t i = 0; i < lines_count; i++)
    {
        unsigned y = lines_start_y + i;
        if (y >= height)
        {
            break;
        }

        unsigned char* line = pixels + y * width;
        uint8_t packets;
        SAIL_TRY(io->strict_read(io->stream, &packets, sizeof(packets)));

        unsigned x = 0;

        for (uint8_t j = 0; j < packets; j++)
        {
            uint8_t skip;
            SAIL_TRY(io->strict_read(io->stream, &skip, sizeof(skip)));
            x += skip;

            int8_t packet;
            SAIL_TRY(io->strict_read(io->stream, &packet, sizeof(packet)));

            if (packet >= 0)
            {
                /* Copy packet. */
                unsigned count = packet;
                if (x + count > width)
                {
                    count = width - x;
                }
                SAIL_TRY(io->strict_read(io->stream, line + x, count));
                x += count;
            }
            else
            {
                /* Run packet. */
                unsigned count = -packet;
                if (x + count > width)
                {
                    count = width - x;
                }
                uint8_t value;
                SAIL_TRY(io->strict_read(io->stream, &value, sizeof(value)));
                memset(line + x, value, count);
                x += count;
            }
        }
    }

    return SAIL_OK;
}

sail_status_t fli_private_decode_ss2(struct sail_io* io, unsigned char* pixels, unsigned width, unsigned height)
{
    uint16_t lines_count;
    SAIL_TRY(io->strict_read(io->stream, &lines_count, sizeof(lines_count)));

    for (uint16_t y = 0; y < lines_count && y < height; y++)
    {
        unsigned char* line = pixels + y * width;
        uint16_t packets;
        SAIL_TRY(io->strict_read(io->stream, &packets, sizeof(packets)));

        /* High byte of packets can indicate lines to skip. */
        uint8_t skip_lines  = (packets >> 8) & 0xFF;
        packets            &= 0xFF;

        if (skip_lines > 0)
        {
            /* Last packet in line - skip to next line. */
            y += skip_lines - 1;
            continue;
        }

        if (packets == 0)
        {
            /* Last line indicator. */
            break;
        }

        unsigned x = 0;

        for (uint16_t j = 0; j < packets; j++)
        {
            uint8_t skip;
            SAIL_TRY(io->strict_read(io->stream, &skip, sizeof(skip)));
            x += skip * 2; /* Word-aligned. */

            int8_t packet;
            SAIL_TRY(io->strict_read(io->stream, &packet, sizeof(packet)));

            if (packet >= 0)
            {
                /* Copy packet (word count). */
                unsigned count = packet * 2;
                if (x + count > width)
                {
                    count = width - x;
                }
                SAIL_TRY(io->strict_read(io->stream, line + x, count));
                x += count;
            }
            else
            {
                /* Run packet. */
                unsigned count = (-packet) * 2;
                if (x + count > width)
                {
                    count = width - x;
                }
                uint8_t value[2];
                SAIL_TRY(io->strict_read(io->stream, value, 2));
                for (unsigned k = 0; k < count; k += 2)
                {
                    if (x + k < width)
                    {
                        line[x + k] = value[0];
                    }
                    if (x + k + 1 < width)
                    {
                        line[x + k + 1] = value[1];
                    }
                }
                x += count;
            }
        }
    }

    return SAIL_OK;
}
