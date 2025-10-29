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

#pragma once

#include <stdint.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_io;
struct sail_palette;

/* FLI/FLC magic numbers. */
#define SAIL_FLI_MAGIC 0xAF11
#define SAIL_FLC_MAGIC 0xAF12

/* Frame magic number. */
#define SAIL_FLI_FRAME_MAGIC 0xF1FA

/* Chunk types. */
#define SAIL_FLI_COLOR256 4
#define SAIL_FLI_SS2 7
#define SAIL_FLI_COLOR64 11
#define SAIL_FLI_LC 12
#define SAIL_FLI_BLACK 13
#define SAIL_FLI_BRUN 15
#define SAIL_FLI_COPY 16
#define SAIL_FLI_PSTAMP 18
#define SAIL_FLI_DTA_BRUN 25
#define SAIL_FLI_DTA_COPY 26
#define SAIL_FLI_DTA_LC 27

struct SailFliHeader
{
    uint32_t size;
    uint16_t magic;
    uint16_t frames;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t flags;
    uint32_t speed;
    uint16_t reserved1;
    uint32_t created;
    uint32_t creator;
    uint32_t updated;
    uint32_t updater;
    uint16_t aspect_x;
    uint16_t aspect_y;
    uint8_t reserved2[38];
    uint32_t oframe1;
    uint32_t oframe2;
    uint8_t reserved3[40];
};

struct SailFliFrameHeader
{
    uint32_t size;
    uint16_t magic;
    uint16_t chunks;
    uint16_t delay;
    uint8_t reserved[6];
};

struct SailFliChunkHeader
{
    uint32_t size;
    uint16_t type;
};

SAIL_HIDDEN sail_status_t fli_private_read_header(struct sail_io* io, struct SailFliHeader* header);

SAIL_HIDDEN sail_status_t fli_private_write_header(struct sail_io* io, const struct SailFliHeader* header);

SAIL_HIDDEN sail_status_t fli_private_read_frame_header(struct sail_io* io, struct SailFliFrameHeader* frame_header);

SAIL_HIDDEN sail_status_t fli_private_write_frame_header(struct sail_io* io,
                                                         const struct SailFliFrameHeader* frame_header);

SAIL_HIDDEN sail_status_t fli_private_read_chunk_header(struct sail_io* io, struct SailFliChunkHeader* chunk_header);

SAIL_HIDDEN sail_status_t fli_private_write_chunk_header(struct sail_io* io,
                                                         const struct SailFliChunkHeader* chunk_header);

SAIL_HIDDEN sail_status_t fli_private_decode_color64(struct sail_io* io,
                                                     uint32_t chunk_size,
                                                     struct sail_palette* palette);

SAIL_HIDDEN sail_status_t fli_private_decode_color256(struct sail_io* io,
                                                      uint32_t chunk_size,
                                                      struct sail_palette* palette);

SAIL_HIDDEN sail_status_t fli_private_encode_color256(struct sail_io* io, const struct sail_palette* palette);

SAIL_HIDDEN sail_status_t fli_private_decode_brun(struct sail_io* io,
                                                  unsigned char* pixels,
                                                  unsigned width,
                                                  unsigned height);

SAIL_HIDDEN sail_status_t fli_private_encode_brun(struct sail_io* io,
                                                  const unsigned char* pixels,
                                                  unsigned width,
                                                  unsigned height);

SAIL_HIDDEN sail_status_t fli_private_decode_copy(struct sail_io* io,
                                                  unsigned char* pixels,
                                                  unsigned width,
                                                  unsigned height);

SAIL_HIDDEN sail_status_t fli_private_encode_copy(struct sail_io* io,
                                                  const unsigned char* pixels,
                                                  unsigned width,
                                                  unsigned height);

SAIL_HIDDEN sail_status_t fli_private_decode_lc(struct sail_io* io,
                                                unsigned char* pixels,
                                                unsigned width,
                                                unsigned height);

SAIL_HIDDEN sail_status_t fli_private_decode_ss2(struct sail_io* io,
                                                 unsigned char* pixels,
                                                 unsigned width,
                                                 unsigned height);
