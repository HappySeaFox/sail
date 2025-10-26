/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <png.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific data types.
 */

static const double COMPRESSION_MIN     = 1;
static const double COMPRESSION_MAX     = 9;
static const double COMPRESSION_DEFAULT = 6;

/*
 * Codec-specific state.
 */
struct png_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    png_structp png_ptr;
    png_infop info_ptr;
    int color_type;
    int bit_depth;
    int interlace_type;

    struct sail_image* first_image;
    int interlaced_passes;
    bool libpng_error;
    bool frame_processed;
    int frames;
    int current_frame;

    /* APNG-specific. */
#ifdef PNG_APNG_SUPPORTED
    bool is_apng;
    unsigned bytes_per_pixel;

    png_uint_32 next_frame_width;
    png_uint_32 next_frame_height;
    png_uint_32 next_frame_x_offset;
    png_uint_32 next_frame_y_offset;
    png_uint_16 next_frame_delay_num;
    png_uint_16 next_frame_delay_den;
    png_byte next_frame_dispose_op;
    png_byte next_frame_blend_op;

    bool skipped_hidden;
    png_bytep* prev;
    /* Temporary scanline to read into. We need it for blending. */
    void* temp_scanline;
    /* Scan line for skipping a first hidden frame. */
    void* scanline_for_skipping;

    /* For writing APNG. */
    bool is_apng_write;
    unsigned total_frames;
    unsigned frames_written;
    unsigned num_plays;
    bool acTL_written;
    png_uint_32 canvas_width;
    png_uint_32 canvas_height;
    unsigned next_seq_num;
#endif
};

static sail_status_t alloc_png_state(const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct png_state** png_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct png_state), &ptr));
    *png_state = ptr;

    **png_state = (struct png_state){
        .load_options = load_options,
        .save_options = save_options,

        .png_ptr           = NULL,
        .info_ptr          = NULL,
        .color_type        = 0,
        .bit_depth         = 0,
        .interlace_type    = 0,
        .first_image       = NULL,
        .interlaced_passes = 0,
        .libpng_error      = false,
        .frame_processed   = false,
        .frames            = 0,
        .current_frame     = 0,

/* APNG-specific. */
#ifdef PNG_APNG_SUPPORTED
        .is_apng         = false,
        .bytes_per_pixel = 0,

        .next_frame_width      = 0,
        .next_frame_height     = 0,
        .next_frame_x_offset   = 0,
        .next_frame_y_offset   = 0,
        .next_frame_delay_num  = 0,
        .next_frame_delay_den  = 0,
        .next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND,
        .next_frame_blend_op   = PNG_BLEND_OP_SOURCE,

        .skipped_hidden        = false,
        .prev                  = NULL,
        .temp_scanline         = NULL,
        .scanline_for_skipping = NULL,

        .is_apng_write  = false,
        .total_frames   = 0,
        .frames_written = 0,
        .num_plays      = 0,
        .acTL_written   = false,
        .canvas_width   = 0,
        .canvas_height  = 0,
        .next_seq_num   = 0,
#endif
    };

    return SAIL_OK;
}

static void destroy_png_state(struct png_state* png_state)
{
    if (png_state == NULL)
    {
        return;
    }

#ifdef PNG_APNG_SUPPORTED
    sail_free(png_state->temp_scanline);
    sail_free(png_state->scanline_for_skipping);

    if (png_state->first_image != NULL)
    {
        png_private_destroy_rows(&png_state->prev, png_state->first_image->height);
    }
#endif

    sail_destroy_image(png_state->first_image);

    sail_free(png_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_png(struct sail_io* io,
                                                      const struct sail_load_options* load_options,
                                                      void** state)
{
    *state = NULL;

    /* Allocate a new state. */
    struct png_state* png_state;
    SAIL_TRY(alloc_png_state(load_options, NULL, &png_state));
    *state = png_state;

    /* Initialize PNG. */
    if ((png_state->png_ptr =
             png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, png_private_my_error_fn, png_private_my_warning_fn,
                                      NULL, png_private_my_malloc_fn, png_private_my_free_fn))
        == NULL)
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL)
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    png_set_read_fn(png_state->png_ptr, io, png_private_my_read_fn);
    png_read_info(png_state->png_ptr, png_state->info_ptr);

    SAIL_TRY(sail_alloc_image(&png_state->first_image));

    if (png_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY(sail_alloc_source_image(&png_state->first_image->source_image));
    }

    png_get_IHDR(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->width,
                 &png_state->first_image->height, &png_state->bit_depth, &png_state->color_type,
                 &png_state->interlace_type,
                 /* compression type */ NULL,
                 /* filter method */ NULL);

    /* Convert to little-endian. */
    png_set_swap(png_state->png_ptr);

    png_state->first_image->pixel_format =
        png_private_png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);
    png_state->first_image->bytes_per_line =
        sail_bytes_per_line(png_state->first_image->width, png_state->first_image->pixel_format);

    /* Fetch palette. */
    if (png_state->color_type == PNG_COLOR_TYPE_PALETTE)
    {
        SAIL_TRY(png_private_fetch_palette(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->palette));
    }

    /* Fetch resolution. */
    SAIL_TRY(
        png_private_fetch_resolution(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->resolution));

    png_state->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);

    SAIL_LOG_TRACE("PNG: Interlaced passes: %d", png_state->interlaced_passes);

#ifdef PNG_APNG_SUPPORTED
    png_state->bytes_per_pixel = sail_bits_per_pixel(png_state->first_image->pixel_format) / 8;
    png_state->is_apng         = png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_acTL) != 0;
    png_state->frames          = png_state->is_apng ? png_get_num_frames(png_state->png_ptr, png_state->info_ptr) : 1;

    if (png_state->frames == 0)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    if (png_state->is_apng)
    {
        SAIL_TRY(png_private_alloc_rows(&png_state->prev, png_state->first_image->bytes_per_line,
                                        png_state->first_image->height));

        if (png_state->load_options->options & (SAIL_OPTION_META_DATA | SAIL_OPTION_SOURCE_IMAGE))
        {
            if (png_state->first_image->source_image == NULL)
            {
                SAIL_TRY(sail_alloc_source_image(&png_state->first_image->source_image));
            }
            if (png_state->first_image->source_image->special_properties == NULL)
            {
                SAIL_TRY(sail_alloc_hash_map(&png_state->first_image->source_image->special_properties));
            }

            SAIL_TRY(png_private_store_num_frames_and_plays(png_state->png_ptr, png_state->info_ptr,
                                                            png_state->first_image->source_image->special_properties));
        }
    }
#else
    png_state->frames = 1;
#endif

    if (png_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        png_state->first_image->source_image->pixel_format =
            png_private_png_color_type_to_pixel_format(png_state->color_type, png_state->bit_depth);
        png_state->first_image->source_image->compression = SAIL_COMPRESSION_DEFLATE;

        if (png_state->interlaced_passes > 1)
        {
            png_state->first_image->source_image->interlaced = true;
        }
    }

    /* Read meta data. */
    if (png_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        SAIL_TRY(png_private_fetch_meta_data(png_state->png_ptr, png_state->info_ptr,
                                             &png_state->first_image->meta_data_node));
    }

    /* Fetch ICC profile. */
    if (png_state->load_options->options & SAIL_OPTION_ICCP)
    {
        SAIL_TRY(png_private_fetch_iccp(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->iccp));
    }

    /* Fetch gamma. */
    if (png_get_gAMA(png_state->png_ptr, png_state->info_ptr, &png_state->first_image->gamma) == 0)
    {
        SAIL_LOG_TRACE("PNG: Failed to read the image gamma so it stays default");
    }

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng)
    {
        SAIL_TRY(sail_malloc(png_state->first_image->bytes_per_line, &png_state->temp_scanline));
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_png(void* state, struct sail_image** image)
{
    struct png_state* png_state = state;

    if (png_state->current_frame == png_state->frames)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (png_state->libpng_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_copy_image(png_state->first_image, &image_local));

#ifdef PNG_APNG_SUPPORTED
    if (png_state->is_apng)
    {
        /* APNG feature: a hidden frame. */
        if (!png_state->skipped_hidden && png_get_first_frame_is_hidden(png_state->png_ptr, png_state->info_ptr))
        {
            SAIL_LOG_TRACE("PNG: Skipping hidden frame");
            SAIL_TRY_OR_CLEANUP(png_private_skip_hidden_frame(png_state->first_image->bytes_per_line,
                                                              png_state->first_image->height, png_state->png_ptr,
                                                              png_state->info_ptr, &png_state->scanline_for_skipping),
                                /* cleanup */ sail_destroy_image(image_local));

            png_state->frames--;

            /* We have just a single frame left - continue to reading scan lines. */
            if (png_state->frames == 1)
            {
                png_state->next_frame_width  = png_state->first_image->width;
                png_state->next_frame_height = png_state->first_image->height;
            }
            else if (png_state->frames == 0)
            {
                sail_destroy_image(image_local);
                return SAIL_ERROR_NO_MORE_FRAMES;
            }
        }

        png_state->skipped_hidden = true;
        png_read_frame_head(png_state->png_ptr, png_state->info_ptr);

        if (png_get_valid(png_state->png_ptr, png_state->info_ptr, PNG_INFO_fcTL) != 0)
        {
            png_get_next_frame_fcTL(
                png_state->png_ptr, png_state->info_ptr, &png_state->next_frame_width, &png_state->next_frame_height,
                &png_state->next_frame_x_offset, &png_state->next_frame_y_offset, &png_state->next_frame_delay_num,
                &png_state->next_frame_delay_den, &png_state->next_frame_dispose_op, &png_state->next_frame_blend_op);
        }
        else
        {
            png_state->next_frame_width      = image_local->width;
            png_state->next_frame_height     = image_local->height;
            png_state->next_frame_x_offset   = 0;
            png_state->next_frame_y_offset   = 0;
            png_state->next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND;
            png_state->next_frame_blend_op   = PNG_BLEND_OP_SOURCE;
        }

        if (png_state->next_frame_width + png_state->next_frame_x_offset > image_local->width
            || png_state->next_frame_height + png_state->next_frame_y_offset > image_local->height)
        {
            sail_destroy_image(image_local);
            SAIL_LOG_ERROR("PNG: Frame %u,%u %ux%u doesn't fit into the canvas image %ux%u",
                           png_state->next_frame_x_offset, png_state->next_frame_y_offset, png_state->next_frame_width,
                           png_state->next_frame_height, image_local->width, image_local->height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
        }

        SAIL_LOG_TRACE("PNG: Frame #%u: %u,%u %ux%u, canvas image: %ux%u", png_state->current_frame,
                       png_state->next_frame_x_offset, png_state->next_frame_y_offset, png_state->next_frame_width,
                       png_state->next_frame_height, image_local->width, image_local->height);

        if (png_state->next_frame_delay_den == 0)
        {
            png_state->next_frame_delay_den = 100;
        }

        image_local->delay = (int)(((double)png_state->next_frame_delay_num / png_state->next_frame_delay_den) * 1000);
    }
#endif

    png_state->current_frame++;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_png(void* state, struct sail_image* image)
{
    struct png_state* png_state = state;

    if (png_state->libpng_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (int current_pass = 0; current_pass < png_state->interlaced_passes; current_pass++)
    {
#ifdef PNG_APNG_SUPPORTED
        if (png_state->is_apng)
        {
            for (unsigned row = 0; row < image->height; row++)
            {
                unsigned char* scanline = sail_scan_line(image, row);

                memcpy(scanline, png_state->prev[row], png_state->first_image->bytes_per_line);

                if (row >= png_state->next_frame_y_offset
                    && row < png_state->next_frame_y_offset + png_state->next_frame_height)
                {
                    png_read_row(png_state->png_ptr, (png_bytep)png_state->temp_scanline, NULL);

                    /* Copy all pixel values including alpha. */
                    if (png_state->current_frame == 1 || png_state->next_frame_blend_op == PNG_BLEND_OP_SOURCE)
                    {
                        SAIL_TRY(png_private_blend_source(scanline, png_state->next_frame_x_offset,
                                                          png_state->temp_scanline, png_state->next_frame_width,
                                                          png_state->bytes_per_pixel));
                    }
                    else
                    { /* PNG_BLEND_OP_OVER */
                        SAIL_TRY(png_private_blend_over(scanline, png_state->next_frame_x_offset,
                                                        png_state->temp_scanline, png_state->next_frame_width,
                                                        image->pixel_format));
                    }

                    /* Workaround: Apply disposal method only for images with bpp >= 8. */
                    if (png_state->bytes_per_pixel > 0)
                    {
                        if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_BACKGROUND)
                        {
                            memset(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                                   0, (size_t)png_state->next_frame_width * png_state->bytes_per_pixel);
                        }
                        else if (png_state->next_frame_dispose_op == PNG_DISPOSE_OP_NONE)
                        {
                            memcpy(png_state->prev[row] + png_state->next_frame_x_offset * png_state->bytes_per_pixel,
                                   scanline, (size_t)png_state->next_frame_width * png_state->bytes_per_pixel);
                        }
                        else
                        { /* PNG_DISPOSE_OP_PREVIOUS */
                        }
                    }
                }
            }
        }
        else
        {
            for (unsigned row = 0; row < image->height; row++)
            {
                png_read_row(png_state->png_ptr, sail_scan_line(image, row), NULL);
            }
        }
#else
        for (unsigned row = 0; row < image->height; row++)
        {
            png_read_row(png_state->png_ptr, sail_scan_line(image, row), NULL);
        }
#endif
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_png(void** state)
{
    struct png_state* png_state = *state;

    *state = NULL;

    if (png_state->png_ptr != NULL)
    {
        if (setjmp(png_jmpbuf(png_state->png_ptr)))
        {
            destroy_png_state(png_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    if (png_state->png_ptr != NULL)
    {
        png_destroy_read_struct(&png_state->png_ptr, &png_state->info_ptr, NULL);
    }

    destroy_png_state(png_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_png(struct sail_io* io,
                                                      const struct sail_save_options* save_options,
                                                      void** state)
{
    *state = NULL;

    struct png_state* png_state;
    SAIL_TRY(alloc_png_state(NULL, save_options, &png_state));
    *state = png_state;

    if (png_state->save_options->compression != SAIL_COMPRESSION_DEFLATE)
    {
        SAIL_LOG_ERROR("PNG: Only DEFLATE compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Initialize PNG. */
    if ((png_state->png_ptr =
             png_create_write_struct_2(PNG_LIBPNG_VER_STRING, NULL, png_private_my_error_fn, png_private_my_warning_fn,
                                       NULL, png_private_my_malloc_fn, png_private_my_free_fn))
        == NULL)
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if ((png_state->info_ptr = png_create_info_struct(png_state->png_ptr)) == NULL)
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Handle tuning. */
    if (png_state->save_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(png_state->save_options->tuning, png_private_tuning_key_value_callback,
                                              png_state->png_ptr);
    }

    png_set_write_fn(png_state->png_ptr, io, png_private_my_write_fn, png_private_my_flush_fn);

#ifdef PNG_APNG_SUPPORTED
    /* Read APNG parameters from tuning options. */
    if (png_state->save_options->tuning != NULL)
    {
        const struct sail_variant* frames_variant = sail_hash_map_value(png_state->save_options->tuning, "apng-frames");
        const struct sail_variant* plays_variant  = sail_hash_map_value(png_state->save_options->tuning, "apng-plays");

        if (frames_variant != NULL)
        {
            png_state->total_frames  = png_private_read_variant_uint(frames_variant);
            png_state->is_apng_write = (png_state->total_frames > 1);
        }
        if (plays_variant != NULL)
        {
            png_state->num_plays = png_private_read_variant_uint(plays_variant);
        }

        if (png_state->is_apng_write)
        {
            SAIL_LOG_TRACE("PNG: APNG write enabled: %u frames, %u plays", png_state->total_frames,
                           png_state->num_plays);
        }
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_png(void* state, const struct sail_image* image)
{
    struct png_state* png_state = state;

#ifdef PNG_APNG_SUPPORTED
    /* Check if we've written all frames. */
    if (png_state->total_frames > 0 && png_state->frames_written >= png_state->total_frames)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }
#else
    if (png_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    png_state->frame_processed = true;
#endif

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    int color_type;
    int bit_depth;
    SAIL_TRY_OR_CLEANUP(png_private_pixel_format_to_png_color_type(image->pixel_format, &color_type, &bit_depth),
                        /* cleanup */ SAIL_LOG_ERROR("PNG: %s pixel format is not currently supported for saving",
                                                     sail_pixel_format_to_string(image->pixel_format)));

#ifdef PNG_APNG_SUPPORTED
    /* On first frame, save canvas dimensions and write IHDR. */
    if (png_state->frames_written == 0)
    {
#endif
        /* Save meta data. */
        if (png_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL)
        {
            SAIL_TRY(png_private_write_meta_data(png_state->png_ptr, png_state->info_ptr, image->meta_data_node));
            SAIL_LOG_TRACE("PNG: Meta data has been written");
        }

        png_set_IHDR(png_state->png_ptr, png_state->info_ptr, image->width, image->height, bit_depth, color_type,
                     (png_state->save_options->options & SAIL_OPTION_INTERLACED) ? PNG_INTERLACE_ADAM7
                                                                                  : PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#ifdef PNG_APNG_SUPPORTED
        png_state->canvas_width  = image->width;
        png_state->canvas_height = image->height;
    }
    else
    {
        /* Subsequent frames: validate dimensions against canvas. */
        if (image->width > png_state->canvas_width || image->height > png_state->canvas_height)
        {
            SAIL_LOG_ERROR("PNG: Frame %u dimensions %ux%u exceed canvas %ux%u", png_state->frames_written,
                           image->width, image->height, png_state->canvas_width, png_state->canvas_height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE_DIMENSIONS);
        }
    }

    /* Save resolution (only on first frame). */
    if (png_state->frames_written == 0)
    {
#endif
        SAIL_TRY(png_private_write_resolution(png_state->png_ptr, png_state->info_ptr, image->resolution));

        /* Save ICC profile. */
        if (png_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL)
        {
            if (setjmp(png_jmpbuf(png_state->png_ptr)))
            {
                SAIL_LOG_WARNING("PNG: ICC profile was rejected (incompatible color space?)");
            }
            else
            {
                png_set_iCCP(png_state->png_ptr, png_state->info_ptr, "ICC profile", PNG_COMPRESSION_TYPE_BASE,
                             (const png_bytep)image->iccp->data, (unsigned)image->iccp->size);

                SAIL_LOG_TRACE("PNG: ICC profile has been written");
            }
        }

        /* Save palette. */
        if (sail_is_indexed(image->pixel_format))
        {
            if (image->palette == NULL)
            {
                SAIL_LOG_ERROR("PNG: The indexed image has no palette");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
            }

            /* Set palette. BPP24-RGB can be used directly, BPP32-RGBA needs conversion. */
            if (image->palette->pixel_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
            {
                png_set_PLTE(png_state->png_ptr, png_state->info_ptr, (png_color*)image->palette->data,
                             image->palette->color_count);
            }
            else if (image->palette->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA)
            {
                void* png_palette_ptr = NULL;
                SAIL_TRY(sail_malloc(image->palette->color_count * sizeof(png_color), &png_palette_ptr));
                png_color* png_palette = (png_color*)png_palette_ptr;

                void* transparency_ptr = NULL;
                SAIL_TRY_OR_CLEANUP(sail_malloc(image->palette->color_count, &transparency_ptr),
                                    /* cleanup */ sail_free(png_palette));
                png_bytep transparency = (png_bytep)transparency_ptr;

                const unsigned char* pal_data = (const unsigned char*)image->palette->data;
                for (unsigned i = 0; i < image->palette->color_count; i++)
                {
                    png_palette[i].red   = pal_data[i * 4 + 0];
                    png_palette[i].green = pal_data[i * 4 + 1];
                    png_palette[i].blue  = pal_data[i * 4 + 2];
                    transparency[i]      = pal_data[i * 4 + 3];
                }

                png_set_PLTE(png_state->png_ptr, png_state->info_ptr, png_palette, image->palette->color_count);
                png_set_tRNS(png_state->png_ptr, png_state->info_ptr, transparency, image->palette->color_count, NULL);

                sail_free(transparency);
                sail_free(png_palette);
            }
            else
            {
                SAIL_LOG_ERROR("PNG: Unsupported palette format %s",
                               sail_pixel_format_to_string(image->palette->pixel_format));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
        }

        /* Save gamma. */
        png_set_gAMA(png_state->png_ptr, png_state->info_ptr, image->gamma);

        /* Set compression. */
        const double compression = (png_state->save_options->compression_level < COMPRESSION_MIN
                                    || png_state->save_options->compression_level > COMPRESSION_MAX)
                                       ? COMPRESSION_DEFAULT
                                       : png_state->save_options->compression_level;

        png_set_compression_level(png_state->png_ptr, (int)compression);

        png_write_info(png_state->png_ptr, png_state->info_ptr);

#ifdef PNG_APNG_SUPPORTED
        /* Write acTL chunk for APNG after png_write_info. */
        if (png_state->is_apng_write && !png_state->acTL_written)
        {
            png_set_acTL(png_state->png_ptr, png_state->info_ptr, png_state->total_frames, png_state->num_plays);
            png_state->acTL_written = true;
            SAIL_LOG_TRACE("PNG: acTL written: %u frames, %u plays", png_state->total_frames, png_state->num_plays);
        }
#endif

#ifdef PNG_APNG_SUPPORTED
    }

    /* Write fcTL for each frame in APNG. */
    if (png_state->is_apng_write)
    {
        png_uint_16 delay_num = 0;
        png_uint_16 delay_den = 1000; /* Default denominator: milliseconds. */

        if (image->delay > 0)
        {
            delay_num = (png_uint_16)image->delay;
        }

        /* Always use default dimensions (full canvas) and position (0, 0). */
        /* Always use DISPOSE_OP_NONE and BLEND_OP_SOURCE for simplicity. */
        png_set_next_frame_fcTL(png_state->png_ptr, png_state->info_ptr, image->width, image->height,
                                0, /* x_offset */
                                0, /* y_offset */
                                delay_num, delay_den, PNG_DISPOSE_OP_NONE, PNG_BLEND_OP_SOURCE);
    }
#endif

#ifdef PNG_APNG_SUPPORTED
    /* Set pixel format transformations only on first frame. */
    if (png_state->frames_written == 0)
    {
#endif
        /* Convert to big-endian for PNG format. */
        png_set_swap(png_state->png_ptr);

        if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR || image->pixel_format == SAIL_PIXEL_FORMAT_BPP48_BGR
            || image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_BGRA || image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR
            || image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_BGRA || image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR)
        {
            png_set_bgr(png_state->png_ptr);
        }

        if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ARGB || image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_ABGR
            || image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ARGB || image->pixel_format == SAIL_PIXEL_FORMAT_BPP64_ABGR)
        {
            png_set_swap_alpha(png_state->png_ptr);
        }

        if (png_state->save_options->options & SAIL_OPTION_INTERLACED)
        {
            png_state->interlaced_passes = png_set_interlace_handling(png_state->png_ptr);
        }
        else
        {
            png_state->interlaced_passes = 1;
        }
#ifdef PNG_APNG_SUPPORTED
    }

    png_state->frames_written++;
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_png(void* state, const struct sail_image* image)
{
    struct png_state* png_state = state;

    if (png_state->libpng_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Error handling setup. */
    if (setjmp(png_jmpbuf(png_state->png_ptr)))
    {
        png_state->libpng_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (int current_pass = 0; current_pass < png_state->interlaced_passes; current_pass++)
    {
        for (unsigned row = 0; row < image->height; row++)
        {
            png_write_row(png_state->png_ptr, sail_scan_line(image, row));
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_png(void** state)
{
    struct png_state* png_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Error handling setup. */
    if (png_state->png_ptr != NULL)
    {
        if (setjmp(png_jmpbuf(png_state->png_ptr)))
        {
            png_state->libpng_error = true;
        }
    }

    if (png_state->png_ptr != NULL && !png_state->libpng_error && png_state->frame_processed)
    {
        png_write_end(png_state->png_ptr, png_state->info_ptr);
    }

    if (png_state->png_ptr != NULL)
    {
        png_destroy_write_struct(&png_state->png_ptr, &png_state->info_ptr);
    }

    if (png_state->libpng_error)
    {
        destroy_png_state(png_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    destroy_png_state(png_state);

    return SAIL_OK;
}
