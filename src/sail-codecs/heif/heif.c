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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libheif/heif.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io.h"
#include "sail-common/status.h"

static const double COMPRESSION_MIN     = 0;
static const double COMPRESSION_MAX     = 100;
static const double COMPRESSION_DEFAULT = 50;

/*
 * Codec-specific state.
 */
struct heif_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    struct heif_context* heif_context;
    struct heif_image_handle** image_handles;
    int num_images;
    int current_image;

    /* For reading */
    struct sail_heif_reader_context reader_context;
    struct heif_reader reader;

    /* For saving */
    struct heif_encoder* encoder;
    struct heif_encoding_options* encoding_options;
    struct sail_heif_writer_context writer_context;
    struct heif_writer writer;
    unsigned frames_saved;
    int threads; /* Number of threads for encoding/decoding */
};

static sail_status_t alloc_heif_state(struct sail_io* io,
                                      const struct sail_load_options* load_options,
                                      const struct sail_save_options* save_options,
                                      struct heif_state** heif_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct heif_state), &ptr));
    *heif_state = ptr;

    **heif_state = (struct heif_state){
        .io               = io,
        .load_options     = load_options,
        .save_options     = save_options,
        .heif_context     = NULL,
        .image_handles    = NULL,
        .num_images       = 0,
        .current_image    = -1,
        .reader_context   = {.io = NULL, .buffer = NULL, .buffer_size = 0},
        .reader           = {0},
        .encoder          = NULL,
        .encoding_options = NULL,
        .writer_context   = {.io = NULL},
        .writer           = {0},
        .frames_saved     = 0,
        .threads          = 1,
    };

    return SAIL_OK;
}

static void destroy_heif_state(struct heif_state* heif_state)
{
    if (heif_state == NULL)
    {
        return;
    }

    if (heif_state->image_handles != NULL)
    {
        for (int i = 0; i < heif_state->num_images; i++)
        {
            if (heif_state->image_handles[i] != NULL)
            {
                heif_image_handle_release(heif_state->image_handles[i]);
            }
        }
        sail_free(heif_state->image_handles);
    }

    if (heif_state->encoder != NULL)
    {
        heif_encoder_release(heif_state->encoder);
    }

    if (heif_state->encoding_options != NULL)
    {
        heif_encoding_options_free(heif_state->encoding_options);
    }

    if (heif_state->heif_context != NULL)
    {
        heif_context_free(heif_state->heif_context);
    }

    sail_free(heif_state->reader_context.buffer);
    sail_free(heif_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_heif(struct sail_io* io,
                                                       const struct sail_load_options* load_options,
                                                       void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct heif_state* heif_state;
    SAIL_TRY(alloc_heif_state(io, load_options, NULL, &heif_state));
    *state = heif_state;

    /* Create context. */
    heif_state->heif_context = heif_context_alloc();

    if (heif_state->heif_context == NULL)
    {
        SAIL_LOG_ERROR("HEIF: Failed to allocate context");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Setup reader. */
    const size_t buffer_size = 64 * 1024;
    void* buffer;
    SAIL_TRY(sail_malloc(buffer_size, &buffer));

    heif_state->reader_context.io          = io;
    heif_state->reader_context.buffer      = buffer;
    heif_state->reader_context.buffer_size = buffer_size;

    heif_state->reader.reader_api_version = 1;
    heif_state->reader.get_position       = heif_private_reader_get_position;
    heif_state->reader.read               = heif_private_reader_read;
    heif_state->reader.seek               = heif_private_reader_seek;
    heif_state->reader.wait_for_file_size = heif_private_reader_wait_for_file_size;

    /* Parse HEIF file. */
    struct heif_error error =
        heif_context_read_from_reader(heif_state->heif_context, &heif_state->reader, &heif_state->reader_context, NULL);

    if (error.code != heif_error_Ok)
    {
        SAIL_LOG_ERROR("HEIF: Failed to read from reader: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    /* Get number of top-level images. */
    heif_state->num_images = heif_context_get_number_of_top_level_images(heif_state->heif_context);

    if (heif_state->num_images <= 0)
    {
        SAIL_LOG_ERROR("HEIF: No images found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Allocate array for image handles. */
    void* ptr;
    SAIL_TRY(sail_malloc(heif_state->num_images * sizeof(struct heif_image_handle*), &ptr));
    heif_state->image_handles = ptr;
    memset(heif_state->image_handles, 0, heif_state->num_images * sizeof(struct heif_image_handle*));

    /* Get all top-level image IDs. */
    heif_item_id* image_ids;
    SAIL_TRY(sail_malloc(heif_state->num_images * sizeof(heif_item_id), &ptr));
    image_ids = ptr;

    heif_context_get_list_of_top_level_image_IDs(heif_state->heif_context, image_ids, heif_state->num_images);

    /* Get handles for all images. */
    for (int i = 0; i < heif_state->num_images; i++)
    {
        error = heif_context_get_image_handle(heif_state->heif_context, image_ids[i], &heif_state->image_handles[i]);

        if (error.code != heif_error_Ok)
        {
            sail_free(image_ids);
            SAIL_LOG_ERROR("HEIF: Failed to get image handle #%d: %s", i, error.message);
            SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
        }
    }

    sail_free(image_ids);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_heif(void* state, struct sail_image** image)
{
    struct heif_state* heif_state = state;

    heif_state->current_image++;

    if (heif_state->current_image >= heif_state->num_images)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    struct heif_image_handle* handle = heif_state->image_handles[heif_state->current_image];

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    /* Get image dimensions. */
    image_local->width  = heif_image_handle_get_width(handle);
    image_local->height = heif_image_handle_get_height(handle);

    /* Determine pixel format. */
    int has_alpha      = heif_image_handle_has_alpha_channel(handle);
    int bits_per_pixel = heif_image_handle_get_luma_bits_per_pixel(handle);

    if (bits_per_pixel <= 0)
    {
        bits_per_pixel = 8;
    }

    /* We'll decode to RGB/RGBA. */
    if (has_alpha)
    {
        image_local->pixel_format = (bits_per_pixel <= 8) ? SAIL_PIXEL_FORMAT_BPP32_RGBA : SAIL_PIXEL_FORMAT_BPP64_RGBA;
    }
    else
    {
        image_local->pixel_format = (bits_per_pixel <= 8) ? SAIL_PIXEL_FORMAT_BPP24_RGB : SAIL_PIXEL_FORMAT_BPP48_RGB;
    }

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Get source pixel format information. */
    if (heif_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = heif_private_sail_pixel_format_from_heif(
            heif_chroma_interleaved_RGB, heif_channel_interleaved, bits_per_pixel);
        image_local->source_image->compression = SAIL_COMPRESSION_HEVC;
    }

    /* Fetch ICC profile. */
    if (heif_state->load_options->options & SAIL_OPTION_ICCP)
    {
        SAIL_TRY_OR_CLEANUP(heif_private_fetch_iccp(handle, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch metadata. */
    if (heif_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        SAIL_TRY_OR_CLEANUP(heif_private_fetch_meta_data(handle, &image_local->meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch specialized properties. */
    if (image_local->special_properties != NULL)
    {
        SAIL_TRY_OR_CLEANUP(heif_private_fetch_depth_info(handle, image_local->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(heif_private_fetch_thumbnail_info(handle, image_local->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
        SAIL_TRY_OR_CLEANUP(heif_private_fetch_primary_flag(handle, image_local->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_heif(void* state, struct sail_image* image)
{
    struct heif_state* heif_state = state;

    /* Check if we have a valid current image. */
    if (heif_state->current_image < 0 || heif_state->current_image >= heif_state->num_images)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    struct heif_image_handle* handle = heif_state->image_handles[heif_state->current_image];

    /* Decode image. */
    struct heif_image* heif_image = NULL;
    enum heif_chroma chroma;
    int bits_per_pixel = heif_image_handle_get_luma_bits_per_pixel(handle);

    if (bits_per_pixel <= 0)
    {
        bits_per_pixel = 8;
    }

    /* Determine target chroma format based on output pixel format. */
    if (heif_image_handle_has_alpha_channel(handle))
    {
        chroma = (bits_per_pixel <= 8) ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RRGGBBAA_BE;
    }
    else
    {
        chroma = (bits_per_pixel <= 8) ? heif_chroma_interleaved_RGB : heif_chroma_interleaved_RRGGBB_BE;
    }

    struct heif_error error = heif_decode_image(handle, &heif_image, heif_colorspace_RGB, chroma, NULL);

    if (error.code != heif_error_Ok)
    {
        SAIL_LOG_ERROR("HEIF: Failed to decode image: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    /* Get pixel data. */
    int stride;
    const uint8_t* src_data = heif_image_get_plane_readonly(heif_image, heif_channel_interleaved, &stride);

    if (src_data == NULL)
    {
        heif_image_release(heif_image);
        SAIL_LOG_ERROR("HEIF: Failed to get image plane");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Copy pixel data. */
    for (unsigned row = 0; row < image->height; row++)
    {
        memcpy(sail_scan_line(image, row), src_data + row * stride, image->bytes_per_line);
    }

    /* Fetch HDR metadata and alpha info from decoded image. */
    if (image->special_properties != NULL)
    {
        heif_private_fetch_hdr_metadata(heif_image, image->special_properties);
        heif_private_fetch_premultiplied_alpha(heif_image, image->special_properties);
    }

    heif_image_release(heif_image);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_heif(void** state)
{
    struct heif_state* heif_state = *state;

    *state = NULL;

    destroy_heif_state(heif_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_heif(struct sail_io* io,
                                                       const struct sail_save_options* save_options,
                                                       void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct heif_state* heif_state;
    SAIL_TRY(alloc_heif_state(io, NULL, save_options, &heif_state));
    *state = heif_state;

    /* Create context. */
    heif_state->heif_context = heif_context_alloc();

    if (heif_state->heif_context == NULL)
    {
        SAIL_LOG_ERROR("HEIF: Failed to allocate context");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Get encoder. */
    struct heif_error error =
        heif_context_get_encoder_for_format(heif_state->heif_context, heif_compression_HEVC, &heif_state->encoder);

    if (error.code != heif_error_Ok)
    {
        SAIL_LOG_ERROR("HEIF: Failed to get encoder: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    /* Set quality. */
    const double compression =
        (save_options->compression_level < COMPRESSION_MIN || save_options->compression_level > COMPRESSION_MAX)
            ? COMPRESSION_DEFAULT
            : save_options->compression_level;

    int quality = (int)(COMPRESSION_MAX - compression);
    heif_encoder_set_lossy_quality(heif_state->encoder, quality);

    /* Handle tuning options. */
    if (save_options->tuning != NULL)
    {
        struct heif_tuning_state tuning_state = {
            .encoder = heif_state->encoder,
            .threads = &heif_state->threads,
        };
        sail_traverse_hash_map_with_user_data(save_options->tuning, heif_private_tuning_key_value_callback,
                                              &tuning_state);
    }

    /* Check compression type. */
    if (save_options->compression != SAIL_COMPRESSION_UNKNOWN && save_options->compression != SAIL_COMPRESSION_HEVC)
    {
        SAIL_LOG_ERROR("HEIF: Only HEVC compression is supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Create encoding options. */
    heif_state->encoding_options = heif_encoding_options_alloc();

    if (heif_state->encoding_options == NULL)
    {
        SAIL_LOG_ERROR("HEIF: Failed to allocate encoding options");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_heif(void* state, const struct sail_image* image)
{
    (void)state;

    /* Determine input pixel format. */
    enum heif_chroma chroma;
    int bits_per_component;
    bool has_alpha;

    if (!heif_private_heif_chroma_from_sail_pixel_format(image->pixel_format, &chroma, &bits_per_component, &has_alpha))
    {
        SAIL_LOG_ERROR("HEIF: %s pixel format is not supported for saving",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_heif(void* state, const struct sail_image* image)
{
    struct heif_state* heif_state = state;

    /* Determine input pixel format. */
    enum heif_chroma chroma;
    int bits_per_component;
    bool has_alpha;

    if (!heif_private_heif_chroma_from_sail_pixel_format(image->pixel_format, &chroma, &bits_per_component, &has_alpha))
    {
        SAIL_LOG_ERROR("HEIF: %s pixel format is not supported for saving",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Create HEIF image with appropriate colorspace. */
    struct heif_image* heif_image = NULL;
    enum heif_colorspace colorspace;

    if (chroma == heif_chroma_monochrome)
    {
        colorspace = heif_colorspace_monochrome;
    }
    else if (chroma == heif_chroma_interleaved_RGB || chroma == heif_chroma_interleaved_RGBA
             || chroma == heif_chroma_interleaved_RRGGBB_BE || chroma == heif_chroma_interleaved_RRGGBBAA_BE)
    {
        colorspace = heif_colorspace_RGB;
    }
    else
    {
        colorspace = heif_colorspace_YCbCr;
    }

    struct heif_error error = heif_image_create((int)image->width, (int)image->height, colorspace, chroma, &heif_image);

    if (error.code != heif_error_Ok)
    {
        SAIL_LOG_ERROR("HEIF: Failed to create image: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    /* Add planes and copy pixel data. */
    if (colorspace == heif_colorspace_RGB || colorspace == heif_colorspace_monochrome)
    {
        /* RGB/Grayscale: interleaved data */
        error = heif_image_add_plane(heif_image, heif_channel_interleaved, (int)image->width, (int)image->height,
                                     bits_per_component);

        if (error.code != heif_error_Ok)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to add plane: %s", error.message);
            SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
        }

        int stride;
        uint8_t* dst_data = heif_image_get_plane(heif_image, heif_channel_interleaved, &stride);

        if (dst_data == NULL)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to get image plane");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        const size_t bytes_to_copy = (stride < (int)image->bytes_per_line) ? (size_t)stride : image->bytes_per_line;

        for (unsigned row = 0; row < image->height; row++)
        {
            memcpy(dst_data + row * stride, sail_scan_line(image, row), bytes_to_copy);
        }
    }
    else
    {
        /* YCbCr: planar data, need to convert from interleaved */
        error =
            heif_image_add_plane(heif_image, heif_channel_Y, (int)image->width, (int)image->height, bits_per_component);
        if (error.code != heif_error_Ok)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to add Y plane: %s", error.message);
            SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
        }

        error = heif_image_add_plane(heif_image, heif_channel_Cb, (int)image->width, (int)image->height,
                                     bits_per_component);
        if (error.code != heif_error_Ok)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to add Cb plane: %s", error.message);
            SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
        }

        error = heif_image_add_plane(heif_image, heif_channel_Cr, (int)image->width, (int)image->height,
                                     bits_per_component);
        if (error.code != heif_error_Ok)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to add Cr plane: %s", error.message);
            SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
        }

        /* Get plane pointers */
        int stride_y, stride_cb, stride_cr;
        uint8_t* y_data  = heif_image_get_plane(heif_image, heif_channel_Y, &stride_y);
        uint8_t* cb_data = heif_image_get_plane(heif_image, heif_channel_Cb, &stride_cb);
        uint8_t* cr_data = heif_image_get_plane(heif_image, heif_channel_Cr, &stride_cr);

        if (!y_data || !cb_data || !cr_data)
        {
            heif_image_release(heif_image);
            SAIL_LOG_ERROR("HEIF: Failed to get YUV planes");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Convert interleaved YUV to planar */
        for (unsigned row = 0; row < image->height; row++)
        {
            const uint8_t* src = (const uint8_t*)sail_scan_line(image, row);
            uint8_t* y_row     = y_data + row * stride_y;
            uint8_t* cb_row    = cb_data + row * stride_cb;
            uint8_t* cr_row    = cr_data + row * stride_cr;

            for (unsigned col = 0; col < image->width; col++)
            {
                y_row[col]  = src[col * 3 + 0];
                cb_row[col] = src[col * 3 + 1];
                cr_row[col] = src[col * 3 + 2];
            }
        }
    }

    /* Encode image. */
    struct heif_image_handle* out_handle = NULL;
    error = heif_context_encode_image(heif_state->heif_context, heif_image, heif_state->encoder,
                                      heif_state->encoding_options, &out_handle);

    if (error.code != heif_error_Ok)
    {
        heif_image_release(heif_image);
        SAIL_LOG_ERROR("HEIF: Failed to encode image: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    /* Write ICC profile. */
    if (heif_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL)
    {
        SAIL_TRY_OR_CLEANUP(heif_private_write_iccp(heif_image, image->iccp),
                            /* cleanup */ heif_image_handle_release(out_handle), heif_image_release(heif_image));
    }

    /* Write metadata only for first frame. */
    if (heif_state->frames_saved == 0)
    {
        if (heif_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL)
        {
            SAIL_TRY_OR_CLEANUP(
                heif_private_write_meta_data(heif_state->heif_context, out_handle, image->meta_data_node),
                /* cleanup */ heif_image_handle_release(out_handle), heif_image_release(heif_image));
        }
    }

    heif_image_handle_release(out_handle);
    heif_image_release(heif_image);

    heif_state->frames_saved++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_heif(void** state)
{
    struct heif_state* heif_state = *state;

    *state = NULL;

    if (heif_state->frames_saved == 0)
    {
        SAIL_LOG_ERROR("HEIF: No frames were saved");
        destroy_heif_state(heif_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* Setup writer. */
    heif_state->writer_context.io = heif_state->io;

    heif_state->writer.writer_api_version = 1;
    heif_state->writer.write              = heif_private_writer_write;

    /* Write to stream. */
    struct heif_error error =
        heif_context_write(heif_state->heif_context, &heif_state->writer, &heif_state->writer_context);

    if (error.code != heif_error_Ok)
    {
        destroy_heif_state(heif_state);
        SAIL_LOG_ERROR("HEIF: Failed to write context: %s", error.message);
        SAIL_LOG_AND_RETURN(heif_private_heif_error_to_sail_status(&error));
    }

    SAIL_LOG_TRACE("HEIF: Saved %u frame(s)", heif_state->frames_saved);

    destroy_heif_state(heif_state);

    return SAIL_OK;
}
