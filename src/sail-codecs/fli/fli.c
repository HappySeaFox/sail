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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct fli_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct SailFliHeader fli_header;
    struct sail_palette* current_palette;
    unsigned char* prev_frame;
    unsigned current_frame_index;
    bool is_fli; /* true for FLI (0xAF11), false for FLC (0xAF12). */

    /* For saving. */
    int frames_written;
    bool is_first_frame;
    unsigned char* first_frame;
};

static sail_status_t alloc_fli_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct fli_state** fli_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct fli_state), &ptr));
    *fli_state = ptr;

    **fli_state = (struct fli_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .current_palette     = NULL,
        .prev_frame          = NULL,
        .current_frame_index = 0,
        .is_fli              = false,

        .frames_written = 0,
        .is_first_frame = true,
        .first_frame    = NULL,
    };

    return SAIL_OK;
}

static void destroy_fli_state(struct fli_state* fli_state)
{
    if (fli_state == NULL)
    {
        return;
    }

    sail_destroy_palette(fli_state->current_palette);
    sail_free(fli_state->prev_frame);
    sail_free(fli_state->first_frame);

    sail_free(fli_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_fli(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct fli_state* fli_state;
    SAIL_TRY(alloc_fli_state(io, load_options, NULL, &fli_state));
    *state = fli_state;

    /* Read FLI header. */
    SAIL_TRY(fli_private_read_header(fli_state->io, &fli_state->fli_header));

    /* Validate magic number. */
    if (fli_state->fli_header.magic != SAIL_FLI_MAGIC && fli_state->fli_header.magic != SAIL_FLC_MAGIC)
    {
        SAIL_LOG_ERROR("FLI: Invalid magic number 0x%04X", fli_state->fli_header.magic);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    fli_state->is_fli = (fli_state->fli_header.magic == SAIL_FLI_MAGIC);

    /* Validate dimensions. */
    if (fli_state->fli_header.width == 0 || fli_state->fli_header.height == 0)
    {
        SAIL_LOG_ERROR("FLI: Invalid dimensions %ux%u", fli_state->fli_header.width, fli_state->fli_header.height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Validate depth (must be 8 for indexed color). */
    if (fli_state->fli_header.depth != 8)
    {
        SAIL_LOG_ERROR("FLI: Unsupported bit depth %u", fli_state->fli_header.depth);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    /* Allocate palette. */
    SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, 256, &fli_state->current_palette));

    /* Initialize palette to black. */
    memset(fli_state->current_palette->data, 0, 256 * 3);

    /* Allocate buffer for previous frame (for delta decompression). */
    void* ptr;
    SAIL_TRY(sail_malloc((size_t)fli_state->fli_header.width * fli_state->fli_header.height, &ptr));
    fli_state->prev_frame = ptr;
    memset(fli_state->prev_frame, 0, (size_t)fli_state->fli_header.width * fli_state->fli_header.height);

    SAIL_LOG_TRACE("FLI: %s format, %ux%u, %u frames, speed=%u", fli_state->is_fli ? "FLI" : "FLC",
                   fli_state->fli_header.width, fli_state->fli_header.height, fli_state->fli_header.frames,
                   fli_state->fli_header.speed);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_fli(void* state, struct sail_image** image)
{
    struct fli_state* fli_state = state;

    if (fli_state->current_frame_index >= fli_state->fli_header.frames)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (fli_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        image_local->source_image->compression  = SAIL_COMPRESSION_RLE;
    }

    image_local->width          = fli_state->fli_header.width;
    image_local->height         = fli_state->fli_header.height;
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Calculate delay. */
    if (fli_state->is_fli)
    {
        /* FLI: speed is in 1/70 second units. */
        image_local->delay = (int)(fli_state->fli_header.speed * 1000.0 / 70.0);
    }
    else
    {
        /* FLC: speed is in milliseconds. */
        image_local->delay = fli_state->fli_header.speed;
    }

    /* Copy palette. */
    SAIL_TRY_OR_CLEANUP(sail_copy_palette(fli_state->current_palette, &image_local->palette),
                        /* cleanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_fli(void* state, struct sail_image* image)
{
    struct fli_state* fli_state = state;

    /* Remember frame start position. */
    size_t frame_start_pos;
    SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &frame_start_pos));

    /* Read frame header. */
    struct SailFliFrameHeader frame_header;
    SAIL_TRY(fli_private_read_frame_header(fli_state->io, &frame_header));

    SAIL_LOG_TRACE("FLI: Frame %u at 0x%zX: size=%u, magic=0x%04X, chunks=%u, delay=%u", fli_state->current_frame_index,
                   frame_start_pos, frame_header.size, frame_header.magic, frame_header.chunks, frame_header.delay);

    if (frame_header.magic != SAIL_FLI_FRAME_MAGIC)
    {
        SAIL_LOG_ERROR("FLI: Invalid frame magic 0x%04X", frame_header.magic);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Copy previous frame to current frame. */
    memcpy(image->pixels, fli_state->prev_frame, (size_t)image->width * image->height);

    /* Process chunks. */
    for (unsigned i = 0; i < frame_header.chunks; i++)
    {
        size_t chunk_start_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &chunk_start_pos));

        struct SailFliChunkHeader chunk_header;
        SAIL_TRY(fli_private_read_chunk_header(fli_state->io, &chunk_header));

        switch (chunk_header.type)
        {
        case SAIL_FLI_COLOR256:
        {
            SAIL_TRY(fli_private_decode_color256(fli_state->io, chunk_header.size, fli_state->current_palette));
            sail_destroy_palette(image->palette);
            SAIL_TRY(sail_copy_palette(fli_state->current_palette, &image->palette));
            break;
        }

        case SAIL_FLI_COLOR64:
        {
            SAIL_TRY(fli_private_decode_color64(fli_state->io, chunk_header.size, fli_state->current_palette));
            sail_destroy_palette(image->palette);
            SAIL_TRY(sail_copy_palette(fli_state->current_palette, &image->palette));
            break;
        }

        case SAIL_FLI_BLACK:
        {
            memset(image->pixels, 0, (size_t)image->width * image->height);
            break;
        }

        case SAIL_FLI_BRUN:
        case SAIL_FLI_DTA_BRUN:
        {
            SAIL_TRY(fli_private_decode_brun(fli_state->io, image->pixels, image->width, image->height));
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)(chunk_start_pos + chunk_header.size), SEEK_SET));
            break;
        }

        case SAIL_FLI_COPY:
        case SAIL_FLI_DTA_COPY:
        {
            SAIL_TRY(fli_private_decode_copy(fli_state->io, image->pixels, image->width, image->height));
            break;
        }

        case SAIL_FLI_LC:
        case SAIL_FLI_DTA_LC:
        {
            SAIL_TRY(fli_private_decode_lc(fli_state->io, image->pixels, image->width, image->height));
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)(chunk_start_pos + chunk_header.size), SEEK_SET));
            break;
        }

        case SAIL_FLI_SS2:
        {
            SAIL_TRY(fli_private_decode_ss2(fli_state->io, image->pixels, image->width, image->height));
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)(chunk_start_pos + chunk_header.size), SEEK_SET));
            break;
        }

        case SAIL_FLI_PSTAMP:
        {
            size_t bytes_to_skip = chunk_header.size - sizeof(chunk_header);
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)bytes_to_skip, SEEK_CUR));
            break;
        }

        default:
        {
            SAIL_LOG_WARNING("FLI: Unknown chunk type %u, skipping", chunk_header.type);
            size_t bytes_to_skip = chunk_header.size - sizeof(chunk_header);
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)bytes_to_skip, SEEK_CUR));
            break;
        }
        }

        /* Ensure we're at the correct position after chunk. */
        size_t current_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &current_pos));
        size_t expected_pos = chunk_start_pos + chunk_header.size;

        if (current_pos != expected_pos)
        {
            SAIL_LOG_WARNING("FLI: Chunk %u position mismatch: at 0x%zX, expected 0x%zX (diff=%zd)", i, current_pos,
                             expected_pos, (long)expected_pos - (long)current_pos);
            SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)expected_pos, SEEK_SET));
        }
    }

    /* Seek to next frame using frame size from header. */
    size_t next_frame_pos = frame_start_pos + frame_header.size;
    SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)next_frame_pos, SEEK_SET));

    /* Save current frame for next delta. */
    memcpy(fli_state->prev_frame, image->pixels, (size_t)image->width * image->height);

    fli_state->current_frame_index++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_fli(void** state)
{
    struct fli_state* fli_state = *state;

    *state = NULL;

    destroy_fli_state(fli_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_fli(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    /* Check compression. */
    if (save_options->compression != SAIL_COMPRESSION_RLE)
    {
        SAIL_LOG_ERROR("FLI: Only RLE compression is supported for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Allocate a new state. */
    struct fli_state* fli_state;
    SAIL_TRY(alloc_fli_state(io, NULL, save_options, &fli_state));
    *state = fli_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_fli(void* state, const struct sail_image* image)
{
    struct fli_state* fli_state = state;

    /* FLI only supports 8-bit indexed images. */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP8_INDEXED)
    {
        SAIL_LOG_ERROR("FLI: Only BPP8-INDEXED pixel format is supported for writing, got %s",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (image->palette == NULL)
    {
        SAIL_LOG_ERROR("FLI: Indexed image must have a palette");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
    }

    if (image->palette->color_count != 256)
    {
        SAIL_LOG_ERROR("FLI: Palette must have exactly 256 colors, got %u", image->palette->color_count);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* First frame: initialize FLI and write header. */
    if (fli_state->is_first_frame)
    {
        fli_state->is_first_frame = false;

        /* Determine if we're writing FLI or FLC. */
        fli_state->is_fli = (image->width == 320 && image->height == 200);

        /* Validate dimensions fit in uint16_t. */
        if (image->width > UINT16_MAX || image->height > UINT16_MAX)
        {
            SAIL_LOG_ERROR("FLI: Image dimensions %ux%u exceed maximum allowed (%ux%u)",
                           image->width, image->height, UINT16_MAX, UINT16_MAX);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
        }

        /* Fill FLI header. */
        memset(&fli_state->fli_header, 0, sizeof(fli_state->fli_header));
        fli_state->fli_header.size   = 0;
        fli_state->fli_header.magic  = fli_state->is_fli ? SAIL_FLI_MAGIC : SAIL_FLC_MAGIC;
        fli_state->fli_header.frames = 0;
        fli_state->fli_header.width  = (uint16_t)image->width;
        fli_state->fli_header.height = (uint16_t)image->height;
        fli_state->fli_header.depth  = 8;
        fli_state->fli_header.flags  = 0;

        if (fli_state->is_fli)
        {
            /* FLI: convert milliseconds to 1/70 second units. */
            fli_state->fli_header.speed = (uint32_t)(image->delay * 70.0 / 1000.0);
            if (fli_state->fli_header.speed == 0)
            {
                fli_state->fli_header.speed = 5; /* Default ~70ms. */
            }
        }
        else
        {
            /* FLC: speed is in milliseconds. */
            fli_state->fli_header.speed = (image->delay > 0) ? image->delay : 70;
        }

        fli_state->fli_header.aspect_x = 6;
        fli_state->fli_header.aspect_y = 5;

        SAIL_TRY(fli_private_write_header(fli_state->io, &fli_state->fli_header));

        void* ptr;
        SAIL_TRY(sail_malloc((size_t)image->width * image->height, &ptr));
        fli_state->first_frame = ptr;
    }

    fli_state->frames_written++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_fli(void* state, const struct sail_image* image)
{
    struct fli_state* fli_state = state;

    size_t frame_pos;
    SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &frame_pos));

    struct SailFliFrameHeader frame_header;
    memset(&frame_header, 0, sizeof(frame_header));
    frame_header.magic  = SAIL_FLI_FRAME_MAGIC;
    frame_header.chunks = 0;
    frame_header.delay  = 0;

    SAIL_TRY(fli_private_write_frame_header(fli_state->io, &frame_header));

    size_t chunks_start_pos;
    SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &chunks_start_pos));

    uint16_t chunk_count = 0;

    /* Write COLOR256 chunk for palette. */
    {
        size_t chunk_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &chunk_pos));

        struct SailFliChunkHeader chunk_header;
        chunk_header.size = 0; /* Will be updated. */
        chunk_header.type = SAIL_FLI_COLOR256;
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));

        SAIL_TRY(fli_private_encode_color256(fli_state->io, image->palette));

        size_t end_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &end_pos));

        /* Update chunk size. */
        chunk_header.size = (uint32_t)(end_pos - chunk_pos);
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)chunk_pos, SEEK_SET));
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)end_pos, SEEK_SET));

        chunk_count++;
    }

    /* Write pixel data using BRUN for first frame or LC for subsequent frames. */
    if (fli_state->frames_written == 1)
    {
        /* First frame: use BRUN compression. */
        size_t chunk_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &chunk_pos));

        struct SailFliChunkHeader chunk_header;
        chunk_header.size = 0;
        chunk_header.type = SAIL_FLI_BRUN;
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));

        SAIL_TRY(fli_private_encode_brun(fli_state->io, image->pixels, image->width, image->height));

        size_t end_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &end_pos));

        chunk_header.size = (uint32_t)(end_pos - chunk_pos);
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)chunk_pos, SEEK_SET));
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)end_pos, SEEK_SET));

        chunk_count++;

        /* Save first frame for later. */
        memcpy(fli_state->first_frame, image->pixels, (size_t)image->width * image->height);
    }
    else
    {
        /* Subsequent frames: use COPY for simplicity. */
        /* A real encoder would use LC or SS2 delta compression. */
        size_t chunk_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &chunk_pos));

        struct SailFliChunkHeader chunk_header;
        chunk_header.size = 0;
        chunk_header.type = SAIL_FLI_COPY;
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));

        SAIL_TRY(fli_private_encode_copy(fli_state->io, image->pixels, image->width, image->height));

        size_t end_pos;
        SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &end_pos));

        chunk_header.size = (uint32_t)(end_pos - chunk_pos);
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)chunk_pos, SEEK_SET));
        SAIL_TRY(fli_private_write_chunk_header(fli_state->io, &chunk_header));
        SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)end_pos, SEEK_SET));

        chunk_count++;
    }

    /* Update frame header. */
    size_t end_pos;
    SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &end_pos));

    frame_header.size   = (uint32_t)(end_pos - frame_pos);
    frame_header.chunks = chunk_count;

    SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)frame_pos, SEEK_SET));
    SAIL_TRY(fli_private_write_frame_header(fli_state->io, &frame_header));
    SAIL_TRY(fli_state->io->seek(fli_state->io->stream, (long)end_pos, SEEK_SET));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_fli(void** state)
{
    struct fli_state* fli_state = *state;

    *state = NULL;

    /* Update file header with actual frame count and file size. */
    size_t file_size;
    SAIL_TRY(fli_state->io->tell(fli_state->io->stream, &file_size));

    fli_state->fli_header.size   = (uint32_t)file_size;
    fli_state->fli_header.frames = (uint16_t)fli_state->frames_written;

    SAIL_TRY(fli_state->io->seek(fli_state->io->stream, 0, SEEK_SET));
    SAIL_TRY(fli_private_write_header(fli_state->io, &fli_state->fli_header));

    destroy_fli_state(fli_state);

    return SAIL_OK;
}
