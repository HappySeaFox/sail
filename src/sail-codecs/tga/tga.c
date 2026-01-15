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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static const char* const TGA_SIGNATURE = "TRUEVISION-XFILE.";
static const int TGA_FOOTER_SIZE       = 26;

/*
 * Codec-specific state.
 */
struct tga_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct TgaFileHeader file_header;
    struct TgaFooter footer;

    bool frame_processed;
    bool tga2;
    bool flipped_h;
    bool flipped_v;

    size_t extension_offset;
};

static sail_status_t alloc_tga_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct tga_state** tga_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tga_state), &ptr));
    *tga_state = ptr;

    **tga_state = (struct tga_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_processed = false,
        .tga2            = false,
        .flipped_h       = false,
        .flipped_v       = false,

        .extension_offset = 0,
    };

    return SAIL_OK;
}

static void destroy_tga_state(struct tga_state* tga_state)
{
    if (tga_state == NULL)
    {
        return;
    }

    sail_free(tga_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_tga(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct tga_state* tga_state;
    SAIL_TRY(alloc_tga_state(io, load_options, NULL, &tga_state));
    *state = tga_state;

    /* Read TGA footer. */
    SAIL_TRY(tga_state->io->seek(tga_state->io->stream, -TGA_FOOTER_SIZE, SEEK_END));
    SAIL_TRY(tga_private_read_file_footer(io, &tga_state->footer));
    SAIL_TRY(tga_state->io->seek(tga_state->io->stream, 0, SEEK_SET));

    tga_state->tga2 = strcmp(TGA_SIGNATURE, (const char*)tga_state->footer.signature) == 0;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_tga(void* state, struct sail_image** image)
{
    struct tga_state* tga_state = state;

    if (tga_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    tga_state->frame_processed = true;

    SAIL_TRY(tga_private_read_file_header(tga_state->io, &tga_state->file_header));

    tga_state->flipped_h = tga_state->file_header.descriptor & 0x10;        /* 4th bit set = flipped H.   */
    tga_state->flipped_v = (tga_state->file_header.descriptor & 0x20) == 0; /* 5th bit unset = flipped V. */

    enum SailPixelFormat pixel_format =
        tga_private_sail_pixel_format(tga_state->file_header.image_type, tga_state->file_header.bpp);

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (tga_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        if (tga_state->flipped_h)
        {
            image_local->source_image->orientation = SAIL_ORIENTATION_MIRRORED_HORIZONTALLY;
        }
        else if (tga_state->flipped_v)
        {
            image_local->source_image->orientation = SAIL_ORIENTATION_MIRRORED_VERTICALLY;
        }

        switch (tga_state->file_header.image_type)
        {
        case TGA_INDEXED_RLE:
        case TGA_TRUE_COLOR_RLE:
        case TGA_GRAY_RLE:
        {
            image_local->source_image->compression = SAIL_COMPRESSION_RLE;
            break;
        }
        default:
        {
            image_local->source_image->compression = SAIL_COMPRESSION_NONE;
        }
        }
    }

    image_local->width          = tga_state->file_header.width;
    image_local->height         = tga_state->file_header.height;
    image_local->pixel_format   = pixel_format;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Allocate special properties if needed. */
    struct sail_hash_map* special_properties = NULL;
    if (tga_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        if (image_local->source_image == NULL)
        {
            SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                                /* cleanup */ sail_destroy_image(image_local));
        }

        SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->source_image->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));

        special_properties = image_local->source_image->special_properties;

        /* Add file header specific properties. */
        struct sail_variant* variant;
        SAIL_TRY(sail_alloc_variant(&variant));

        /* Origin X coordinate. */
        sail_set_variant_unsigned_short(variant, tga_state->file_header.x);
        sail_put_hash_map(special_properties, "tga-origin-x", variant);

        /* Origin Y coordinate. */
        sail_set_variant_unsigned_short(variant, tga_state->file_header.y);
        sail_put_hash_map(special_properties, "tga-origin-y", variant);

        /* Alpha bits (bits 0-3 of descriptor). */
        sail_set_variant_unsigned_char(variant, tga_state->file_header.descriptor & 0x0F);
        sail_put_hash_map(special_properties, "tga-alpha-bits", variant);

        /* Flipped horizontally. */
        sail_set_variant_bool(variant, tga_state->flipped_h);
        sail_put_hash_map(special_properties, "tga-flipped-h", variant);

        /* Flipped vertically. */
        sail_set_variant_bool(variant, tga_state->flipped_v);
        sail_put_hash_map(special_properties, "tga-flipped-v", variant);

        sail_destroy_variant(variant);
    }

    /* Identificator. */
    if (tga_state->file_header.id_length > 0)
    {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_id(tga_state->io, &tga_state->file_header, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Extension area. */
    if (tga_state->tga2 && tga_state->footer.extension_area_offset > 0)
    {
        /* Seek to offset. */
        size_t offset;
        SAIL_TRY_OR_CLEANUP(tga_state->io->tell(tga_state->io->stream, &offset),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(
            tga_state->io->seek(tga_state->io->stream, (long)tga_state->footer.extension_area_offset, SEEK_SET),
            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(
            tga_private_fetch_extension(tga_state->io, &image_local->gamma, &image_local->meta_data_node, special_properties),
            /* cleanup */ sail_destroy_image(image_local));

        SAIL_TRY_OR_CLEANUP(tga_state->io->seek(tga_state->io->stream, (long)offset, SEEK_SET),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Palette. */
    if (tga_state->file_header.color_map_type == TGA_HAS_COLOR_MAP)
    {
        SAIL_TRY_OR_CLEANUP(tga_private_fetch_palette(tga_state->io, &tga_state->file_header, &image_local->palette),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_tga(void* state, struct sail_image* image)
{
    struct tga_state* tga_state = state;

    switch (tga_state->file_header.image_type)
    {
    case TGA_INDEXED:
    case TGA_TRUE_COLOR:
    case TGA_GRAY:
    {
        SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, image->pixels,
                                            (size_t)image->bytes_per_line * image->height));
        break;
    }
    case TGA_INDEXED_RLE:
    case TGA_TRUE_COLOR_RLE:
    case TGA_GRAY_RLE:
    {
        const unsigned pixel_size = (tga_state->file_header.bpp + 7) / 8;

        /* Validate pixel size to prevent buffer overflow. */
        if (pixel_size > 16 || pixel_size == 0)
        {
            SAIL_LOG_ERROR("TGA: Invalid pixel size %u (bpp=%d), must be 1-16 bytes", pixel_size,
                           tga_state->file_header.bpp);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
        }
        const unsigned pixels_num = image->width * image->height;

        unsigned char* pixels = image->pixels;

        for (unsigned i = 0; i < pixels_num;)
        {
            unsigned char marker;
            SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, &marker, 1));

            unsigned count = (marker & 0x7F) + 1;

            /* 7th bit set = RLE packet. */
            if (marker & 0x80)
            {
                unsigned char pixel[4];

                SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, pixel, pixel_size));

                /* Round to the buffer size. */
                count = (i + count) <= pixels_num ? count : (pixels_num - i);

                for (unsigned j = 0; j < count; j++, i++)
                {
                    memcpy(pixels, pixel, pixel_size);
                    pixels += pixel_size;
                }
            }
            else
            {
                for (unsigned j = 0; j < count; j++, i++)
                {
                    SAIL_TRY(tga_state->io->strict_read(tga_state->io->stream, pixels, pixel_size));
                    pixels += pixel_size;
                }
            }
        }
        break;
    }
    }

    /* TODO: We can avoid this by putting pixels in reverse order like in the BMP codec. */
    if (tga_state->flipped_v)
    {
        sail_mirror_vertically(image);
    }
    if (tga_state->flipped_h)
    {
        sail_mirror_horizontally(image);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_tga(void** state)
{
    struct tga_state* tga_state = *state;

    *state = NULL;

    destroy_tga_state(tga_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_tga(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct tga_state* tga_state;
    SAIL_TRY(alloc_tga_state(io, NULL, save_options, &tga_state));
    *state = tga_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_tga(void* state, const struct sail_image* image)
{
    struct tga_state* tga_state = state;

    if (tga_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    tga_state->frame_processed = true;

    if (image->width > UINT16_MAX || image->height > UINT16_MAX)
    {
        SAIL_LOG_ERROR("TGA: Image dimensions are too large");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Determine TGA format from pixel format. */
    uint8_t image_type, bpp;
    tga_private_pixel_format_to_tga_format(image->pixel_format, &image_type, &bpp);

    if (image_type == TGA_NO_IMAGE)
    {
        SAIL_LOG_ERROR("TGA: %s pixel format is not supported for saving",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Support RLE compression if requested. */
    if (tga_state->save_options->compression == SAIL_COMPRESSION_RLE)
    {
        switch (image_type)
        {
        case TGA_INDEXED: image_type = TGA_INDEXED_RLE; break;
        case TGA_TRUE_COLOR: image_type = TGA_TRUE_COLOR_RLE; break;
        case TGA_GRAY: image_type = TGA_GRAY_RLE; break;
        }
    }
    else if (tga_state->save_options->compression != SAIL_COMPRESSION_NONE)
    {
        SAIL_LOG_ERROR("TGA: Only NONE and RLE compressions are supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Fill file header. */
    memset(&tga_state->file_header, 0, sizeof(tga_state->file_header));
    tga_state->file_header.id_length  = 0;
    tga_state->file_header.image_type = image_type;
    tga_state->file_header.x          = 0;
    tga_state->file_header.y          = 0;
    tga_state->file_header.width      = (uint16_t)image->width;
    tga_state->file_header.height     = (uint16_t)image->height;
    tga_state->file_header.bpp        = bpp;

    /* Descriptor byte: bits 3-0 = alpha bits, bit 5 = top-left origin. */
    uint8_t alpha_bits = 0;
    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA)
    {
        alpha_bits = 8;
    }
    tga_state->file_header.descriptor = alpha_bits | 0x20; /* 0x20 = top-left origin. */

    /* Setup palette header fields. */
    if (image->palette != NULL)
    {
        tga_state->file_header.color_map_type              = TGA_HAS_COLOR_MAP;
        tga_state->file_header.first_color_map_entry_index = 0;
        tga_state->file_header.color_map_elements          = (uint16_t)image->palette->color_count;

        /* Determine palette entry size. */
        switch (image->palette->pixel_format)
        {
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        {
            tga_state->file_header.color_map_entry_size = 24;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        {
            tga_state->file_header.color_map_entry_size = 32;
            break;
        }
        default:
        {
            SAIL_LOG_ERROR("TGA: Unsupported palette pixel format for writing");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
        }
    }
    else
    {
        tga_state->file_header.color_map_type              = TGA_HAS_NO_COLOR_MAP;
        tga_state->file_header.first_color_map_entry_index = 0;
        tga_state->file_header.color_map_elements          = 0;
        tga_state->file_header.color_map_entry_size        = 0;
    }

    /* Write file header. */
    SAIL_TRY(tga_private_write_file_header(tga_state->io, &tga_state->file_header));

    /* Write palette if exists. */
    if (image->palette != NULL)
    {
        SAIL_TRY(tga_private_write_palette(tga_state->io, image->palette, &tga_state->file_header));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_tga(void* state, const struct sail_image* image)
{
    struct tga_state* tga_state = state;

    const unsigned pixel_size = (tga_state->file_header.bpp + 7) / 8;

    switch (tga_state->file_header.image_type)
    {
    case TGA_INDEXED:
    case TGA_TRUE_COLOR:
    case TGA_GRAY:
    {
        /* Write uncompressed pixel data. */
        SAIL_TRY(tga_state->io->strict_write(tga_state->io->stream, image->pixels,
                                             (size_t)image->bytes_per_line * image->height));
        break;
    }
    case TGA_INDEXED_RLE:
    case TGA_TRUE_COLOR_RLE:
    case TGA_GRAY_RLE:
    {
        /* Write RLE compressed pixel data. */
        const unsigned pixels_num   = image->width * image->height;
        const unsigned char* pixels = image->pixels;

        for (unsigned i = 0; i < pixels_num;)
        {
            /* Look ahead for run length. */
            unsigned run_length                = 1;
            const unsigned char* current_pixel = pixels;

            /* Check for RLE run (repeated pixels). */
            while (run_length < 128 && (i + run_length) < pixels_num)
            {
                if (memcmp(current_pixel, current_pixel + (size_t)run_length * pixel_size, pixel_size) != 0)
                {
                    break;
                }
                run_length++;
            }

            if (run_length > 1)
            {
                /* RLE packet: 1-bit flag (1) + 7-bit count. */
                unsigned char marker = 0x80 | (unsigned char)(run_length - 1);
                SAIL_TRY(tga_state->io->strict_write(tga_state->io->stream, &marker, 1));
                SAIL_TRY(tga_state->io->strict_write(tga_state->io->stream, current_pixel, pixel_size));

                pixels += (size_t)run_length * pixel_size;
                i      += run_length;
            }
            else
            {
                /* Find raw run (non-repeated pixels). */
                unsigned raw_length = 1;
                while (raw_length < 128 && (i + raw_length) < pixels_num)
                {
                    /* Check if next pixel starts a run. */
                    if (raw_length + 1 < 128 && (i + raw_length + 1) < pixels_num)
                    {
                        if (memcmp(pixels + (size_t)raw_length * pixel_size,
                                   pixels + (size_t)(raw_length + 1) * pixel_size, pixel_size)
                            == 0)
                        {
                            break;
                        }
                    }
                    raw_length++;
                }

                /* Raw packet: 1-bit flag (0) + 7-bit count. */
                unsigned char marker = (unsigned char)(raw_length - 1);
                SAIL_TRY(tga_state->io->strict_write(tga_state->io->stream, &marker, 1));
                SAIL_TRY(tga_state->io->strict_write(tga_state->io->stream, pixels, (size_t)raw_length * pixel_size));

                pixels += (size_t)raw_length * pixel_size;
                i      += raw_length;
            }
        }
        break;
    }
    default:
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }

    /* Write extension area if meta data or gamma is present. */
    if ((tga_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL)
        || image->gamma != 0)
    {
        SAIL_TRY(tga_state->io->tell(tga_state->io->stream, &tga_state->extension_offset));
        SAIL_TRY(tga_private_write_extension_area(tga_state->io, image->gamma, image->meta_data_node));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_tga(void** state)
{
    struct tga_state* tga_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Write TGA 2.0 footer. */
    memcpy(tga_state->footer.signature, TGA_SIGNATURE, sizeof(tga_state->footer.signature));
    tga_state->footer.extension_area_offset = (uint32_t)tga_state->extension_offset;
    tga_state->footer.developer_area_offset = 0;

    SAIL_TRY_OR_CLEANUP(tga_private_write_file_footer(tga_state->io, &tga_state->footer),
                        /* cleanup */ destroy_tga_state(tga_state));

    destroy_tga_state(tga_state);

    return SAIL_OK;
}
