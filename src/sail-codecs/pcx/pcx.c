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
#include <stdint.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/* PCX signature. */
static const uint8_t SAIL_PCX_SIGNATURE = 0x0A;

/* RLE markers. */
static const uint8_t SAIL_PCX_RLE_MARKER     = 0xC0;
static const uint8_t SAIL_PCX_RLE_COUNT_MASK = 0x3F;

/*
 * Codec-specific state.
 */
struct pcx_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct SailPcxHeader pcx_header;
    unsigned char* scanline_buffer; /* buffer to read/write a single plane scan line. */

    bool frame_loaded;
    bool frame_saved;
};

static sail_status_t alloc_pcx_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct pcx_state** pcx_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct pcx_state), &ptr));
    *pcx_state = ptr;

    **pcx_state = (struct pcx_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .scanline_buffer = NULL,
        .frame_loaded    = false,
        .frame_saved     = false,
    };

    return SAIL_OK;
}

static void destroy_pcx_state(struct pcx_state* pcx_state)
{
    if (pcx_state == NULL)
    {
        return;
    }

    sail_free(pcx_state->scanline_buffer);

    sail_free(pcx_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_pcx(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct pcx_state* pcx_state;
    SAIL_TRY(alloc_pcx_state(io, load_options, NULL, &pcx_state));
    *state = pcx_state;

    /* Read PCX header. */
    SAIL_TRY(pcx_private_read_header(pcx_state->io, &pcx_state->pcx_header));

    if (pcx_state->pcx_header.id != SAIL_PCX_SIGNATURE)
    {
        SAIL_LOG_ERROR("PCX: ID is %u, but must be %u", pcx_state->pcx_header.id, SAIL_PCX_SIGNATURE);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    if (pcx_state->pcx_header.bytes_per_line == 0)
    {
        SAIL_LOG_ERROR("PCX: Bytes per line is 0");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    SAIL_LOG_TRACE("PCX: planes(%u), bytes per line(%u), compressed(%s)", pcx_state->pcx_header.planes,
                   pcx_state->pcx_header.bytes_per_line,
                   (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING) ? "no" : "yes");

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_pcx(void* state, struct sail_image** image)
{
    struct pcx_state* pcx_state = state;

    if (pcx_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    pcx_state->frame_loaded = true;

    enum SailPixelFormat pixel_format;
    SAIL_TRY(pcx_private_sail_pixel_format(pcx_state->pcx_header.bits_per_plane, pcx_state->pcx_header.planes,
                                           &pixel_format));

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (pcx_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression =
            (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING) ? SAIL_COMPRESSION_NONE : SAIL_COMPRESSION_RLE;
    }

    image_local->width          = pcx_state->pcx_header.xmax - pcx_state->pcx_header.xmin + 1;
    image_local->height         = pcx_state->pcx_header.ymax - pcx_state->pcx_header.ymin + 1;
    image_local->pixel_format   = pixel_format;
    image_local->bytes_per_line = pcx_state->pcx_header.bytes_per_line * pcx_state->pcx_header.planes;

    /* Scan line buffer to store planes so we can merge them later into individual pixels. */
    void* ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(image_local->bytes_per_line, &ptr),
                        /* cleanup */ sail_destroy_image(image_local));
    pcx_state->scanline_buffer = ptr;

    /* Build palette if needed. */
    SAIL_TRY_OR_CLEANUP(pcx_private_build_palette(image_local->pixel_format, pcx_state->io,
                                                  pcx_state->pcx_header.palette, &image_local->palette),
                        /* cleanup */ sail_destroy_image(image_local));

    if (pcx_state->pcx_header.hdpi > 0 && pcx_state->pcx_header.vdpi > 0)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_resolution_from_data(SAIL_RESOLUTION_UNIT_INCH, pcx_state->pcx_header.hdpi,
                                                            pcx_state->pcx_header.vdpi, &image_local->resolution),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_pcx(void* state, struct sail_image* image)
{
    const struct pcx_state* pcx_state = state;

    if (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING)
    {
        SAIL_TRY(pcx_private_read_uncompressed(pcx_state->io, pcx_state->pcx_header.bytes_per_line,
                                               pcx_state->pcx_header.planes, pcx_state->scanline_buffer, image));
    }
    else
    {
        for (unsigned row = 0; row < image->height; row++)
        {
            unsigned buffer_offset = 0;

            /* Decode all planes of a single scan line. */
            for (unsigned bytes = 0; bytes < image->bytes_per_line;)
            {
                uint8_t marker;
                SAIL_TRY(pcx_state->io->strict_read(pcx_state->io->stream, &marker, sizeof(marker)));

                uint8_t count;
                uint8_t value;

                /* RLE marker set. */
                if ((marker & SAIL_PCX_RLE_MARKER) == SAIL_PCX_RLE_MARKER)
                {
                    count = marker & SAIL_PCX_RLE_COUNT_MASK;
                    SAIL_TRY(pcx_state->io->strict_read(pcx_state->io->stream, &value, sizeof(value)));
                }
                else
                {
                    /* Pixel value. */
                    count = 1;
                    value = marker;
                }

                /* Round to the buffer size. */
                count = (bytes + count) <= image->bytes_per_line ? count : (uint8_t)(image->bytes_per_line - bytes);

                bytes += count;

                memset(pcx_state->scanline_buffer + buffer_offset, value, count);
                buffer_offset += count;
            }

            /* Merge planes into the image pixels. */
            unsigned char* const scan = sail_scan_line(image, row);

            for (unsigned plane = 0; plane < pcx_state->pcx_header.planes; plane++)
            {
                const unsigned buffer_plane_offset = plane * pcx_state->pcx_header.bytes_per_line;

                for (unsigned column = 0; column < pcx_state->pcx_header.bytes_per_line; column++)
                {
                    *(scan + column * pcx_state->pcx_header.planes + plane) =
                        *(pcx_state->scanline_buffer + buffer_plane_offset + column);
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_pcx(void** state)
{
    struct pcx_state* pcx_state = *state;

    *state = NULL;

    destroy_pcx_state(pcx_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_pcx(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct pcx_state* pcx_state;
    SAIL_TRY(alloc_pcx_state(io, NULL, save_options, &pcx_state));
    *state = pcx_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_pcx(void* state, const struct sail_image* image)
{
    struct pcx_state* pcx_state = state;

    if (pcx_state->frame_saved)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    pcx_state->frame_saved = true;

    /* Determine PCX format from pixel format. */
    uint8_t bits_per_plane, planes;
    SAIL_TRY(pcx_private_pixel_format_to_pcx_format(image->pixel_format, &bits_per_plane, &planes));

    /* Support RLE compression if requested. */
    uint8_t encoding;
    if (pcx_state->save_options->compression == SAIL_COMPRESSION_RLE)
    {
        encoding = SAIL_PCX_RLE_ENCODING;
    }
    else if (pcx_state->save_options->compression == SAIL_COMPRESSION_NONE)
    {
        encoding = SAIL_PCX_NO_ENCODING;
    }
    else
    {
        SAIL_LOG_ERROR("PCX: Only NONE and RLE compressions are supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Calculate bytes per line (must be even). */
    unsigned bytes_per_plane = (image->width * bits_per_plane + 7) / 8;
    if (bytes_per_plane % 2 != 0)
    {
        bytes_per_plane++;
    }

    /* Fill PCX header. */
    memset(&pcx_state->pcx_header, 0, sizeof(pcx_state->pcx_header));
    pcx_state->pcx_header.id             = SAIL_PCX_SIGNATURE;
    pcx_state->pcx_header.version        = SAIL_PCX_V5;
    pcx_state->pcx_header.encoding       = encoding;
    pcx_state->pcx_header.bits_per_plane = bits_per_plane;
    pcx_state->pcx_header.xmin           = 0;
    pcx_state->pcx_header.ymin           = 0;
    pcx_state->pcx_header.xmax           = (uint16_t)(image->width - 1);
    pcx_state->pcx_header.ymax           = (uint16_t)(image->height - 1);
    pcx_state->pcx_header.planes         = planes;
    pcx_state->pcx_header.bytes_per_line = (uint16_t)bytes_per_plane;

    /* Set DPI from resolution if available. */
    if (image->resolution != NULL)
    {
        if (image->resolution->unit == SAIL_RESOLUTION_UNIT_INCH)
        {
            pcx_state->pcx_header.hdpi = (uint16_t)image->resolution->x;
            pcx_state->pcx_header.vdpi = (uint16_t)image->resolution->y;
        }
        else if (image->resolution->unit == SAIL_RESOLUTION_UNIT_CENTIMETER)
        {
            pcx_state->pcx_header.hdpi = (uint16_t)(image->resolution->x * 2.54);
            pcx_state->pcx_header.vdpi = (uint16_t)(image->resolution->y * 2.54);
        }
    }

    /* Default DPI. */
    if (pcx_state->pcx_header.hdpi == 0)
    {
        pcx_state->pcx_header.hdpi = 72;
    }
    if (pcx_state->pcx_header.vdpi == 0)
    {
        pcx_state->pcx_header.vdpi = 72;
    }

    /* Setup palette header fields for indexed images. */
    if (sail_is_indexed(image->pixel_format))
    {
        /* Indexed formats require a palette. */
        if (image->palette == NULL)
        {
            SAIL_LOG_ERROR("PCX: Indexed pixel format requires a palette");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }

        pcx_state->pcx_header.palette_info = 1; /* Color palette. */

        /* Check palette format (support both RGB and RGBA). */
        if (image->palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB
            && image->palette->pixel_format != SAIL_PIXEL_FORMAT_BPP32_RGBA)
        {
            SAIL_LOG_ERROR("PCX: Unsupported palette pixel format %s. Only BPP24-RGB and BPP32-RGBA are supported",
                           sail_pixel_format_to_string(image->palette->pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }

        /* Validate and copy palette into header (1-bit, 4-bit) or validate (8-bit). */
        if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED)
        {
            if (image->palette->color_count > 16)
            {
                SAIL_LOG_ERROR("PCX: 4-bit indexed images support maximum 16 colors, got %u",
                               image->palette->color_count);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
            pcx_private_palette_to_rgb(image->palette, pcx_state->pcx_header.palette, 16);
        }
        else if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED)
        {
            if (image->palette->color_count > 256)
            {
                SAIL_LOG_ERROR("PCX: 8-bit indexed images support maximum 256 colors, got %u",
                               image->palette->color_count);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }
        else if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED)
        {
            if (image->palette->color_count > 2)
            {
                SAIL_LOG_ERROR("PCX: 1-bit indexed images support maximum 2 colors, got %u",
                               image->palette->color_count);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
            pcx_private_palette_to_rgb(image->palette, pcx_state->pcx_header.palette, 2);
        }
    }

    /* Write PCX header. */
    SAIL_TRY(pcx_private_write_header(pcx_state->io, &pcx_state->pcx_header));

    /* Allocate scanline buffer. */
    void* ptr;
    SAIL_TRY(sail_malloc(bytes_per_plane * planes, &ptr));
    pcx_state->scanline_buffer = ptr;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_pcx(void* state, const struct sail_image* image)
{
    struct pcx_state* pcx_state = state;

    if (pcx_state->pcx_header.encoding == SAIL_PCX_NO_ENCODING)
    {
        /* Write uncompressed data. */
        for (unsigned row = 0; row < image->height; row++)
        {
            const unsigned char* scan = sail_scan_line(image, row);

            /* Convert interleaved pixels to planar format. */
            for (unsigned plane = 0; plane < pcx_state->pcx_header.planes; plane++)
            {
                for (unsigned column = 0; column < pcx_state->pcx_header.bytes_per_line; column++)
                {
                    if (column * pcx_state->pcx_header.planes + plane < image->bytes_per_line)
                    {
                        pcx_state->scanline_buffer[plane * pcx_state->pcx_header.bytes_per_line + column] =
                            scan[column * pcx_state->pcx_header.planes + plane];
                    }
                    else
                    {
                        pcx_state->scanline_buffer[plane * pcx_state->pcx_header.bytes_per_line + column] = 0;
                    }
                }
            }

            /* Write all planes. */
            SAIL_TRY(pcx_state->io->strict_write(pcx_state->io->stream, pcx_state->scanline_buffer,
                                                 pcx_state->pcx_header.bytes_per_line * pcx_state->pcx_header.planes));
        }
    }
    else
    {
        /* Write RLE compressed data. */
        for (unsigned row = 0; row < image->height; row++)
        {
            const unsigned char* scan = sail_scan_line(image, row);

            /* Convert interleaved pixels to planar format. */
            for (unsigned plane = 0; plane < pcx_state->pcx_header.planes; plane++)
            {
                for (unsigned column = 0; column < pcx_state->pcx_header.bytes_per_line; column++)
                {
                    if (column * pcx_state->pcx_header.planes + plane < image->bytes_per_line)
                    {
                        pcx_state->scanline_buffer[plane * pcx_state->pcx_header.bytes_per_line + column] =
                            scan[column * pcx_state->pcx_header.planes + plane];
                    }
                    else
                    {
                        pcx_state->scanline_buffer[plane * pcx_state->pcx_header.bytes_per_line + column] = 0;
                    }
                }
            }

            /* RLE encode and write each plane. */
            for (unsigned plane = 0; plane < pcx_state->pcx_header.planes; plane++)
            {
                const unsigned char* plane_data =
                    pcx_state->scanline_buffer + plane * pcx_state->pcx_header.bytes_per_line;
                unsigned i = 0;

                while (i < pcx_state->pcx_header.bytes_per_line)
                {
                    unsigned char value = plane_data[i];
                    unsigned count      = 1;

                    /* Count consecutive identical bytes (max 63). */
                    while (count < 63 && i + count < pcx_state->pcx_header.bytes_per_line
                           && plane_data[i + count] == value)
                    {
                        count++;
                    }

                    /* Write RLE packet if count > 1 or value has RLE marker bits set. */
                    if (count > 1 || (value & SAIL_PCX_RLE_MARKER) == SAIL_PCX_RLE_MARKER)
                    {
                        uint8_t marker = SAIL_PCX_RLE_MARKER | (uint8_t)count;
                        SAIL_TRY(pcx_state->io->strict_write(pcx_state->io->stream, &marker, sizeof(marker)));
                        SAIL_TRY(pcx_state->io->strict_write(pcx_state->io->stream, &value, sizeof(value)));
                    }
                    else
                    {
                        /* Write raw byte. */
                        SAIL_TRY(pcx_state->io->strict_write(pcx_state->io->stream, &value, sizeof(value)));
                    }

                    i += count;
                }
            }
        }
    }

    /* Write 256-color palette at the end of file if needed. */
    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED && image->palette != NULL)
    {
        SAIL_TRY(pcx_private_write_palette(pcx_state->io, image->palette));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_pcx(void** state)
{
    struct pcx_state* pcx_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_pcx_state(pcx_state);

    return SAIL_OK;
}
