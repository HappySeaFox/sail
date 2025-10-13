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

/*
 * Codec-specific state.
 */
struct wal_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    unsigned frame_number;

    struct WalFileHeader wal_header;
    unsigned width;
    unsigned height;

    /* For saving. */
    bool header_written;
    void* mipmap_buffers[4];
    size_t mipmap_sizes[4];
};

static sail_status_t alloc_wal_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct wal_state** wal_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct wal_state), &ptr));
    *wal_state = ptr;

    **wal_state = (struct wal_state){
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_number = 0,
        .width        = 0,
        .height       = 0,

        .header_written = false,
        .mipmap_buffers = {NULL, NULL, NULL, NULL},
        .mipmap_sizes   = {0, 0, 0, 0},
    };

    return SAIL_OK;
}

static void destroy_wal_state(struct wal_state* wal_state)
{
    if (wal_state == NULL)
    {
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        sail_free(wal_state->mipmap_buffers[i]);
    }

    sail_free(wal_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_wal(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct wal_state* wal_state;
    SAIL_TRY(alloc_wal_state(io, load_options, NULL, &wal_state));
    *state = wal_state;

    /* Read WAL header. */
    SAIL_TRY(wal_private_read_file_header(wal_state->io, &wal_state->wal_header));

    wal_state->width  = wal_state->wal_header.width;
    wal_state->height = wal_state->wal_header.height;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_wal(void* state, struct sail_image** image)
{
    struct wal_state* wal_state = state;

    if (wal_state->frame_number >= 4)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    if (wal_state->frame_number > 0)
    {
        wal_state->width  /= 2;
        wal_state->height /= 2;
    }

    /* Validate dimensions for this mipmap level. */
    if (wal_state->width == 0 || wal_state->height == 0)
    {
        SAIL_LOG_ERROR("WAL: Invalid mipmap level %u dimensions: %ux%u", wal_state->frame_number, wal_state->width,
                       wal_state->height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    /* Check for potential overflow in image size calculation. */
    const size_t image_size = (size_t)wal_state->width * wal_state->height;
    if (image_size > SIZE_MAX / 2)
    {
        SAIL_LOG_ERROR("WAL: Image size calculation overflow for dimensions %ux%u", wal_state->width,
                       wal_state->height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (wal_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        image_local->source_image->compression  = SAIL_COMPRESSION_NONE;
    }

    image_local->width          = wal_state->width;
    image_local->height         = wal_state->height;
    image_local->pixel_format   = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    SAIL_TRY_OR_CLEANUP(wal_private_assign_palette(image_local),
                        /* cleanup */ sail_destroy_image(image_local));
    SAIL_TRY_OR_CLEANUP(wal_private_assign_meta_data(&wal_state->wal_header, &image_local->meta_data_node),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(
        wal_state->io->seek(wal_state->io->stream, wal_state->wal_header.offset[wal_state->frame_number], SEEK_SET),
        /* cleanup */ sail_destroy_image(image_local));

    wal_state->frame_number++;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_wal(void* state, struct sail_image* image)
{
    struct wal_state* wal_state = state;

    const size_t bytes_to_read = (size_t)image->bytes_per_line * image->height;

    if (bytes_to_read == 0)
    {
        SAIL_LOG_ERROR("WAL: Invalid image size for reading");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    SAIL_TRY(wal_state->io->strict_read(wal_state->io->stream, image->pixels, bytes_to_read));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_wal(void** state)
{
    struct wal_state* wal_state = *state;

    *state = NULL;

    destroy_wal_state(wal_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_wal(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct wal_state* wal_state;
    SAIL_TRY(alloc_wal_state(io, NULL, save_options, &wal_state));
    *state = wal_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_wal(void* state, const struct sail_image* image)
{
    struct wal_state* wal_state = state;

    /* WAL format supports up to 4 mipmap levels. */
    if (wal_state->frame_number >= 4)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Verify pixel format. */
    SAIL_TRY(wal_private_supported_write_pixel_format(image->pixel_format));

    /* First frame determines the dimensions. */
    if (wal_state->frame_number == 0)
    {
        /* Check that dimensions are valid and divisible by 8 for mipmaps. */
        if (image->width == 0 || image->height == 0)
        {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
        }
        if ((image->width % 8) != 0 || (image->height % 8) != 0)
        {
            SAIL_LOG_ERROR("WAL: Image dimensions must be divisible by 8 for mipmap generation. Got %ux%u",
                           image->width, image->height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
        }

        wal_state->width  = image->width;
        wal_state->height = image->height;

        /* Initialize header. */
        memset(&wal_state->wal_header, 0, sizeof(wal_state->wal_header));

        /* Extract texture name from metadata if available. */
        const struct sail_meta_data_node* meta_data_node = image->meta_data_node;
        while (meta_data_node != NULL)
        {
            if (meta_data_node->meta_data != NULL && meta_data_node->meta_data->key == SAIL_META_DATA_NAME
                && meta_data_node->meta_data->value != NULL
                && meta_data_node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING)
            {
                const char* name = sail_variant_to_string(meta_data_node->meta_data->value);
#ifdef _MSC_VER
                strncpy_s(wal_state->wal_header.name, sizeof(wal_state->wal_header.name), name, sizeof(wal_state->wal_header.name) - 1);
#else
                strncpy(wal_state->wal_header.name, name, sizeof(wal_state->wal_header.name) - 1);
#endif
                wal_state->wal_header.name[sizeof(wal_state->wal_header.name) - 1] = '\0';
                break;
            }
            meta_data_node = meta_data_node->next;
        }

        wal_state->wal_header.width  = wal_state->width;
        wal_state->wal_header.height = wal_state->height;
    }
    else
    {
        /* Verify subsequent frames have correct dimensions (half of previous). */
        unsigned expected_width  = wal_state->width;
        unsigned expected_height = wal_state->height;

        for (unsigned i = 0; i < wal_state->frame_number; i++)
        {
            expected_width  /= 2;
            expected_height /= 2;
        }

        if (image->width != expected_width || image->height != expected_height)
        {
            SAIL_LOG_ERROR("WAL: Mipmap level %u has incorrect dimensions. Expected %ux%u, got %ux%u",
                           wal_state->frame_number, expected_width, expected_height, image->width, image->height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
        }
    }

    wal_state->frame_number++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_wal(void* state, const struct sail_image* image)
{
    struct wal_state* wal_state = state;

    /* Store the mipmap data. */
    const unsigned mipmap_index = wal_state->frame_number - 1;
    const size_t data_size      = (size_t)image->width * image->height;

    void* buffer;
    SAIL_TRY(sail_malloc(data_size, &buffer));
    memcpy(buffer, image->pixels, data_size);

    wal_state->mipmap_buffers[mipmap_index] = buffer;
    wal_state->mipmap_sizes[mipmap_index]   = data_size;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_wal(void** state)
{
    struct wal_state* wal_state = *state;

    if (wal_state->frame_number == 0)
    {
        SAIL_LOG_ERROR("WAL: No frames were provided");
        destroy_wal_state(wal_state);
        *state = NULL;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* If only one frame was provided, generate mipmaps automatically. */
    if (wal_state->frame_number == 1)
    {
        unsigned current_width  = wal_state->width;
        unsigned current_height = wal_state->height;
        const void* src_data    = wal_state->mipmap_buffers[0];

        for (unsigned i = 1; i < 4; i++)
        {
            void* dst_data;
            unsigned dst_width, dst_height;

            SAIL_TRY_OR_CLEANUP(wal_private_downsample_indexed(src_data, current_width, current_height, &dst_data,
                                                               &dst_width, &dst_height),
                                /* cleanup */ destroy_wal_state(wal_state);
                                *state = NULL);

            wal_state->mipmap_buffers[i] = dst_data;
            wal_state->mipmap_sizes[i]   = (size_t)dst_width * dst_height;

            src_data       = dst_data;
            current_width  = dst_width;
            current_height = dst_height;
        }
    }

    /* Calculate offsets for each mipmap level. */
    const size_t header_size = sizeof(wal_state->wal_header.name) + sizeof(wal_state->wal_header.width)
                               + sizeof(wal_state->wal_header.height) + sizeof(wal_state->wal_header.offset)
                               + sizeof(wal_state->wal_header.next_name) + sizeof(wal_state->wal_header.flags)
                               + sizeof(wal_state->wal_header.contents) + sizeof(wal_state->wal_header.value);

    wal_state->wal_header.offset[0] = (int)header_size;

    for (unsigned i = 1; i < 4; i++)
    {
        wal_state->wal_header.offset[i] = wal_state->wal_header.offset[i - 1] + (int)wal_state->mipmap_sizes[i - 1];
    }

    /* Write the header. */
    SAIL_TRY_OR_CLEANUP(wal_private_write_file_header(wal_state->io, &wal_state->wal_header),
                        /* cleanup */ destroy_wal_state(wal_state);
                        *state = NULL);

    /* Write all mipmap levels. */
    for (unsigned i = 0; i < 4; i++)
    {
        if (wal_state->mipmap_buffers[i] != NULL)
        {
            SAIL_TRY_OR_CLEANUP(wal_state->io->strict_write(wal_state->io->stream, wal_state->mipmap_buffers[i],
                                                            wal_state->mipmap_sizes[i]),
                                /* cleanup */ destroy_wal_state(wal_state);
                                *state = NULL);
        }
    }

    destroy_wal_state(wal_state);
    *state = NULL;

    return SAIL_OK;
}
