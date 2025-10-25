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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct tiff_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    TIFF* tiff;
    uint16_t current_frame;
    bool libtiff_error;
    int save_compression;
    enum SailPixelFormat pixel_format;
    uint16_t photometric;
    uint16_t bits_per_sample;
    uint16_t samples_per_pixel;
    int line;
};

static sail_status_t alloc_tiff_state(const struct sail_load_options* load_options,
                                      const struct sail_save_options* save_options,
                                      struct tiff_state** tiff_state)
{
    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct tiff_state), &ptr));
    *tiff_state = ptr;

    **tiff_state = (struct tiff_state){
        .load_options = load_options,
        .save_options = save_options,

        .tiff              = NULL,
        .current_frame     = 0,
        .libtiff_error     = false,
        .save_compression  = COMPRESSION_NONE,
        .pixel_format      = SAIL_PIXEL_FORMAT_UNKNOWN,
        .photometric       = 0,
        .bits_per_sample   = 0,
        .samples_per_pixel = 0,
        .line              = 0,
    };

    return SAIL_OK;
}

static void destroy_tiff_state(struct tiff_state* tiff_state)
{
    if (tiff_state == NULL)
    {
        return;
    }

    sail_free(tiff_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_tiff(struct sail_io* io,
                                                       const struct sail_load_options* load_options,
                                                       void** state)
{
    *state = NULL;

    TIFFSetWarningHandler(tiff_private_my_warning_fn);
    TIFFSetErrorHandler(tiff_private_my_error_fn);

    /* Allocate a new state. */
    struct tiff_state* tiff_state;
    SAIL_TRY(alloc_tiff_state(load_options, NULL, &tiff_state));
    *state = tiff_state;

    /* Initialize TIFF.
     *
     * 'r': reading operation
     * 'h': read TIFF header only
     * 'm': disable use of memory-mapped files
     */
    tiff_state->tiff =
        TIFFClientOpen("sail-codec-tiff", "rhm", io, tiff_private_my_read_proc, tiff_private_my_write_proc,
                       tiff_private_my_seek_proc, tiff_private_my_dummy_close_proc, tiff_private_my_dummy_size_proc,
                       /* map */ NULL,
                       /* unmap */ NULL);

    if (tiff_state->tiff == NULL)
    {
        tiff_state->libtiff_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_tiff(void* state, struct sail_image** image)
{
    struct tiff_state* tiff_state = state;

    if (tiff_state->libtiff_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    /* Start reading the next directory. */
    if (!TIFFSetDirectory(tiff_state->tiff, tiff_state->current_frame++))
    {
        sail_destroy_image(image_local);
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    /* Fill the image properties. */
    if (!TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH, &image_local->width)
        || !TIFFGetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, &image_local->height))
    {
        SAIL_LOG_ERROR("TIFF: Failed to get the image dimensions");
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Determine pixel format from TIFF tags. */
    SAIL_TRY_OR_CLEANUP(tiff_private_sail_pixel_format_from_tiff(tiff_state->tiff, &tiff_state->pixel_format),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Get TIFF properties for later use. */
    TIFFGetField(tiff_state->tiff, TIFFTAG_PHOTOMETRIC, &tiff_state->photometric);
    TIFFGetField(tiff_state->tiff, TIFFTAG_BITSPERSAMPLE, &tiff_state->bits_per_sample);
    TIFFGetField(tiff_state->tiff, TIFFTAG_SAMPLESPERPIXEL, &tiff_state->samples_per_pixel);

    /* Fetch meta data. */
    if (tiff_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        struct sail_meta_data_node** last_meta_data_node = &image_local->meta_data_node;

        SAIL_TRY_OR_CLEANUP(tiff_private_fetch_meta_data(tiff_state->tiff, &last_meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));

        /* Fetch XMP data. */
        SAIL_TRY_OR_CLEANUP(tiff_private_fetch_xmp(tiff_state->tiff, &last_meta_data_node),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch ICC profile. */
    if (tiff_state->load_options->options & SAIL_OPTION_ICCP)
    {
        SAIL_TRY_OR_CLEANUP(tiff_private_fetch_iccp(tiff_state->tiff, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    /* Fetch resolution. */
    SAIL_TRY_OR_CLEANUP(tiff_private_fetch_resolution(tiff_state->tiff, &image_local->resolution),
                        /* cleanup */ sail_destroy_image(image_local));

    /* Fetch palette for indexed images. */
    if (tiff_state->photometric == PHOTOMETRIC_PALETTE)
    {
        uint16_t* red_colormap   = NULL;
        uint16_t* green_colormap = NULL;
        uint16_t* blue_colormap  = NULL;

        if (TIFFGetField(tiff_state->tiff, TIFFTAG_COLORMAP, &red_colormap, &green_colormap, &blue_colormap))
        {
            const unsigned palette_count = 1 << tiff_state->bits_per_sample;

            SAIL_TRY_OR_CLEANUP(sail_alloc_palette(&image_local->palette),
                                /* cleanup */ sail_destroy_image(image_local));

            image_local->palette->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
            image_local->palette->color_count  = palette_count;

            void* ptr;
            SAIL_TRY_OR_CLEANUP(sail_malloc(palette_count * 3, &ptr),
                                /* cleanup */ sail_destroy_image(image_local));
            image_local->palette->data = ptr;

            uint8_t* palette_data = image_local->palette->data;

            for (unsigned i = 0; i < palette_count; i++)
            {
                /* TIFF palette values are 16-bit, convert to 8-bit. */
                *palette_data++ = (uint8_t)(red_colormap[i] >> 8);
                *palette_data++ = (uint8_t)(green_colormap[i] >> 8);
                *palette_data++ = (uint8_t)(blue_colormap[i] >> 8);
            }

            SAIL_LOG_TRACE("TIFF: Loaded palette with %u colors", palette_count);
        }
    }

    image_local->pixel_format   = tiff_state->pixel_format;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Source image. */
    if (tiff_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        int compression = COMPRESSION_NONE;
        if (!TIFFGetField(tiff_state->tiff, TIFFTAG_COMPRESSION, &compression))
        {
            SAIL_LOG_ERROR("TIFF: Failed to get the image compression type");
            sail_destroy_image(image_local);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = tiff_state->pixel_format;
        image_local->source_image->compression  = tiff_private_compression_to_sail_compression(compression);
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_tiff(void* state, struct sail_image* image)
{
    struct tiff_state* tiff_state = state;

    if (tiff_state->libtiff_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Read scanlines one by one. */
    for (unsigned row = 0; row < image->height; row++)
    {
        uint8_t* scan = sail_scan_line(image, row);

        if (TIFFReadScanline(tiff_state->tiff, scan, row, 0) < 0)
        {
            SAIL_LOG_ERROR("TIFF: Failed to read scanline %u", row);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    /* Handle PHOTOMETRIC_MINISWHITE - invert the values. */
    if (tiff_state->photometric == PHOTOMETRIC_MINISWHITE)
    {
        const size_t pixels_size = (size_t)image->bytes_per_line * image->height;

        for (size_t i = 0; i < pixels_size; i++)
        {
            ((uint8_t*)image->pixels)[i] = ~((uint8_t*)image->pixels)[i];
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_tiff(void** state)
{
    struct tiff_state* tiff_state = *state;

    *state = NULL;

    if (tiff_state->tiff != NULL)
    {
        TIFFCleanup(tiff_state->tiff);
    }

    destroy_tiff_state(tiff_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_tiff(struct sail_io* io,
                                                       const struct sail_save_options* save_options,
                                                       void** state)
{
    *state = NULL;

    struct tiff_state* tiff_state;
    SAIL_TRY(alloc_tiff_state(NULL, save_options, &tiff_state));
    *state = tiff_state;

    /* Sanity check. */
    SAIL_TRY_OR_EXECUTE(tiff_private_sail_compression_to_compression(tiff_state->save_options->compression,
                                                                     &tiff_state->save_compression),
                        /* cleanup */ SAIL_LOG_ERROR("TIFF: %s compression is not supported for saving",
                                                     sail_compression_to_string(tiff_state->save_options->compression));
                        return __sail_status);

    TIFFSetWarningHandler(tiff_private_my_warning_fn);
    TIFFSetErrorHandler(tiff_private_my_error_fn);

    /* Initialize TIFF.
     *
     * 'w': writing operation
     * 'm': disable use of memory-mapped files
     */
    tiff_state->tiff = TIFFClientOpen(
        "tiff-sail-codec", "wm", io, tiff_private_my_read_proc, tiff_private_my_write_proc, tiff_private_my_seek_proc,
        /* libsail will close for us. */ tiff_private_my_dummy_close_proc, tiff_private_my_dummy_size_proc,
        /* map */ NULL,
        /* unmap */ NULL);

    if (tiff_state->tiff == NULL)
    {
        tiff_state->libtiff_error = true;
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_tiff(void* state, const struct sail_image* image)
{
    struct tiff_state* tiff_state = state;

    if (tiff_state->libtiff_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    tiff_state->line         = 0;
    tiff_state->pixel_format = image->pixel_format;

    /* Determine TIFF tags from pixel format. */
    uint16_t photometric       = 0;
    uint16_t bits_per_sample   = 0;
    uint16_t samples_per_pixel = 0;

    SAIL_TRY_OR_CLEANUP(tiff_private_sail_pixel_format_to_tiff(image->pixel_format,
                                                                &photometric,
                                                                &bits_per_sample,
                                                                &samples_per_pixel),
                        /* cleanup */ SAIL_LOG_ERROR("TIFF: Unsupported pixel format '%s' for saving",
                                                     sail_pixel_format_to_string(image->pixel_format)));

    /*
     * For JPEG compression, avoid YCbCr as it requires the height to be a multiple of 16.
     * Convert YCbCr to RGB instead. While libjpeg itself supports any height via padding,
     * libtiff does not handle this correctly and reports "fractional scanline discarded".
     *
     * Checking for "image->height % 16 != 0" doesn't make sense because we may write
     * frames with different heights.
     */
    if (tiff_state->save_compression == COMPRESSION_JPEG && photometric == PHOTOMETRIC_YCBCR)
    {
        photometric = PHOTOMETRIC_RGB;
        SAIL_LOG_DEBUG("TIFF: Changed YCbCr to RGB for JPEG compression compatibility");
    }

    /* Write TIFF tags. */
    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGEWIDTH, image->width);
    TIFFSetField(tiff_state->tiff, TIFFTAG_IMAGELENGTH, image->height);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tiff_state->tiff, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
    TIFFSetField(tiff_state->tiff, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff_state->tiff, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tiff_state->tiff, TIFFTAG_COMPRESSION, tiff_state->save_compression);
    TIFFSetField(tiff_state->tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff_state->tiff, (uint32_t)-1));

    if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_FLOAT)
    {
        TIFFSetField(tiff_state->tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }
    else if (image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_UINT)
    {
        TIFFSetField(tiff_state->tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    }

    /* Handle tuning options. */
    if (tiff_state->save_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(tiff_state->save_options->tuning, tiff_private_tuning_key_value_callback,
                                              tiff_state->tiff);
    }

    /* Save palette for indexed images. */
    if (photometric == PHOTOMETRIC_PALETTE)
    {
        if (image->palette == NULL)
        {
            SAIL_LOG_ERROR("TIFF: Indexed image must have a palette");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MISSING_PALETTE);
        }

        const unsigned palette_count = 1 << bits_per_sample;
        uint16_t* red_colormap       = NULL;
        uint16_t* green_colormap     = NULL;
        uint16_t* blue_colormap      = NULL;

        void* ptr;
        SAIL_TRY(sail_malloc(palette_count * sizeof(uint16_t) * 3, &ptr));

        red_colormap   = ptr;
        green_colormap = red_colormap + palette_count;
        blue_colormap  = green_colormap + palette_count;

        const uint8_t* palette_data = image->palette->data;
        const unsigned actual_colors =
            image->palette->color_count < palette_count ? image->palette->color_count : palette_count;

        for (unsigned i = 0; i < actual_colors; i++)
        {
            /* Convert 8-bit palette to 16-bit TIFF palette. */
            red_colormap[i]   = (uint16_t)(palette_data[i * 3 + 0] << 8);
            green_colormap[i] = (uint16_t)(palette_data[i * 3 + 1] << 8);
            blue_colormap[i]  = (uint16_t)(palette_data[i * 3 + 2] << 8);
        }

        /* Fill remaining entries with black if needed. */
        for (unsigned i = actual_colors; i < palette_count; i++)
        {
            red_colormap[i]   = 0;
            green_colormap[i] = 0;
            blue_colormap[i]  = 0;
        }

        TIFFSetField(tiff_state->tiff, TIFFTAG_COLORMAP, red_colormap, green_colormap, blue_colormap);

        sail_free(ptr);

        SAIL_LOG_TRACE("TIFF: Saved palette with %u colors", actual_colors);
    }

    /* Save ICC profile. */
    if (tiff_state->save_options->options & SAIL_OPTION_ICCP && image->iccp != NULL)
    {
        TIFFSetField(tiff_state->tiff, TIFFTAG_ICCPROFILE, image->iccp->size, image->iccp->data);
        SAIL_LOG_TRACE("TIFF: ICC profile has been saved");
    }

    /* Save meta data. */
    if (tiff_state->save_options->options & SAIL_OPTION_META_DATA && image->meta_data_node != NULL)
    {
        SAIL_LOG_TRACE("TIFF: Saving meta data");
        SAIL_TRY(tiff_private_write_meta_data(tiff_state->tiff, image->meta_data_node));

        /* Save XMP data. */
        SAIL_TRY(tiff_private_write_xmp(tiff_state->tiff, image->meta_data_node));
    }

    /* Save resolution. */
    SAIL_TRY(tiff_private_write_resolution(tiff_state->tiff, image->resolution));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_tiff(void* state, const struct sail_image* image)
{
    struct tiff_state* tiff_state = state;

    if (tiff_state->libtiff_error)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    for (unsigned row = 0; row < image->height; row++)
    {
        if (TIFFWriteScanline(tiff_state->tiff, sail_scan_line(image, row), tiff_state->line++, 0) < 0)
        {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    }

    if (!TIFFWriteDirectory(tiff_state->tiff))
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_tiff(void** state)
{
    struct tiff_state* tiff_state = *state;

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    /* Destroy internal TIFF objects. */
    if (tiff_state->tiff != NULL)
    {
        TIFFCleanup(tiff_state->tiff);
    }

    destroy_tiff_state(tiff_state);

    return SAIL_OK;
}
